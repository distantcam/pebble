#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define SHOW_SECONDS false


#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168

#define HOUR_RADIUS -55
#define MIN_RADIUS -24
#define SEC_RADIUS -20


#define MY_UUID { 0x7A, 0xCB, 0x68, 0x2C, 0x4E, 0x6C, 0x4A, 0xDA, 0x93, 0x36, 0x00, 0x47, 0xFF, 0x28, 0xEF, 0xB5 }
PBL_APP_INFO(MY_UUID,
#if SHOW_SECONDS == true
             "Orbit + Seconds", 
#else
             "Orbit",
#endif
             "Cameron MacFarland",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

BmpContainer faceImage;
RotBmpPairContainer minuteImage;
RotBmpPairContainer hourImage;

TextLayer hourText;
TextLayer minuteText;

#if SHOW_SECONDS == true
Layer secondLayer;
#endif


GPoint get_angle_point(unsigned int angle, GPoint offset) {
  unsigned int pebble_angle = TRIG_MAX_ANGLE * angle / 360;

  return GPoint(
    offset.x * cos_lookup(pebble_angle) / TRIG_MAX_RATIO - offset.y * sin_lookup(pebble_angle) / TRIG_MAX_RATIO,
    offset.x * sin_lookup(pebble_angle) / TRIG_MAX_RATIO + offset.y * cos_lookup(pebble_angle) / TRIG_MAX_RATIO);
}


#if SHOW_SECONDS == true
void secondLayer_Update_Callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;

  get_time(&t);

  int minAngle = t.tm_min * 6;
  int secAngle = t.tm_sec * 6;

  GPoint center = grect_center_point(&me->frame);

  GPoint toMinute = get_angle_point(minAngle, GPoint(0, MIN_RADIUS));
  GPoint toSec = get_angle_point(secAngle, GPoint(0, SEC_RADIUS));

  graphics_context_set_fill_color(ctx, GColorWhite);

  for (int x = -1; x < 3; x++)
    for (int y = -1; y < 3; y++)
      if (x + y > -2 && x + y < 4 && x - y < 3 && y - x < 3)
        graphics_draw_pixel(ctx, GPoint(center.x + toMinute.x + toSec.x + x, center.y + toMinute.y + toSec.y + y));
}
#endif


void set_hand_angle(RotBmpPairContainer *hand_image_container, unsigned int hand_angle, GPoint offset) {
  GPoint fudge = get_angle_point(hand_angle, offset);

  // (144 = screen width, 168 = screen height)
  hand_image_container->layer.layer.frame.origin.x = (SCREEN_WIDTH/2) - (hand_image_container->layer.layer.frame.size.w/2) + fudge.x;
  hand_image_container->layer.layer.frame.origin.y = (SCREEN_HEIGHT/2) - (hand_image_container->layer.layer.frame.size.h/2) + fudge.y;

  layer_mark_dirty(&hand_image_container->layer.layer);
}


void set_hand_text(TextLayer *text_layer, unsigned int hand_angle, char *text, GPoint offset, GSize size) {
  GPoint fudge = get_angle_point(hand_angle, offset);

  // (144 = screen width, 168 = screen height)
  signed short x_fudge = (SCREEN_WIDTH/2) - (size.w/2) + fudge.x;
  signed short y_fudge = (SCREEN_HEIGHT/2) - (size.h/2) + fudge.y;

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
    hour_format = "%H";
  } else {
    hour_format = "%I";
  }

  string_format_time(hour_str, sizeof(hour_str), hour_format, &t);
  string_format_time(minute_str, sizeof(minute_str), "%M", &t);

  if (!clock_is_24h_style() && (hour_str[0] == '0'))
    memmove(hour_str, &hour_str[1], sizeof(hour_str) - 1);

  set_hand_text(&hourText, hourAngle, hour_str, GPoint(0, HOUR_RADIUS), GSize(28,28));
  set_hand_text(&minuteText, minAngle, minute_str, GPoint(0, MIN_RADIUS), GSize(18,18));

  set_hand_angle(&hourImage, hourAngle, GPoint(0, HOUR_RADIUS));
  set_hand_angle(&minuteImage, minAngle, GPoint(0, MIN_RADIUS));
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;
  (void)t;

  if ((t->units_changed & MINUTE_UNIT) != 0) {
    update_rings();
  }

#if SHOW_SECONDS == true
  if ((t->units_changed & SECOND_UNIT) != 0) {
    layer_mark_dirty(&secondLayer);
  }
#endif
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Orbit");
  window_stack_push(&window, false /* Not Animated */);
  window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&APP_RESOURCES);

  bmp_init_container(RESOURCE_ID_IMAGE_FACE, &faceImage);
  layer_add_child(&window.layer, &faceImage.layer.layer);

  rotbmp_pair_init_container(RESOURCE_ID_IMAGE_HOUR_PLANET_WHITE, RESOURCE_ID_IMAGE_HOUR_PLANET_BLACK, &hourImage);
  layer_add_child(&window.layer, &hourImage.layer.layer);

  rotbmp_pair_init_container(RESOURCE_ID_IMAGE_MINUTE_PLANET_WHITE, RESOURCE_ID_IMAGE_MINUTE_PLANET_BLACK, &minuteImage);
  layer_add_child(&window.layer, &minuteImage.layer.layer);

  text_layer_init(&hourText, GRect(0,0,0,0));
  text_layer_set_text_color(&hourText, GColorWhite);
  text_layer_set_background_color(&hourText, GColorClear);
  text_layer_set_text_alignment(&hourText, GTextAlignmentCenter);
  text_layer_set_font(&hourText, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_22)));
  layer_add_child(&window.layer, &hourText.layer);

  text_layer_init(&minuteText, GRect(0,0,0,0));
  text_layer_set_text_color(&minuteText, GColorWhite);
  text_layer_set_background_color(&minuteText, GColorClear);
  text_layer_set_text_alignment(&minuteText, GTextAlignmentCenter);
  text_layer_set_font(&minuteText, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_14)));
  layer_add_child(&window.layer, &minuteText.layer);

#if SHOW_SECONDS == true
  layer_init(&secondLayer, window.layer.frame);
  secondLayer.update_proc = &secondLayer_Update_Callback;
  layer_add_child(&window.layer, &secondLayer);
#endif

  update_rings();
}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  bmp_deinit_container(&faceImage);
  rotbmp_pair_deinit_container(&minuteImage);
  rotbmp_pair_deinit_container(&hourImage);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_tick,
#if SHOW_SECONDS == true
      .tick_units = SECOND_UNIT
#else
      .tick_units = MINUTE_UNIT
#endif
    }
  };
  app_event_loop(params, &handlers);
}
