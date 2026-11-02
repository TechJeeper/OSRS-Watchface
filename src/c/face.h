#pragma once

#include <pebble.h>

void face_init(Window *window);
void face_deinit(void);
void face_tick(void);
void face_apply_settings(void);
void face_weather_changed(void);
