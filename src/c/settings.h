#pragma once

#include <pebble.h>
#include <stdbool.h>

typedef enum {
  TIME_FMT_SYSTEM = 0,
  TIME_FMT_12H,
  TIME_FMT_24H
} TimeFormat;

typedef enum {
  THEME_CLASSIC = 0,
  THEME_DARK,
  THEME_WILDERNESS
} ThemeMode;

typedef struct {
  TimeFormat time_format;
  bool leading_zero;
  bool show_date;
  bool show_chips;
  bool show_orbs;
  bool show_forecast;
  bool show_tomorrow;
  bool show_heart_rate;
  bool temp_unit_f;
  int refresh_minutes;
  ThemeMode theme;
  bool invert_bw;
} FaceSettings;

typedef void (*SettingsChangedFn)(void);

void settings_init(void);
void settings_deinit(void);
void settings_set_changed_callback(SettingsChangedFn cb);
const FaceSettings *settings_get(void);
void settings_handle_inbox(DictionaryIterator *iter);
