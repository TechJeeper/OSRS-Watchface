#include "face.h"
#include "settings.h"
#include "theme.h"
#include "weather.h"

static Window *s_window;
static Layer *s_layer;

static GBitmap *s_orb_hp;
static GBitmap *s_orb_prayer;
static GBitmap *s_orb_run;

static int s_hour;
static int s_minute;
static int s_wday;
static int s_mday;
static int s_mon;
static int s_battery_pct = 100;
static bool s_charging;
static bool s_bt_connected = true;
static int s_steps = -1;
static int s_hr = -1;

static const char *const CAT_LABEL[] = {
  "CLEAR", "CLOUD", "RAIN", "SNOW", "STORM", "FOG"
};

static bool use_24h(void) {
  const FaceSettings *s = settings_get();
  if (s->time_format == TIME_FMT_24H) return true;
  if (s->time_format == TIME_FMT_12H) return false;
  return clock_is_24h_style();
}

static void format_time(char *buf, size_t n) {
  int hour = s_hour;
  if (!use_24h()) {
    hour = hour % 12;
    if (hour == 0) hour = 12;
  }
  if (settings_get()->leading_zero) {
    snprintf(buf, n, "%02d:%02d", hour, s_minute);
  } else {
    snprintf(buf, n, "%d:%02d", hour, s_minute);
  }
}

static void format_date_short(char *buf, size_t n) {
  static const char *const DOW[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  static const char *const MON[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
  };
  snprintf(buf, n, "%s %d %s", DOW[s_wday % 7], s_mday, MON[s_mon % 12]);
}

static void format_steps(char *buf, size_t n, int steps) {
  if (steps < 0) {
    snprintf(buf, n, "--");
  } else if (steps >= 10000) {
    snprintf(buf, n, "%dk", steps / 1000);
  } else if (steps >= 1000) {
    snprintf(buf, n, "%d.%dk", steps / 1000, (steps % 1000) / 100);
  } else {
    snprintf(buf, n, "%d", steps);
  }
}

static void draw_text_shadowed(GContext *ctx, const char *text, GFont font, GRect box,
                               GTextAlignment align, GColor fill, GColor shadow) {
  graphics_context_set_text_color(ctx, shadow);
  graphics_draw_text(ctx, text, font,
                     GRect(box.origin.x + 1, box.origin.y + 1, box.size.w, box.size.h),
                     GTextOverflowModeTrailingEllipsis, align, NULL);
  graphics_context_set_text_color(ctx, fill);
  graphics_draw_text(ctx, text, font, box,
                     GTextOverflowModeTrailingEllipsis, align, NULL);
}

static void draw_orb(GContext *ctx, GBitmap *bmp, GPoint origin, int fill_pct, GColor fill) {
  if (!bmp) return;
  GSize size = gbitmap_get_bounds(bmp).size;
  if (fill_pct < 0) fill_pct = 0;
  if (fill_pct > 100) fill_pct = 100;
  const int pad = 3;
  const int inner_h = size.h - 2 * pad;
  const int filled = (inner_h * fill_pct) / 100;
  if (filled > 0) {
    graphics_context_set_fill_color(ctx, fill);
    graphics_fill_rect(ctx,
                       GRect(origin.x + pad, origin.y + size.h - pad - filled,
                             size.w - 2 * pad, filled),
                       0, GCornerNone);
  }
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, bmp, (GRect){.origin = origin, .size = size});
}

static void draw_full_scene(GContext *ctx, GRect bounds) {
  GBitmap *scene = weather_scene_bitmap();
  if (!scene) return;
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  // Stretch scene to fill the whole watchface.
  graphics_draw_bitmap_in_rect(ctx, scene, bounds);
}

static void update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const int W = bounds.size.w;
  const int H = bounds.size.h;
  const bool tall = H >= 200;
  const FaceSettings *cfg = settings_get();
  ThemePalette pal = theme_palette(cfg->theme, cfg->invert_bw);
  const WeatherState *wx = weather_get();

  graphics_context_set_fill_color(ctx, pal.bg);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  draw_full_scene(ctx, bounds);

  // No chrome boxes / solid bars — scene stays full-bleed. Text uses ink shadows.
  const int bottom_band = tall ? 48 : 40;

  // Optional status orbs along the left edge, floating over the scene.
  int time_left = 0;
  int time_right = 0;
  if (cfg->show_orbs && s_orb_hp) {
    const int ox = 4;
    const int oy = tall ? 56 : 48;
    const int gap = tall ? 30 : 26;
    int run_pct = (s_minute * 100) / 59;
    draw_orb(ctx, s_orb_hp, GPoint(ox, oy), s_battery_pct, pal.hp);
    draw_orb(ctx, s_orb_prayer, GPoint(ox, oy + gap), s_bt_connected ? 100 : 20, pal.prayer);
    draw_orb(ctx, s_orb_run, GPoint(ox, oy + 2 * gap), run_pct, pal.run);
    time_left = 30;
  }

  // Big time — hero overlay.
  char time_buf[16];
  format_time(time_buf, sizeof(time_buf));
  GFont time_font = fonts_get_system_font(tall ? FONT_KEY_LECO_42_NUMBERS
                                               : FONT_KEY_LECO_36_BOLD_NUMBERS);
  const int time_y = tall ? 10 : 6;
  draw_text_shadowed(ctx, time_buf, time_font,
                     GRect(time_left, time_y, W - time_left - time_right, tall ? 46 : 40),
                     GTextAlignmentCenter, pal.text, GColorBlack);

  int y = time_y + (tall ? 44 : 38);

  if (cfg->show_date) {
    char date_buf[24];
    format_date_short(date_buf, sizeof(date_buf));
    draw_text_shadowed(ctx, date_buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                       GRect(0, y, W, 20), GTextAlignmentCenter, pal.text_dim, GColorBlack);
    y += 18;
  }

  if (wx->valid) {
    char temp_buf[20];
    snprintf(temp_buf, sizeof(temp_buf), "%d°  %s", wx->temp_now,
             CAT_LABEL[wx->category_now % WX_COUNT]);
    draw_text_shadowed(ctx, temp_buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                       GRect(0, y, W, 20), GTextAlignmentCenter, pal.text, GColorBlack);
  } else if (!s_bt_connected) {
    draw_text_shadowed(ctx, "No connection", fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(0, y, W, 18), GTextAlignmentCenter, pal.text_dim, GColorBlack);
  }

  // Bottom info strip — plain text over the darkened ground, no boxes.
  int by = H - bottom_band + 2;

  if (cfg->show_chips) {
    char steps_buf[12], hr_buf[12], line[48];
    format_steps(steps_buf, sizeof(steps_buf), s_steps);
    if (cfg->show_heart_rate && s_hr > 0) {
      snprintf(hr_buf, sizeof(hr_buf), "%d", s_hr);
    } else {
      snprintf(hr_buf, sizeof(hr_buf), "--");
    }
    snprintf(line, sizeof(line), "%s  %s  %d%%%s",
             steps_buf, hr_buf, s_battery_pct, s_charging ? "+" : "");
    draw_text_shadowed(ctx, line, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       GRect(2, by, W - 4, 16), GTextAlignmentCenter, pal.text, GColorBlack);
    by += 15;
  }

  if (cfg->show_forecast && wx->valid) {
    char line[64];
    snprintf(line, sizeof(line), "%s %d°  %s %d°  %s %d°  %s %d°",
             wx->hour_label[0], wx->hour_temp[0],
             wx->hour_label[1], wx->hour_temp[1],
             wx->hour_label[2], wx->hour_temp[2],
             wx->hour_label[3], wx->hour_temp[3]);
    draw_text_shadowed(ctx, line, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(1, by, W - 2, 16), GTextAlignmentCenter, pal.text_dim, GColorBlack);
    by += 14;
  }

  if (cfg->show_tomorrow) {
    char line[32];
    if (wx->valid) {
      snprintf(line, sizeof(line), "Tmr  %d° / %d°", wx->tomorrow_max, wx->tomorrow_min);
    } else {
      snprintf(line, sizeof(line), "Tmr  -- / --");
    }
    draw_text_shadowed(ctx, line, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       GRect(2, by, W - 4, 16), GTextAlignmentCenter, pal.text, GColorBlack);
  }
}

static void mark_dirty(void) {
  if (s_layer) layer_mark_dirty(s_layer);
  if (s_window) {
    ThemePalette pal = theme_palette(settings_get()->theme, settings_get()->invert_bw);
    window_set_background_color(s_window, pal.bg);
  }
}

static void refresh_health(void) {
#if defined(PBL_HEALTH)
  HealthServiceAccessibilityMask steps_mask =
      health_service_metric_accessible(HealthMetricStepCount, time_start_of_today(), time(NULL));
  if (steps_mask & HealthServiceAccessibilityMaskAvailable) {
    s_steps = (int)health_service_sum_today(HealthMetricStepCount);
  } else {
    s_steps = -1;
  }
  if (settings_get()->show_heart_rate) {
    HealthServiceAccessibilityMask hr_mask =
        health_service_metric_accessible(HealthMetricHeartRateBPM, time(NULL), time(NULL));
    if (hr_mask & HealthServiceAccessibilityMaskAvailable) {
      s_hr = (int)health_service_peek_current_value(HealthMetricHeartRateBPM);
    } else {
      s_hr = -1;
    }
  } else {
    s_hr = -1;
  }
#else
  s_steps = -1;
  s_hr = -1;
#endif
}

void face_init(Window *window) {
  s_window = window;
  s_orb_hp = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ORB_HP);
  s_orb_prayer = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ORB_PRAYER);
  s_orb_run = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ORB_RUN);

  BatteryChargeState bat = battery_state_service_peek();
  s_battery_pct = bat.charge_percent;
  s_charging = bat.is_charging;
  s_bt_connected = bluetooth_connection_service_peek();
  refresh_health();

  Layer *root = window_get_root_layer(window);
  s_layer = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_layer, update_proc);
  layer_add_child(root, s_layer);

  face_tick();
  weather_request();
  mark_dirty();
}

void face_deinit(void) {
  if (s_layer) {
    layer_destroy(s_layer);
    s_layer = NULL;
  }
  if (s_orb_hp) { gbitmap_destroy(s_orb_hp); s_orb_hp = NULL; }
  if (s_orb_prayer) { gbitmap_destroy(s_orb_prayer); s_orb_prayer = NULL; }
  if (s_orb_run) { gbitmap_destroy(s_orb_run); s_orb_run = NULL; }
  s_window = NULL;
}

void face_tick(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  s_hour = t->tm_hour;
  s_minute = t->tm_min;
  s_wday = t->tm_wday;
  s_mday = t->tm_mday;
  s_mon = t->tm_mon;

  BatteryChargeState bat = battery_state_service_peek();
  s_battery_pct = bat.charge_percent;
  s_charging = bat.is_charging;
  s_bt_connected = bluetooth_connection_service_peek();
  refresh_health();
  weather_request();
  mark_dirty();
}

void face_apply_settings(void) {
  weather_request();
  mark_dirty();
}

void face_weather_changed(void) {
  mark_dirty();
}
