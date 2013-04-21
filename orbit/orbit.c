#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x7A, 0xCB, 0x68, 0x2C, 0x4E, 0x6C, 0x4A, 0xDA, 0x93, 0x36, 0x00, 0x47, 0xFF, 0x28, 0xEF, 0xB5 }
PBL_APP_INFO(MY_UUID,
             "Orbit", "Cameron MacFarland",
             0, 1, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

RotBmpPairContainer minuteImage;
RotBmpPairContainer hourImage;

TextLayer minuteText;
TextLayer hourText;


void set_hand_angle(RotBmpPairContainer *hand_image_container, unsigned int hand_angle, GPoint offset) {

  signed short x_fudge = 0;
  signed short y_fudge = 0;

  unsigned int pebble_angle = TRIG_MAX_ANGLE * hand_angle / 360;

  rotbmp_pair_layer_set_angle(&hand_image_container->layer, pebble_angle);

  x_fudge += offset.x * cos_lookup(pebble_angle) / TRIG_MAX_RATIO - offset.y * sin_lookup(pebble_angle) / TRIG_MAX_RATIO;
  y_fudge += offset.x * sin_lookup(pebble_angle) / TRIG_MAX_RATIO + offset.y * cos_lookup(pebble_angle) / TRIG_MAX_RATIO;

  // (144 = screen width, 168 = screen height)
  hand_image_container->layer.layer.frame.origin.x = (144/2) - (hand_image_container->layer.layer.frame.size.w/2) + x_fudge;
  hand_image_container->layer.layer.frame.origin.y = (168/2) - (hand_image_container->layer.layer.frame.size.h/2) + y_fudge;

  layer_mark_dirty(&hand_image_container->layer.layer);
}

void set_hand_text(TextLayer *text_layer, unsigned int hand_angle, char *text, GPoint offset, GSize size) {
  signed short x_fudge = (144/2);
  signed short y_fudge = (168/2);

  unsigned int pebble_angle = TRIG_MAX_ANGLE * hand_angle / 360;

  x_fudge += offset.x * cos_lookup(pebble_angle) / TRIG_MAX_RATIO - offset.y * sin_lookup(pebble_angle) / TRIG_MAX_RATIO;
  y_fudge += offset.x * sin_lookup(pebble_angle) / TRIG_MAX_RATIO + offset.y * cos_lookup(pebble_angle) / TRIG_MAX_RATIO;

  x_fudge -= size.w / 2;
  y_fudge -= size.h / 2;

  layer_set_frame(&text_layer->layer, GRect(x_fudge, y_fudge, size.w, size.h));
  text_layer_set_text(text_layer, text);
}

void update_rings() {
  PblTm t;

  get_time(&t);

  int hourAngle = ((t.tm_hour % 12) * 30) + (t.tm_min / 2);
  int minAngle = t.tm_min * 6;

  static char hour_str[] = "00";
  static char minute_str[] = "00";

  char *hour_format;
  if (clock_is_24h_style()) {
    hour_format = "%k";
  } else {
    hour_format = "%l";
  }

  string_format_time(hour_str, sizeof(hour_str), hour_format, &t);
  string_format_time(minute_str, sizeof(minute_str), "%M", &t);

  set_hand_text(&hourText, hourAngle, hour_str, GPoint(0,-55), GSize(26,26));
  set_hand_text(&minuteText, minAngle, minute_str, GPoint(0,-25), GSize(18,18));

  set_hand_angle(&hourImage, hourAngle, GPoint(0, -9));
  set_hand_angle(&minuteImage, minAngle, GPoint(0, -6));
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;
  (void)t;

  update_rings();
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Orbit");
  window_stack_push(&window, false /* Not Animated */);
  window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&RESOURCES);

  rotbmp_pair_init_container(RESOURCE_ID_IMAGE_MINUTE_RING_WHITE, RESOURCE_ID_IMAGE_MINUTE_RING_BLACK, &minuteImage);
  layer_add_child(&window.layer, &minuteImage.layer.layer);

  rotbmp_pair_init_container(RESOURCE_ID_IMAGE_HOUR_RING_WHITE, RESOURCE_ID_IMAGE_HOUR_RING_BLACK, &hourImage);
  layer_add_child(&window.layer, &hourImage.layer.layer);

  text_layer_init(&minuteText, GRect(0,0,0,0));
  text_layer_set_text_color(&minuteText, GColorWhite);
  text_layer_set_background_color(&minuteText, GColorClear);
  text_layer_set_text_alignment(&minuteText, GTextAlignmentCenter);
  text_layer_set_font(&minuteText, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_14)));
  layer_add_child(&window.layer, &minuteText.layer);

  text_layer_init(&hourText, GRect(0,0,0,0));
  text_layer_set_text_color(&hourText, GColorWhite);
  text_layer_set_background_color(&hourText, GColorClear);
  text_layer_set_text_alignment(&hourText, GTextAlignmentCenter);
  text_layer_set_font(&hourText, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_22)));
  layer_add_child(&window.layer, &hourText.layer);

  update_rings();
}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  rotbmp_pair_deinit_container(&minuteImage);
  rotbmp_pair_deinit_container(&hourImage);
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
