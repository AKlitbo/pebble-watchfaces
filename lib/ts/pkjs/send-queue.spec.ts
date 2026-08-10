/**
 * Specs for the AppMessage send queue.
 *
 * PebbleKit JS drops a send that starts while another is in flight, so this queue is the only
 * thing keeping a cold boot's four concurrent sends from losing three of them. The transitions
 * worth pinning are the ones a reader cannot check by eye: that the queue holds the next send
 * until the current one settles, that a nack retries the same head rather than skipping it, that
 * a send nobody ever acks or nacks still frees the queue, and that a settled send cannot settle
 * twice.
 */

import { describe, test, expect, vi, beforeEach, afterEach } from 'vitest';
import { createSendQueue, SEND_RETRIES, SEND_RETRY_MS, SEND_WATCHDOG_MS } from './send-queue';
import type { SendFn } from './send-queue';

/** A captured send, kept so a spec can ack or nack it whenever it likes. */
interface Call {
  dict: AppMessageDict;
  ok: () => void;
  fail: () => void;
}

/** A send that records each call and hands back the ack/nack so the spec drives the timing. */
function recordingSend(): { calls: Call[]; send: SendFn } {
  const calls: Call[] = [];
  const send: SendFn = (dict, onOk, onFail) => {
    calls.push({ dict: dict, ok: onOk, fail: onFail });
  };

  return { calls: calls, send: send };
}

beforeEach(() => {
  vi.useFakeTimers();
});

afterEach(() => {
  vi.useRealTimers();
});

describe('createSendQueue', () => {
  /** The first send must go straight out, or every reading waits on a timer that never fires. */
  test('sends the first queued dict immediately', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);

    queueSend({ a: 1 });

    expect(calls).toHaveLength(1);
    expect(calls[0].dict).toEqual({ a: 1 });
  });

  /**
   * The bug this queue exists for. Two sends racing at cold boot means PebbleKit drops the loser
   * with no retry, so the face sits blank until some later send happens to get through.
   */
  test('holds the second dict until the first one is acked', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);

    queueSend({ a: 1 });
    queueSend({ b: 2 });

    expect(calls).toHaveLength(1);

    calls[0].ok();

    expect(calls).toHaveLength(2);
    expect(calls[1].dict).toEqual({ b: 2 });
  });

  /** onOk is how the callers latch "the watch has this now", so it must fire on the ack. */
  test('calls onOk when the send is acked', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);
    const onOk = vi.fn();

    queueSend({ a: 1 }, onOk);
    calls[0].ok();

    expect(onOk).toHaveBeenCalledTimes(1);
  });

  /** A nack usually just means a momentarily busy outbox, so the same dict must go again. */
  test('retries the same dict after a nack once the backoff has passed', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);

    queueSend({ a: 1 });
    calls[0].fail();

    expect(calls).toHaveLength(1);

    vi.advanceTimersByTime(SEND_RETRY_MS);

    expect(calls).toHaveLength(2);
    expect(calls[1].dict).toEqual({ a: 1 });
  });

  /** Retrying instantly would just lose the same race again, so the backoff has to be waited out. */
  test('does not retry before the backoff has elapsed', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);

    queueSend({ a: 1 });
    calls[0].fail();
    vi.advanceTimersByTime(SEND_RETRY_MS - 1);

    expect(calls).toHaveLength(1);
  });

  /** Without a cap on the retries one unsendable dict would wedge the queue forever. */
  test('gives up on a dict after the retry cap and calls onFail', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);
    const onFail = vi.fn();

    queueSend({ a: 1 }, undefined, onFail);
    for (let attempt = 0; attempt < SEND_RETRIES; attempt++) {
      calls[calls.length - 1].fail();
      vi.advanceTimersByTime(SEND_RETRY_MS);
    }

    expect(calls).toHaveLength(SEND_RETRIES);
    expect(onFail).toHaveBeenCalledTimes(1);
  });

  /** A dropped dict must not take the rest of the queue with it, or one bad send blanks the face. */
  test('moves on to the next dict after giving up on a stuck one', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);

    queueSend({ a: 1 });
    queueSend({ b: 2 });
    for (let attempt = 0; attempt < SEND_RETRIES; attempt++) {
      calls[calls.length - 1].fail();
      vi.advanceTimersByTime(SEND_RETRY_MS);
    }

    expect(calls[calls.length - 1].dict).toEqual({ b: 2 });
  });

  /**
   * The queue's worst failure: PebbleKit calls back neither the ack nor the nack, so without the
   * watchdog the in-flight flag stays set and nothing is ever sent again for the life of the app.
   */
  test('frees the queue with the watchdog when a send is never acked or nacked', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);

    queueSend({ a: 1 });
    vi.advanceTimersByTime(SEND_WATCHDOG_MS);
    vi.advanceTimersByTime(SEND_RETRY_MS);

    expect(calls).toHaveLength(2);
    expect(calls[1].dict).toEqual({ a: 1 });
  });

  /** The watchdog counts as a try, so a silent send must still be given up on rather than looping. */
  test('gives up on a silent send after the retry cap', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);
    const onFail = vi.fn();

    queueSend({ a: 1 }, undefined, onFail);
    for (let attempt = 0; attempt < SEND_RETRIES; attempt++) {
      vi.advanceTimersByTime(SEND_WATCHDOG_MS);
      vi.advanceTimersByTime(SEND_RETRY_MS);
    }

    expect(calls).toHaveLength(SEND_RETRIES);
    expect(onFail).toHaveBeenCalledTimes(1);
  });

  /** A late ack arriving after the watchdog already fired would shift a dict that is no longer the head. */
  test('ignores an ack that arrives after the watchdog gave up on the send', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);
    const onOk = vi.fn();

    queueSend({ a: 1 }, onOk);
    vi.advanceTimersByTime(SEND_WATCHDOG_MS);
    calls[0].ok();

    expect(onOk).not.toHaveBeenCalled();
  });

  /** A doubled ack would shift the queue twice and silently swallow the send behind it. */
  test('ignores a second ack for the same send', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);
    const onOk = vi.fn();

    queueSend({ a: 1 }, onOk);
    queueSend({ b: 2 });
    calls[0].ok();
    calls[0].ok();

    expect(onOk).toHaveBeenCalledTimes(1);
    expect(calls).toHaveLength(2);
  });

  /** An ack followed by a nack must not re-queue a send the watch already has. */
  test('ignores a nack that follows an ack for the same send', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);

    queueSend({ a: 1 });
    calls[0].ok();
    calls[0].fail();
    vi.advanceTimersByTime(SEND_RETRY_MS);

    expect(calls).toHaveLength(1);
  });

  /** The watchdog must be cleared on a clean ack, or it would later fail a send that already succeeded. */
  test('does not fire the watchdog for a send that was already acked', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);

    queueSend({ a: 1 });
    calls[0].ok();
    vi.advanceTimersByTime(SEND_WATCHDOG_MS + SEND_RETRY_MS);

    expect(calls).toHaveLength(1);
  });

  /** A cold boot queues four sends at once, and all four have to reach the watch in order. */
  test('drains a burst of sends one at a time and in order', () => {
    const { calls, send } = recordingSend();
    const queueSend = createSendQueue(send);

    queueSend({ settings: 1 });
    queueSend({ weather: 1 });
    queueSend({ stocks: 1 });
    queueSend({ calendar: 1 });
    for (let index = 0; index < 4; index++) {
      expect(calls).toHaveLength(index + 1);
      calls[index].ok();
    }

    expect(calls.map((call) => call.dict)).toEqual([
      { settings: 1 },
      { weather: 1 },
      { stocks: 1 },
      { calendar: 1 },
    ]);
  });

  /** Two faces must not share a slot, or one face's send would block the other's. */
  test('keeps separate queues independent', () => {
    const first = recordingSend();
    const second = recordingSend();
    const queueFirst = createSendQueue(first.send);
    const queueSecond = createSendQueue(second.send);

    queueFirst({ a: 1 });
    queueSecond({ b: 2 });

    expect(first.calls).toHaveLength(1);
    expect(second.calls).toHaveLength(1);
  });
});
