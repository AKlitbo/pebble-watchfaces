/*
 * fetch-request.js - Shared HTTP `request` for the live integration specs.
 *
 * Mirrors request() in src/pkjs/index.js: the body is handed back for any
 * completed response (so the providers can read the API error codes the
 * upstreams return with a non-2xx status), and only a failed fetch surfaces
 * as a transport error. Backed by Node's global fetch.
 */

/**
 * Performs an HTTP GET, shaped like request() in src/pkjs/index.js so the
 * providers can be exercised against the real upstream APIs.
 * @param {string} url
 * @param {function(?string, string=)} callback callback(error, responseText)
 */
export function fetchRequest(url, callback) {
  fetch(url)
    .then((res) => res.text().then((body) => callback(null, body)))
    .catch(() => callback('network error'));
}
