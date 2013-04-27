#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x00, 0xB3, 0xC1, 0xF8, 0x31, 0xE6, 0x4E, 0x9A, 0x8E, 0x16, 0xFA, 0xB2, 0x18, 0xCF, 0x19, 0x83 }
PBL_APP_INFO(MY_UUID,
             "Bar Graph", "Cameron MacFarland",
             1, 1, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

BmpContainer faceImage;

Layer hourLayer;
Layer minLayer;

TextLayer dateText;


void update_hour(Layer *me, GContext* ctx) {
  PblTm t;

  get_time(&t);

  int height = t.tm_hour;

  if (height > 12)
    height -= 12;

  if (height == 0)
    height = 12;

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  graphics_fill_rect(ctx, GRect(38, 132 - height * 10, 30, height * 10), 0, GCornerNone);

  for (int i = 0; i < height-1; i++)
  {
    graphics_draw_line(ctx, GPoint(38, 122 - i*10), GPoint(67, 122 - i*10));
  }
}


void update_minute(Layer *me, GContext* ctx) {
  PblTm t;

  get_time(&t);

  int height = t.tm_min * 2;

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  graphics_fill_rect(ctx, GRect(76, 132 - height, 30, height), 0, GCornerNone);

  for (int i = 10; i < height; i+=10)
  {
    graphics_draw_line(ctx, GPoint(76, 132 - i), GPoint(105, 132 - i));
  }
}


void update_date()
{
  PblTm t;

  get_time(&t);

  static char date_text[] = "xxx, xxx 00";

  string_format_time(date_text, sizeof(date_text), "%a, %b %e", &t);
  text_layer_set_text(&dateText, date_text);
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;
  (void)t;

  if ((t->units_changed & HOUR_UNIT) != 0) {
    layer_mark_dirty(&hourLayer);
  }

  if ((t->units_changed & MINUTE_UNIT) != 0) {
    layer_mark_dirty(&minLayer);
  }

  if ((t->units_changed & DAY_UNIT) != 0) {
    update_date();
  }
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Bar Graph");
  window_stack_push(&window, true /* Animated */);

  resource_init_current_app(&APP_RESOURCES);

  bmp_init_container(RESOURCE_ID_IMAGE_FACE, &faceImage);
  layer_add_child(&window.layer, &faceImage.layer.layer);

  layer_init(&hourLayer, window.layer.frame);
  hourLayer.update_proc = &update_hour;
  layer_add_child(&window.layer, &hourLayer);

  layer_init(&minLayer, window.layer.frame);
  minLayer.update_proc = &update_minute;
  layer_add_child(&window.layer, &minLayer);

  text_layer_init(&dateText, GRect(0,138,144,30));
  text_layer_set_text_color(&dateText, GColorWhite);
  text_layer_set_background_color(&dateText, GColorClear);
  text_layer_set_text_alignment(&dateText, GTextAlignmentCenter);
  text_layer_set_font(&dateText, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ARIAL_22)));
  layer_add_child(&window.layer, &dateText.layer);

  update_date();
}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  bmp_deinit_container(&faceImage);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
