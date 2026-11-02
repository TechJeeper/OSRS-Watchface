#include <pebble.h>
#include "face.h"
#include "settings.h"
#include "weather.h"

static Window *s_window;

static void on_tick(struct tm *tick_time, TimeUnits units_changed) {
  (void)tick_time;
  (void)units_changed;
  face_tick();
}

static void on_battery(BatteryChargeState charge) {
  (void)charge;
  face_tick();
}

static void on_bluetooth(bool connected) {
  (void)connected;
  face_tick();
}

#if defined(PBL_HEALTH)
static void on_health(HealthEventType type, void *context) {
  (void)type;
  (void)context;
  face_tick();
}
#endif

static void on_settings_changed(void) {
  face_apply_settings();
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  (void)context;
  settings_handle_inbox(iter);
  weather_handle_inbox(iter);
  face_weather_changed();
}

static void window_load(Window *window) {
  face_init(window);
}

static void window_unload(Window *window) {
  (void)window;
  face_deinit();
}

static void init(void) {
  settings_init();
  settings_set_changed_callback(on_settings_changed);
  weather_init();

  app_message_register_inbox_received(inbox_received);
  app_message_open(app_message_inbox_size_maximum(),
                   app_message_outbox_size_maximum());

  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, on_tick);
  battery_state_service_subscribe(on_battery);
  bluetooth_connection_service_subscribe(on_bluetooth);
#if defined(PBL_HEALTH)
  health_service_events_subscribe(on_health, NULL);
#endif
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  weather_deinit();
  settings_deinit();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
