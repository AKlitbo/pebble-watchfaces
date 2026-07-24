/**
 * Weather lookup dispatcher.
 *
 * Routes a request to the configured provider module. Each provider returns
 * its result through `done` as {temperature, condition, location, ok}, already
 * formatted to the user's unit preference.
 */

import openMeteo from './providers/openmeteo';
import owm from './providers/owm';
import weatherApi from './providers/weatherapi';
import type { RequestFn, DoneFn, WeatherOpts } from './util';

/** A provider module: the one fetch entry the dispatcher calls. */
interface WeatherProvider {
  fetch: (opts: WeatherOpts, request: RequestFn, done: DoneFn) => void;
}

// provider name -> module
const PROVIDERS: Record<string, WeatherProvider> = {
  openmeteo: openMeteo,
  owm: owm,
  weatherapi: weatherApi,
};

/** Looks up current weather using the configured provider and location. */
function fetchWeather(opts: WeatherOpts, request: RequestFn, done: DoneFn): void {
  const key = String(opts.provider || '').toLowerCase();
  const provider = PROVIDERS[key];
  if (!provider) {
    // an unknown provider would quietly fall back to open-meteo and hide the mistake so log it
    console.log(`Unknown weather provider "${opts.provider}", using open-meteo`);
  }

  // each provider attaches its own forecast strip when the face asks for one so the
  // dispatcher just routes and hands the result straight back
  (provider || openMeteo).fetch(opts, request, done);
}

export default { fetchWeather };
