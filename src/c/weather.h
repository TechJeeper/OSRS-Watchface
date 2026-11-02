#pragma once

#include <pebble.h>
#include <stdbool.h>
#include <stdint.h>

enum {
  WX_CLEAR = 0,
  WX_CLOUD,
  WX_RAIN,
  WX_SNOW,
  WX_STORM,
  WX_FOG,
  WX_COUNT
};

typedef struct {
  bool valid;
  int16_t temp_now;
  int16_t feels;
  uint8_t code_now;
  uint8_t category_now;
  int16_t hour_temp[4];
  uint8_t hour_code[4];
  uint8_t hour_cat[4];
  uint8_t hour_pop[4];
  char hour_label[4][4];
  int16_t today_min;
  int16_t today_max;
  int16_t tomorrow_min;
  int16_t tomorrow_max;
  uint8_t tomorrow_cat;
  int32_t fetched_at;
} WeatherState;

void weather_init(void);
void weather_deinit(void);
void weather_request(void);
void weather_handle_inbox(DictionaryIterator *iter);
const WeatherState *weather_get(void);
uint8_t weather_category_from_code(int code);
GBitmap *weather_scene_bitmap(void);
void weather_ensure_scene(uint8_t category);
