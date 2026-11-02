#include "settings.h"

enum {
  PERSIST_TIME_FORMAT = 1,
  PERSIST_LEADING_ZERO = 2,
  PERSIST_SHOW_DATE = 3,
  PERSIST_SHOW_CHIPS = 4,
  PERSIST_SHOW_ORBS = 5,
  PERSIST_SHOW_FORECAST = 6,
  PERSIST_SHOW_TOMORROW = 7,
  PERSIST_SHOW_HR = 8,
  PERSIST_TEMP_F = 9,
  PERSIST_REFRESH = 10,
  PERSIST_THEME = 11,
  PERSIST_INVERT_BW = 12,
};

static FaceSettings s_settings;
static SettingsChangedFn s_changed;

static int clamp_int(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static void normalize(void) {
  s_settings.time_format = (TimeFormat)clamp_int(
      (int)s_settings.time_format, TIME_FMT_SYSTEM, TIME_FMT_24H);
  s_settings.theme = (ThemeMode)clamp_int(
      (int)s_settings.theme, THEME_CLASSIC, THEME_WILDERNESS);
  switch (s_settings.refresh_minutes) {
    case 15: case 30: case 60: case 120: case 180: break;
    default: s_settings.refresh_minutes = 60; break;
  }
}

static void settings_set_defaults(void) {
  s_settings.time_format = TIME_FMT_SYSTEM;
  s_settings.leading_zero = true;
  s_settings.show_date = true;
  s_settings.show_chips = true;
  s_settings.show_orbs = false;
  s_settings.show_forecast = true;
  s_settings.show_tomorrow = true;
  s_settings.show_heart_rate = true;
  s_settings.temp_unit_f = false;
  s_settings.refresh_minutes = 60;
  s_settings.theme = THEME_CLASSIC;
  s_settings.invert_bw = false;
}

static void settings_load(void) {
  settings_set_defaults();
  if (persist_exists(PERSIST_TIME_FORMAT))
    s_settings.time_format = (TimeFormat)persist_read_int(PERSIST_TIME_FORMAT);
  if (persist_exists(PERSIST_LEADING_ZERO))
    s_settings.leading_zero = persist_read_bool(PERSIST_LEADING_ZERO);
  if (persist_exists(PERSIST_SHOW_DATE))
    s_settings.show_date = persist_read_bool(PERSIST_SHOW_DATE);
  if (persist_exists(PERSIST_SHOW_CHIPS))
    s_settings.show_chips = persist_read_bool(PERSIST_SHOW_CHIPS);
  if (persist_exists(PERSIST_SHOW_ORBS))
    s_settings.show_orbs = persist_read_bool(PERSIST_SHOW_ORBS);
  if (persist_exists(PERSIST_SHOW_FORECAST))
    s_settings.show_forecast = persist_read_bool(PERSIST_SHOW_FORECAST);
  if (persist_exists(PERSIST_SHOW_TOMORROW))
    s_settings.show_tomorrow = persist_read_bool(PERSIST_SHOW_TOMORROW);
  if (persist_exists(PERSIST_SHOW_HR))
    s_settings.show_heart_rate = persist_read_bool(PERSIST_SHOW_HR);
  if (persist_exists(PERSIST_TEMP_F))
    s_settings.temp_unit_f = persist_read_bool(PERSIST_TEMP_F);
  if (persist_exists(PERSIST_REFRESH))
    s_settings.refresh_minutes = persist_read_int(PERSIST_REFRESH);
  if (persist_exists(PERSIST_THEME))
    s_settings.theme = (ThemeMode)persist_read_int(PERSIST_THEME);
  if (persist_exists(PERSIST_INVERT_BW))
    s_settings.invert_bw = persist_read_bool(PERSIST_INVERT_BW);
  normalize();
}

static void settings_save(void) {
  normalize();
  persist_write_int(PERSIST_TIME_FORMAT, (int)s_settings.time_format);
  persist_write_bool(PERSIST_LEADING_ZERO, s_settings.leading_zero);
  persist_write_bool(PERSIST_SHOW_DATE, s_settings.show_date);
  persist_write_bool(PERSIST_SHOW_CHIPS, s_settings.show_chips);
  persist_write_bool(PERSIST_SHOW_ORBS, s_settings.show_orbs);
  persist_write_bool(PERSIST_SHOW_FORECAST, s_settings.show_forecast);
  persist_write_bool(PERSIST_SHOW_TOMORROW, s_settings.show_tomorrow);
  persist_write_bool(PERSIST_SHOW_HR, s_settings.show_heart_rate);
  persist_write_bool(PERSIST_TEMP_F, s_settings.temp_unit_f);
  persist_write_int(PERSIST_REFRESH, s_settings.refresh_minutes);
  persist_write_int(PERSIST_THEME, (int)s_settings.theme);
  persist_write_bool(PERSIST_INVERT_BW, s_settings.invert_bw);
}

static int tuple_as_int(Tuple *t, int fallback) {
  if (!t) return fallback;
  if (t->type == TUPLE_INT) return (int)t->value->int32;
  if (t->type == TUPLE_CSTRING) return atoi(t->value->cstring);
  return fallback;
}

static bool tuple_as_bool(Tuple *t, bool fallback) {
  if (!t) return fallback;
  if (t->type == TUPLE_INT) return t->value->int32 != 0;
  if (t->type == TUPLE_CSTRING) {
    return strcmp(t->value->cstring, "true") == 0 ||
           strcmp(t->value->cstring, "1") == 0;
  }
  return fallback;
}

void settings_handle_inbox(DictionaryIterator *iter) {
  Tuple *t;
  bool touched = false;

  t = dict_find(iter, MESSAGE_KEY_TimeFormat);
  if (t) { s_settings.time_format = (TimeFormat)tuple_as_int(t, (int)s_settings.time_format); touched = true; }
  t = dict_find(iter, MESSAGE_KEY_LeadingZero);
  if (t) { s_settings.leading_zero = tuple_as_bool(t, s_settings.leading_zero); touched = true; }
  t = dict_find(iter, MESSAGE_KEY_ShowDate);
  if (t) { s_settings.show_date = tuple_as_bool(t, s_settings.show_date); touched = true; }
  t = dict_find(iter, MESSAGE_KEY_ShowChips);
  if (t) { s_settings.show_chips = tuple_as_bool(t, s_settings.show_chips); touched = true; }
  t = dict_find(iter, MESSAGE_KEY_ShowOrbs);
  if (t) { s_settings.show_orbs = tuple_as_bool(t, s_settings.show_orbs); touched = true; }
  t = dict_find(iter, MESSAGE_KEY_ShowForecast);
  if (t) { s_settings.show_forecast = tuple_as_bool(t, s_settings.show_forecast); touched = true; }
  t = dict_find(iter, MESSAGE_KEY_ShowTomorrow);
  if (t) { s_settings.show_tomorrow = tuple_as_bool(t, s_settings.show_tomorrow); touched = true; }
  t = dict_find(iter, MESSAGE_KEY_ShowHeartRate);
  if (t) { s_settings.show_heart_rate = tuple_as_bool(t, s_settings.show_heart_rate); touched = true; }
  t = dict_find(iter, MESSAGE_KEY_TemperatureUnit);
  if (t) { s_settings.temp_unit_f = tuple_as_int(t, s_settings.temp_unit_f ? 1 : 0) != 0; touched = true; }
  t = dict_find(iter, MESSAGE_KEY_RefreshMinutes);
  if (t) { s_settings.refresh_minutes = tuple_as_int(t, s_settings.refresh_minutes); touched = true; }
  t = dict_find(iter, MESSAGE_KEY_Theme);
  if (t) { s_settings.theme = (ThemeMode)tuple_as_int(t, (int)s_settings.theme); touched = true; }
  t = dict_find(iter, MESSAGE_KEY_InvertBw);
  if (t) { s_settings.invert_bw = tuple_as_bool(t, s_settings.invert_bw); touched = true; }

  if (!touched) return;
  settings_save();
  if (s_changed) s_changed();
}

void settings_init(void) {
  settings_load();
}

void settings_deinit(void) {
  settings_save();
}

void settings_set_changed_callback(SettingsChangedFn cb) {
  s_changed = cb;
}

const FaceSettings *settings_get(void) {
  return &s_settings;
}
