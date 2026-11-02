#include "weather.h"
#include "settings.h"

enum {
  PERSIST_WX = 80,
};

static WeatherState s_wx;
static GBitmap *s_scene;
static uint8_t s_scene_cat = 255;
static int32_t s_last_request_at;

static const uint32_t SCENE_RES[WX_COUNT] = {
  RESOURCE_ID_IMAGE_SCENE_CLEAR,
  RESOURCE_ID_IMAGE_SCENE_CLOUD,
  RESOURCE_ID_IMAGE_SCENE_RAIN,
  RESOURCE_ID_IMAGE_SCENE_SNOW,
  RESOURCE_ID_IMAGE_SCENE_STORM,
  RESOURCE_ID_IMAGE_SCENE_FOG,
};

uint8_t weather_category_from_code(int code) {
  if (code == 0 || code == 1) return WX_CLEAR;
  if (code == 2 || code == 3) return WX_CLOUD;
  if (code == 45 || code == 48) return WX_FOG;
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return WX_RAIN;
  if ((code >= 71 && code <= 77) || (code >= 85 && code <= 86)) return WX_SNOW;
  if (code >= 95) return WX_STORM;
  if (code >= 4) return WX_CLOUD;
  return WX_CLEAR;
}

static int tuple_as_int(Tuple *t, int fallback) {
  if (!t) return fallback;
  if (t->type == TUPLE_INT) return (int)t->value->int32;
  if (t->type == TUPLE_CSTRING) return atoi(t->value->cstring);
  return fallback;
}

static void copy_hour_label(int i, Tuple *t) {
  if (!t || t->type != TUPLE_CSTRING) {
    snprintf(s_wx.hour_label[i], sizeof(s_wx.hour_label[i]), "--");
    return;
  }
  snprintf(s_wx.hour_label[i], sizeof(s_wx.hour_label[i]), "%s", t->value->cstring);
}

void weather_ensure_scene(uint8_t category) {
  if (category >= WX_COUNT) category = WX_CLEAR;
  if (s_scene && s_scene_cat == category) return;
  if (s_scene) {
    gbitmap_destroy(s_scene);
    s_scene = NULL;
  }
  s_scene = gbitmap_create_with_resource(SCENE_RES[category]);
  s_scene_cat = category;
}

GBitmap *weather_scene_bitmap(void) {
  weather_ensure_scene(s_wx.valid ? s_wx.category_now : WX_CLEAR);
  return s_scene;
}

void weather_request(void) {
  const FaceSettings *cfg = settings_get();
  int32_t now = (int32_t)time(NULL);
  if (s_last_request_at > 0 && (now - s_last_request_at) < 30) {
    return;
  }
  if (s_wx.valid && s_wx.fetched_at > 0) {
    int age = now - s_wx.fetched_at;
    if (age >= 0 && age < cfg->refresh_minutes * 60) {
      return;
    }
  }
  s_last_request_at = now;

  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) return;
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
  dict_write_uint8(iter, MESSAGE_KEY_TemperatureUnit, cfg->temp_unit_f ? 1 : 0);
  dict_write_uint16(iter, MESSAGE_KEY_RefreshMinutes, (uint16_t)cfg->refresh_minutes);
  app_message_outbox_send();
}

void weather_handle_inbox(DictionaryIterator *iter) {
  Tuple *err = dict_find(iter, MESSAGE_KEY_WEATHER_ERROR);
  if (err && tuple_as_int(err, 0) != 0) {
    return;
  }

  Tuple *t;
  bool any = false;

  t = dict_find(iter, MESSAGE_KEY_WEATHER_TEMP_NOW);
  if (t) { s_wx.temp_now = (int16_t)tuple_as_int(t, s_wx.temp_now); any = true; }
  t = dict_find(iter, MESSAGE_KEY_WEATHER_FEELS);
  if (t) { s_wx.feels = (int16_t)tuple_as_int(t, s_wx.feels); any = true; }
  t = dict_find(iter, MESSAGE_KEY_WEATHER_CODE_NOW);
  if (t) {
    s_wx.code_now = (uint8_t)tuple_as_int(t, s_wx.code_now);
    s_wx.category_now = weather_category_from_code(s_wx.code_now);
    any = true;
  }
  t = dict_find(iter, MESSAGE_KEY_WEATHER_FETCHED_AT);
  if (t) { s_wx.fetched_at = (int32_t)tuple_as_int(t, s_wx.fetched_at); any = true; }

  t = dict_find(iter, MESSAGE_KEY_HOUR0_TEMP); if (t) { s_wx.hour_temp[0] = (int16_t)tuple_as_int(t, 0); any = true; }
  t = dict_find(iter, MESSAGE_KEY_HOUR1_TEMP); if (t) { s_wx.hour_temp[1] = (int16_t)tuple_as_int(t, 0); any = true; }
  t = dict_find(iter, MESSAGE_KEY_HOUR2_TEMP); if (t) { s_wx.hour_temp[2] = (int16_t)tuple_as_int(t, 0); any = true; }
  t = dict_find(iter, MESSAGE_KEY_HOUR3_TEMP); if (t) { s_wx.hour_temp[3] = (int16_t)tuple_as_int(t, 0); any = true; }

  t = dict_find(iter, MESSAGE_KEY_HOUR0_CODE); if (t) { s_wx.hour_code[0] = (uint8_t)tuple_as_int(t, 0); s_wx.hour_cat[0] = weather_category_from_code(s_wx.hour_code[0]); }
  t = dict_find(iter, MESSAGE_KEY_HOUR1_CODE); if (t) { s_wx.hour_code[1] = (uint8_t)tuple_as_int(t, 0); s_wx.hour_cat[1] = weather_category_from_code(s_wx.hour_code[1]); }
  t = dict_find(iter, MESSAGE_KEY_HOUR2_CODE); if (t) { s_wx.hour_code[2] = (uint8_t)tuple_as_int(t, 0); s_wx.hour_cat[2] = weather_category_from_code(s_wx.hour_code[2]); }
  t = dict_find(iter, MESSAGE_KEY_HOUR3_CODE); if (t) { s_wx.hour_code[3] = (uint8_t)tuple_as_int(t, 0); s_wx.hour_cat[3] = weather_category_from_code(s_wx.hour_code[3]); }

  t = dict_find(iter, MESSAGE_KEY_HOUR0_POP); if (t) s_wx.hour_pop[0] = (uint8_t)tuple_as_int(t, 0);
  t = dict_find(iter, MESSAGE_KEY_HOUR1_POP); if (t) s_wx.hour_pop[1] = (uint8_t)tuple_as_int(t, 0);
  t = dict_find(iter, MESSAGE_KEY_HOUR2_POP); if (t) s_wx.hour_pop[2] = (uint8_t)tuple_as_int(t, 0);
  t = dict_find(iter, MESSAGE_KEY_HOUR3_POP); if (t) s_wx.hour_pop[3] = (uint8_t)tuple_as_int(t, 0);

  copy_hour_label(0, dict_find(iter, MESSAGE_KEY_HOUR0_TIME));
  copy_hour_label(1, dict_find(iter, MESSAGE_KEY_HOUR1_TIME));
  copy_hour_label(2, dict_find(iter, MESSAGE_KEY_HOUR2_TIME));
  copy_hour_label(3, dict_find(iter, MESSAGE_KEY_HOUR3_TIME));

  t = dict_find(iter, MESSAGE_KEY_TODAY_MIN); if (t) { s_wx.today_min = (int16_t)tuple_as_int(t, 0); any = true; }
  t = dict_find(iter, MESSAGE_KEY_TODAY_MAX); if (t) { s_wx.today_max = (int16_t)tuple_as_int(t, 0); any = true; }
  t = dict_find(iter, MESSAGE_KEY_TOMORROW_MIN); if (t) { s_wx.tomorrow_min = (int16_t)tuple_as_int(t, 0); any = true; }
  t = dict_find(iter, MESSAGE_KEY_TOMORROW_MAX); if (t) { s_wx.tomorrow_max = (int16_t)tuple_as_int(t, 0); any = true; }
  t = dict_find(iter, MESSAGE_KEY_TOMORROW_CODE);
  if (t) {
    s_wx.tomorrow_cat = weather_category_from_code(tuple_as_int(t, 0));
    any = true;
  }

  if (any) {
    s_wx.valid = true;
    if (s_wx.fetched_at <= 0) s_wx.fetched_at = (int32_t)time(NULL);
    persist_write_data(PERSIST_WX, &s_wx, sizeof(s_wx));
    weather_ensure_scene(s_wx.category_now);
  }
}

void weather_init(void) {
  memset(&s_wx, 0, sizeof(s_wx));
  if (persist_exists(PERSIST_WX)) {
    WeatherState cached;
    if (persist_read_data(PERSIST_WX, &cached, sizeof(cached)) == (int)sizeof(cached)) {
      s_wx = cached;
      if (s_wx.valid) {
        weather_ensure_scene(s_wx.category_now);
      }
    }
  }
}

void weather_deinit(void) {
  if (s_scene) {
    gbitmap_destroy(s_scene);
    s_scene = NULL;
  }
  s_scene_cat = 255;
}

const WeatherState *weather_get(void) {
  return &s_wx;
}
