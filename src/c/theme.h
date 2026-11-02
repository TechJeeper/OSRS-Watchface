#pragma once

#include <pebble.h>
#include "settings.h"

typedef struct {
  GColor bg;
  GColor panel;
  GColor panel_edge;
  GColor bevel_light;
  GColor bevel_dark;
  GColor text;
  GColor text_dim;
  GColor xp_fill;
  GColor xp_track;
  GColor hp;
  GColor prayer;
  GColor run;
} ThemePalette;

ThemePalette theme_palette(ThemeMode theme, bool invert_bw);
