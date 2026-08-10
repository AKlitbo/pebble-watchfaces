/**
 * Serializes the watch's AppMessage sends so they never collide.
 *
 * PebbleKit JS only allows one AppMessage in flight, so concurrent sends collide and the loser is
 * silently dropped. At cold boot the settings round-trip, weather, stocks, and calendar all fire
 * at once, so a fetched reading would drop with no retry and leave the face blank until a lone
 * later send (like a settings change) got through. Every send goes through one queue: one at a
 * time, each retried a few times on a nack before it is given up.
 */

/** How a queued dict reaches the watch. Injected so the specs can drive the ack and the nack. */
export type SendFn = (dict: AppMessageDict, onOk: () => void, onFail: () => void) => void;

/** What queueSend looks like at the call site. */
export type QueueSendFn = (dict: AppMessageDict, onOk?: () => void, onFail?: () => void) => void;

/** How many times a nacked send is retried before it is dropped and the queue moves on. */
export const SEND_RETRIES = 3;

/** How long to back off after a nack before retrying the same head. */
export const SEND_RETRY_MS = 250;

/** How long to wait for an ack or a nack before counting the send as failed. */
export const SEND_WATCHDOG_MS = 8000;

/** A send waiting its turn, with the retry count it has used up so far. */
interface QueueItem {
  dict: AppMessageDict;
  onOk?: () => void;
  onFail?: () => void;
  tries: number;
}

/**
 * Builds a queueSend that pushes through `send` one message at a time.
 *
 * Each queue is independent, so a face gets one and every send it makes shares that single slot.
 */
export function createSendQueue(send: SendFn): QueueSendFn {
  const items: QueueItem[] = [];
  let sending = false;

  function pump(): void {
    if (sending || items.length === 0) {
      return;
    }
    sending = true;
    const item = items[0];

    // resolve each send exactly once. a lost ack/nack (neither callback ever fires) would otherwise
    // leave sending true forever and wedge the whole queue, so a watchdog counts as a failure
    let settled = false;
    const settle = (ok: boolean): void => {
      if (settled) {
        return;
      }
      settled = true;
      clearTimeout(watchdog);
      sending = false;

      if (ok) {
        items.shift();
        if (item.onOk) {
          item.onOk();
        }
        pump();
        return;
      }

      // a nack usually just means the outbox was momentarily busy, so back off and retry the same
      // head a few times before dropping it and moving on so one stuck send can't wedge the queue
      item.tries++;
      if (item.tries >= SEND_RETRIES) {
        items.shift();
        if (item.onFail) {
          item.onFail();
        }
        pump();
      } else {
        setTimeout(pump, SEND_RETRY_MS);
      }
    };

    const watchdog = setTimeout(() => settle(false), SEND_WATCHDOG_MS);
    send(item.dict, () => settle(true), () => settle(false));
  }

  return function queueSend(dict, onOk, onFail) {
    items.push({ dict: dict, onOk: onOk, onFail: onFail, tries: 0 });
    pump();
  };
}

export default { createSendQueue, SEND_RETRIES, SEND_RETRY_MS, SEND_WATCHDOG_MS };
