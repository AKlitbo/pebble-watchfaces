/*
 * weather.spec.js - Specs for the provider dispatcher.
 *
 * fetchWeather routes the configured provider name to its module and falls
 * back to Open-Meteo for anything unknown. The provider names are a wire
 * contract with config.js (the WEATHER_PROVIDER select values), so a drift
 * between the two silently routes the user to the wrong source.
 *
 * Rather than mock the modules, each provider is identified by the distinct
 * endpoint it requests, so the real dispatch path is exercised end to end.
 */

import { describe, test, expect } from 'vitest';
import weather from './weather.js';

// the endpoint each provider hits - the fingerprint that tells them apart
const ENDPOINT = {
  openmeteo: '/v1/forecast',
  owm: '/data/2.5/weather',
  weatherapi: '/v1/current.json'
};

// stub `request` that records the requested url, then feeds back an empty body
// so the routed provider finishes without a network call
function routing(calls) {
  return (url, callback) => {
    calls.push(url);
    callback(null, '{}');
  };
}

const BASE = { key: 'k', place: 'London', coords: { lat: 40, lon: -73 }, fahrenheit: false };

describe('fetchWeather dispatcher', () => {
  /** Each config provider name must route to its own module, or the user's choice is silently ignored. */
  test.each(['openmeteo', 'owm', 'weatherapi'])('routes provider "%s" to its endpoint', (name) => {
    const calls = [];

    weather.fetchWeather({ ...BASE, provider: name }, routing(calls), () => {});

    expect(calls).toHaveLength(1);
    expect(calls[0]).toContain(ENDPOINT[name]);
  });

  /** An unknown provider must fall back to Open-Meteo, never leave the weather unfetched. */
  test('falls back to Open-Meteo for an unknown provider', () => {
    const calls = [];

    weather.fetchWeather({ ...BASE, provider: 'definitely-not-a-provider' }, routing(calls), () => {});

    expect(calls[0]).toContain(ENDPOINT.openmeteo);
  });

  /** A missing provider (no config yet) must also fall back to Open-Meteo, not crash on undefined. */
  test('falls back to Open-Meteo when no provider is given', () => {
    const calls = [];

    weather.fetchWeather({ ...BASE, provider: undefined }, routing(calls), () => {});

    expect(calls[0]).toContain(ENDPOINT.openmeteo);
  });

  /** The dispatcher must hand the provider the opts it was given, or the request reads the wrong place. */
  test('passes the request coordinates through to the routed provider', () => {
    const calls = [];

    weather.fetchWeather({ ...BASE, provider: 'openmeteo' }, routing(calls), () => {});

    expect(calls[0]).toMatch(/latitude=40&longitude=-73/);
  });
});
