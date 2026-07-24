/**
 * Shared HTTP `request` for the live integration specs.
 *
 * Mirrors request() in lib/ts/pkjs/app.ts: a 2xx passes (null, body), a non-2xx
 * passes ('http ' + status, body) so providers can still read the error JSON the
 * upstreams return, and a failed fetch surfaces as a transport error. Backed by
 * Node's global fetch.
 */

/**
 * Performs an HTTP GET, shaped like request() in lib/ts/pkjs/app.ts so the
 * providers can be exercised against the real upstream APIs.
 */
export function fetchRequest(url: string, callback: (err: string | null, body?: string) => void): void {
  fetch(url)
    .then((res) => res.text().then((body) => callback(res.ok ? null : 'http ' + res.status, body)))
    .catch(() => callback('network error'));
}
