/*

  Segment Six watch.

 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID {0xE1, 0x72, 0xD7, 0x10, 0xE1, 0x69, 0x49, 0x24, 0xA8, 0xB8, 0x76, 0xC3, 0x2F, 0xAA, 0x7A, 0x66}
PBL_APP_INFO(MY_UUID, "Segment Six", "Pebble Technology", 0x2, 0x0, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;

Layer minute_display_layer;
Layer hour_display_layer;


void graphics_draw_arc(GContext *ctx, GPoint p, int radius, int thickness, int start, int end) {
  start = start % 360 - 90;
  end = end % 360 - 90;
  
  int sslope = TRIG_MAX_RATIO * sin_lookup(start * TRIG_MAX_ANGLE / 360) / cos_lookup(start * TRIG_MAX_ANGLE / 360);
  int eslope = TRIG_MAX_RATIO * sin_lookup(end * TRIG_MAX_ANGLE / 360) / cos_lookup(end * TRIG_MAX_ANGLE / 360);

  int ir2 = (radius - thickness) * (radius - thickness);
  int or2 = radius * radius;

  for (int x = -radius; x <= radius; x++)
    for (int y = -radius; y <= radius; y++)
    {
      int x2 = x * x;
      int y2 = y * y;

      if ((x2 + y2 < or2 && x2 + y2 >= ir2) &&
        ((start == 0 && x >= 0) || (start == 180 && x <= 0) || (y > x * sslope / TRIG_MAX_RATIO && start < 180) || (y <= x * sslope / TRIG_MAX_RATIO && start > 180)) &&
        ((end == 360 && x <= 0) || (end == 180 && x >= 0) || (y < x * eslope / TRIG_MAX_RATIO && end < 180) || (y >= x * eslope / TRIG_MAX_RATIO && end > 180)))
        graphics_draw_pixel(ctx, GPoint(p.x + x, p.y + y));
    }
}


void minute_display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;

  get_time(&t);

  unsigned int angle = t.tm_min * 6;

  GPoint center = grect_center_point(&me->frame);

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_draw_arc(ctx, center, 77, 25, 0, angle);
}


void hour_display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;

  get_time(&t);

  unsigned int angle = (((t.tm_hour % 12) * 30) + (t.tm_min/2));

  GPoint center = grect_center_point(&me->frame);

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_draw_arc(ctx, center, 48, 40, 0, angle);
}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {

  (void)t; // TODO: Pass the time direct to the layers?
  (void)ctx;

  layer_mark_dirty(&minute_display_layer);
  layer_mark_dirty(&hour_display_layer);
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Segment Six watch");
  window_stack_push(&window, true);
  window_set_background_color(&window, GColorBlack);


  // Init the layer for the minute display
  layer_init(&minute_display_layer, window.layer.frame);
  minute_display_layer.update_proc = &minute_display_layer_update_callback;
  layer_add_child(&window.layer, &minute_display_layer);


  // Init the layer for the hour display
  layer_init(&hour_display_layer, window.layer.frame);
  hour_display_layer.update_proc = &hour_display_layer_update_callback;
  layer_add_child(&window.layer, &hour_display_layer);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    // Handle time updates
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }

  };
  app_event_loop(params, &handlers);
}
