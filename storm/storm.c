#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x4B, 0x56, 0x10, 0x5B, 0xF7, 0xC5, 0x40, 0x07, 0x85, 0x84, 0x8D, 0x88, 0xB3, 0xCB, 0x5E, 0xB4 }
PBL_APP_INFO(MY_UUID,
             "Storm", "Cameron MacFarland",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

Layer month_display_layer;
Layer day_display_layer;
Layer hour_display_layer;
Layer minute_display_layer;

TextLayer date_text_layer;
TextLayer time_text_layer;


void graphics_draw_arc(GContext *ctx, GPoint p, int radius, int thickness, int start, int end) {
  start = start % 360;
  end = end % 360;

  while (start < 0) start += 360;
  while (end < 0) end += 360;

  if (end == 0) end = 360;
  
  float sslope = (float)cos_lookup(start * TRIG_MAX_ANGLE / 360) / (float)sin_lookup(start * TRIG_MAX_ANGLE / 360);
  float eslope = (float)cos_lookup(end * TRIG_MAX_ANGLE / 360) / (float)sin_lookup(end * TRIG_MAX_ANGLE / 360);

  if (end == 360) eslope = -1000000;

  int ir2 = (radius - thickness) * (radius - thickness);
  int or2 = radius * radius;

  for (int x = -radius; x <= radius; x++)
    for (int y = -radius; y <= radius; y++)
    {
      int x2 = x * x;
      int y2 = y * y;

      if (
        (x2 + y2 < or2 && x2 + y2 >= ir2) &&
        (
          (y > 0 && start < 180 && x <= y * sslope) ||
          (y < 0 && start > 180 && x >= y * sslope) ||
          (y < 0 && start <= 180) ||
          (y == 0 && start <= 180 && x < 0) ||
          (y == 0 && start == 0 && x > 0)
        ) &&
        (
          (y > 0 && end < 180 && x >= y * eslope) ||
          (y < 0 && end > 180 && x <= y * eslope) ||
          (y > 0 && end >= 180) ||
          (y == 0 && end >= 180 && x < 0) ||
          (y == 0 && start == 0 && x > 0)
        )
      )
        graphics_draw_pixel(ctx, GPoint(p.x + x, p.y + y));
    }
}


void month_display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;

  get_time(&t);

  unsigned int angle = (t.tm_mon + 1) * 15;

  GPoint center = grect_center_point(&me->frame);
  center.y += 10;

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_draw_arc(ctx, center, 42, 20, 0, angle);

  //graphics_draw_line(ctx, GPoint(7, center.y), GPoint(20, center.y));
  //graphics_draw_line(ctx, GPoint(124, center.y), GPoint(137, center.y));

  for (int x = 0; x < 2; x++)
    for (int y = 0; y < 2; y++)
    {
      graphics_draw_pixel(ctx, GPoint(center.x - 01 + x, center.y + 18 + y));
      graphics_draw_pixel(ctx, GPoint(center.x - 13 + x, center.y + 13 + y));
      graphics_draw_pixel(ctx, GPoint(center.x + 13 + x, center.y + 13 + y));
    }
}


void day_display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;

  get_time(&t);

  int daysInMonth = t.tm_mon == 3 || t.tm_mon == 5 || t.tm_mon == 8 || t.tm_mon == 10 ? 30 :
    t.tm_mon != 1 ? 31:
    t.tm_year % 4 == 0 && (t.tm_year % 400 == 0 || t.tm_year % 100 != 0) ? 29 : 28;

  unsigned int angle = (t.tm_mday * 180) / daysInMonth;

  GPoint center = grect_center_point(&me->frame);
  center.y += 10;

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_draw_arc(ctx, center, 62, 10, 0, angle);

  //graphics_draw_line(ctx, GPoint(27, center.y), GPoint(50, center.y));
  //graphics_draw_line(ctx, GPoint(94, center.y), GPoint(117, center.y));

  for (int x = -1; x < 3; x++)
    for (int y = -1; y < 3; y++)
      if (x + y > -2 && x + y < 4 && x - y < 3 && y - x < 3)
      {
        graphics_draw_pixel(ctx, GPoint(center.x - 00 + x, center.y + 66 + y));
        graphics_draw_pixel(ctx, GPoint(center.x - 47 + x, center.y + 47 + y));
        graphics_draw_pixel(ctx, GPoint(center.x + 47 + x, center.y + 47 + y));
      }

  for (int x = 0; x < 2; x++)
    for (int y = 0; y < 2; y++)
    {
      graphics_draw_pixel(ctx, GPoint(center.x - 64 + x, center.y + 17 + y));
      graphics_draw_pixel(ctx, GPoint(center.x - 57 + x, center.y + 33 + y));
      graphics_draw_pixel(ctx, GPoint(center.x - 33 + x, center.y + 57 + y));
      graphics_draw_pixel(ctx, GPoint(center.x - 17 + x, center.y + 64 + y));

      graphics_draw_pixel(ctx, GPoint(center.x + 64 + x, center.y + 17 + y));
      graphics_draw_pixel(ctx, GPoint(center.x + 57 + x, center.y + 33 + y));
      graphics_draw_pixel(ctx, GPoint(center.x + 33 + x, center.y + 57 + y));
      graphics_draw_pixel(ctx, GPoint(center.x + 17 + x, center.y + 64 + y));
    }
}


void hour_display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;

  get_time(&t);

  unsigned int angle = (t.tm_hour % 12) * 15 + 180;

  GPoint center = grect_center_point(&me->frame);
  center.y -= 10;

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_draw_arc(ctx, center, 62, 10, 180, angle);

  //graphics_draw_line(ctx, GPoint(7, center.y), GPoint(20, center.y));
  //graphics_draw_line(ctx, GPoint(124, center.y), GPoint(137, center.y));

  for (int x = -1; x < 3; x++)
    for (int y = -1; y < 3; y++)
      if (x + y > -2 && x + y < 4 && x - y < 3 && y - x < 3)
      {
        graphics_draw_pixel(ctx, GPoint(center.x - 00 + x, center.y - 66 + y));
        graphics_draw_pixel(ctx, GPoint(center.x - 47 + x, center.y - 47 + y));
        graphics_draw_pixel(ctx, GPoint(center.x + 47 + x, center.y - 47 + y));
      }

  for (int x = 0; x < 2; x++)
    for (int y = 0; y < 2; y++)
    {
      graphics_draw_pixel(ctx, GPoint(center.x - 64 + x, center.y - 17 + y));
      graphics_draw_pixel(ctx, GPoint(center.x - 57 + x, center.y - 33 + y));
      graphics_draw_pixel(ctx, GPoint(center.x - 33 + x, center.y - 57 + y));
      graphics_draw_pixel(ctx, GPoint(center.x - 17 + x, center.y - 64 + y));

      graphics_draw_pixel(ctx, GPoint(center.x + 64 + x, center.y - 17 + y));
      graphics_draw_pixel(ctx, GPoint(center.x + 57 + x, center.y - 33 + y));
      graphics_draw_pixel(ctx, GPoint(center.x + 33 + x, center.y - 57 + y));
      graphics_draw_pixel(ctx, GPoint(center.x + 17 + x, center.y - 64 + y));
    }
}


void minute_display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;

  get_time(&t);

  unsigned int angle = t.tm_min * 3 + 180;

  GPoint center = grect_center_point(&me->frame);
  center.y -= 10;

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_draw_arc(ctx, center, 42, 20, 180, angle);

  //graphics_draw_line(ctx, GPoint(27, center.y), GPoint(50, center.y));
  //graphics_draw_line(ctx, GPoint(94, center.y), GPoint(117, center.y));

  for (int x = 0; x < 2; x++)
    for (int y = 0; y < 2; y++)
    {
      graphics_draw_pixel(ctx, GPoint(center.x - 01 + x, center.y - 18 + y));
      graphics_draw_pixel(ctx, GPoint(center.x - 13 + x, center.y - 13 + y));
      graphics_draw_pixel(ctx, GPoint(center.x + 13 + x, center.y - 13 + y));
    }
}


void update_text() {
  PblTm t;

  get_time(&t);

  static char date_str[] = "xxx 00";
  static char time_str[] = "00 00";

  char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%H %M";
  } else {
    time_format = "%I %M";
  }

  string_format_time(time_str, sizeof(time_str), time_format, &t);
  string_format_time(date_str, sizeof(date_str), "%b %d", &t);

  text_layer_set_text(&date_text_layer, date_str);
  text_layer_set_text(&time_text_layer, time_str);
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;

  layer_mark_dirty(&month_display_layer);
  layer_mark_dirty(&day_display_layer);
  layer_mark_dirty(&hour_display_layer);
  layer_mark_dirty(&minute_display_layer);

  update_text();
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Storm");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&APP_RESOURCES);

  layer_init(&month_display_layer, window.layer.frame);
  month_display_layer.update_proc = &month_display_layer_update_callback;
  layer_add_child(&window.layer, &month_display_layer);

  layer_init(&day_display_layer, window.layer.frame);
  day_display_layer.update_proc = &day_display_layer_update_callback;
  layer_add_child(&window.layer, &day_display_layer);

  layer_init(&hour_display_layer, window.layer.frame);
  hour_display_layer.update_proc = &hour_display_layer_update_callback;
  layer_add_child(&window.layer, &hour_display_layer);

  layer_init(&minute_display_layer, window.layer.frame);
  minute_display_layer.update_proc = &minute_display_layer_update_callback;
  layer_add_child(&window.layer, &minute_display_layer);

  text_layer_init(&date_text_layer, GRect(82,168/2-12,62,24));
  text_layer_set_text_color(&date_text_layer, GColorWhite);
  text_layer_set_background_color(&date_text_layer, GColorClear);
  text_layer_set_text_alignment(&date_text_layer, GTextAlignmentCenter);
  text_layer_set_font(&date_text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTOCONDENSED_LIGHT_18)));
  layer_add_child(&window.layer, &date_text_layer.layer);

  text_layer_init(&time_text_layer, GRect(0,168/2-13,62,26));
  text_layer_set_text_color(&time_text_layer, GColorWhite);
  text_layer_set_background_color(&time_text_layer, GColorClear);
  text_layer_set_text_alignment(&time_text_layer, GTextAlignmentCenter);
  text_layer_set_font(&time_text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTOCONDENSED_BOLD_20)));
  layer_add_child(&window.layer, &time_text_layer.layer);

  update_text();
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
