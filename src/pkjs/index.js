var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

var MAX_STALE_CACHE_MS = 12 * 60 * 60 * 1000;

function xhrRequest(url, type, callback, errorCallback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    if (xhr.status >= 200 && xhr.status < 300) {
      callback(this.responseText);
    } else if (errorCallback) {
      errorCallback('HTTP ' + xhr.status);
    }
  };
  xhr.onerror = function () {
    if (errorCallback) errorCallback('network error');
  };
  xhr.open(type, url);
  xhr.send();
}

function pad2(n) {
  return (n < 10 ? '0' : '') + n;
}

function nowSeconds() {
  return Math.floor(Date.now() / 1000);
}

function clampRefreshMinutes(value) {
  var n = Number(value || 60);
  if (n !== 15 && n !== 30 && n !== 60 && n !== 120 && n !== 180) return 60;
  return n;
}

function storageGet(key) {
  try {
    if (typeof localStorage === 'undefined') return null;
    var value = localStorage.getItem(key);
    return value ? JSON.parse(value) : null;
  } catch (e) {
    return null;
  }
}

function storageSet(key, value) {
  try {
    if (typeof localStorage === 'undefined') return;
    localStorage.setItem(key, JSON.stringify(value));
  } catch (e) {}
}

function weatherCacheKey(useF) {
  // Bump when hourly indexing / labels change so phones drop bad caches.
  return 'osrs-weather-cache-v2-' + (useF ? 'f' : 'c');
}

function locationCacheKey() {
  return 'osrs-weather-location-v1';
}

function numOr(value, fallback) {
  var n = Number(value);
  return value == null || isNaN(n) ? fallback : n;
}

// Open-Meteo with timezone=auto returns wall-clock times with no offset
// (e.g. "2026-11-02T16:00"). new Date(iso) often treats that as UTC and
// shifts the hourly strip by the phone's timezone — parse as local instead.
function parseOmLocal(isoTime) {
  var m = /^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2})/.exec(isoTime || '');
  if (!m) return null;
  return new Date(+m[1], +m[2] - 1, +m[3], +m[4], +m[5], 0, 0);
}

function hourLabel(isoTime) {
  var d = parseOmLocal(isoTime);
  if (!d || isNaN(d.getTime())) {
    var m = /T(\d\d):/.exec(isoTime || '');
    return m ? m[1] : '--';
  }
  return pad2(d.getHours());
}

function findCurrentHourIndex(hourlyTimes) {
  var now = new Date();
  var best = 0;
  for (var i = 0; i < hourlyTimes.length; i++) {
    var d = parseOmLocal(hourlyTimes[i]);
    if (d && !isNaN(d.getTime()) && d.getTime() >= now.getTime() - 30 * 60 * 1000) {
      return i;
    }
    best = i;
  }
  return best;
}

function sendPayload(payload, label) {
  Pebble.sendAppMessage(
    payload,
    function () {
      console.log(label + ' sent');
    },
    function (e) {
      console.log('Failed ' + label + ': ' + JSON.stringify(e));
    }
  );
}

function readWeatherCache(useF) {
  var cached = storageGet(weatherCacheKey(useF));
  if (!cached || !cached.payload || !cached.fetchedAt) return null;
  return cached;
}

function writeWeatherCache(useF, payload) {
  storageSet(weatherCacheKey(useF), {
    fetchedAt: payload.WEATHER_FETCHED_AT || nowSeconds(),
    payload: payload
  });
}

function sendCachedWeather(useF, maxAgeMs, allowStale, reason) {
  var cached = readWeatherCache(useF);
  if (!cached) return false;
  var ageMs = Date.now() - cached.fetchedAt * 1000;
  if (ageMs <= maxAgeMs || (allowStale && ageMs <= MAX_STALE_CACHE_MS)) {
    sendPayload(cached.payload, reason || 'cached weather');
    return true;
  }
  return false;
}

function sendWeather(json, useF) {
  var hourly = json.hourly || {};
  var daily = json.daily || {};
  var current = json.current || {};
  var times = hourly.time || [];
  var start = findCurrentHourIndex(times);
  var payload = {};

  payload.WEATHER_TEMP_NOW = Math.round(numOr(current.temperature_2m, 0));
  payload.WEATHER_FEELS = Math.round(numOr(current.apparent_temperature, payload.WEATHER_TEMP_NOW));
  payload.WEATHER_CODE_NOW = numOr(current.weather_code, 0);
  payload.WEATHER_UPDATED = pad2(new Date().getHours()) + ':' + pad2(new Date().getMinutes());
  payload.WEATHER_FETCHED_AT = nowSeconds();

  for (var i = 0; i < 4; i++) {
    var idx = Math.min(start + i, Math.max(0, times.length - 1));
    payload['HOUR' + i + '_TEMP'] = Math.round(numOr((hourly.temperature_2m || [])[idx], 0));
    payload['HOUR' + i + '_CODE'] = numOr((hourly.weather_code || [])[idx], 0);
    payload['HOUR' + i + '_POP'] = numOr((hourly.precipitation_probability || [])[idx], 0);
    payload['HOUR' + i + '_TIME'] = hourLabel(times[idx] || '');
  }

  payload.TODAY_MIN = Math.round(numOr((daily.temperature_2m_min || [])[0], 0));
  payload.TODAY_MAX = Math.round(numOr((daily.temperature_2m_max || [])[0], 0));
  payload.TODAY_CODE = numOr((daily.weather_code || [])[0], 0);
  payload.TODAY_POP = numOr((daily.precipitation_probability_max || [])[0], 0);
  payload.TOMORROW_MIN = Math.round(numOr((daily.temperature_2m_min || [])[1], payload.TODAY_MIN));
  payload.TOMORROW_MAX = Math.round(numOr((daily.temperature_2m_max || [])[1], payload.TODAY_MAX));
  payload.TOMORROW_CODE = numOr((daily.weather_code || [])[1], payload.TODAY_CODE);
  payload.TOMORROW_POP = numOr((daily.precipitation_probability_max || [])[1], payload.TODAY_POP);

  writeWeatherCache(useF, payload);
  sendPayload(payload, 'weather forecast');
}

function fetchWeatherAt(coords, useF, errorCallback) {
  var unit = useF ? 'fahrenheit' : 'celsius';
  var url =
    'https://api.open-meteo.com/v1/forecast?' +
    'latitude=' +
    encodeURIComponent(coords.latitude) +
    '&longitude=' +
    encodeURIComponent(coords.longitude) +
    '&current=temperature_2m,apparent_temperature,weather_code' +
    '&hourly=temperature_2m,weather_code,precipitation_probability' +
    '&daily=temperature_2m_min,temperature_2m_max,weather_code,precipitation_probability_max' +
    '&forecast_days=2' +
    '&timezone=auto' +
    '&temperature_unit=' +
    unit;

  xhrRequest(
    url,
    'GET',
    function (responseText) {
      try {
        sendWeather(JSON.parse(responseText), useF);
      } catch (e) {
        errorCallback('parse error: ' + e.message);
      }
    },
    errorCallback
  );
}

function getWeather(useF, refreshMinutes) {
  var refresh = clampRefreshMinutes(refreshMinutes);
  var cacheMaxAge = refresh * 60 * 1000;

  if (sendCachedWeather(useF, cacheMaxAge, false, 'fresh cached weather')) {
    return;
  }

  function fallback(reason) {
    if (!sendCachedWeather(useF, cacheMaxAge, true, 'stale cached weather')) {
      console.log('Weather unavailable: ' + reason);
      sendPayload({ WEATHER_ERROR: 1 }, 'weather error');
    }
  }

  navigator.geolocation.getCurrentPosition(
    function (pos) {
      storageSet(locationCacheKey(), {
        storedAt: Date.now(),
        coords: { latitude: pos.coords.latitude, longitude: pos.coords.longitude }
      });
      fetchWeatherAt(pos.coords, useF, function (err) {
        fallback('weather request failed: ' + err);
      });
    },
    function () {
      var last = storageGet(locationCacheKey());
      if (last && last.coords && Date.now() - last.storedAt < 24 * 60 * 60 * 1000) {
        fetchWeatherAt(last.coords, useF, function (err) {
          fallback('last-location weather failed: ' + err);
        });
      } else {
        fallback('geolocation failed');
      }
    },
    { enableHighAccuracy: false, maximumAge: 15 * 60 * 1000, timeout: 15000 }
  );
}

Pebble.addEventListener('ready', function () {
  getWeather(false, 60);
});

Pebble.addEventListener('appmessage', function (e) {
  var dict = e.payload || {};
  if (dict.REQUEST_WEATHER != null) {
    var useF = Number(dict.TemperatureUnit || 0) !== 0;
    var refresh = clampRefreshMinutes(dict.RefreshMinutes);
    getWeather(useF, refresh);
  }
});

Pebble.addEventListener('showConfiguration', function () {
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function (e) {
  if (!e || !e.response) return;
  var settings = clay.getSettings(e.response);
  Pebble.sendAppMessage(settings, function () {
    var useF = Number(settings.TemperatureUnit || 0) !== 0;
    var refresh = clampRefreshMinutes(settings.RefreshMinutes);
    getWeather(useF, refresh);
  });
});
