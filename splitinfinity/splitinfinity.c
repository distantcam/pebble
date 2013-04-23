#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0xC3, 0xE6, 0x7E, 0x32, 0x97, 0x7A, 0x48, 0xCC, 0xBF, 0x7E, 0xAA, 0xE5, 0xF5, 0xBC, 0x01, 0x61 }
PBL_APP_INFO(MY_UUID,
             "Split Infinity", "Cameron MacFarland",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

BmpContainer faceImage;
RotBmpContainer tophourImage;
RotBmpContainer topminuteImage;
RotBmpContainer bottomhourImage;
RotBmpContainer bottomminuteImage;


/* -------------- TODO: Remove this and use Public API ! ------------------- */

// from src/core/util/misc.h

#define MAX(a,b) (((a)>(b))?(a):(b))

// From src/fw/ui/rotate_bitmap_layer.c

//! newton's method for floor(sqrt(x)) -> should always converge
static int32_t integer_sqrt(int32_t x) {
  if (x < 0) {
    ////    PBL_LOG(LOG_LEVEL_ERROR, "Looking for sqrt of negative number");
    return 0;
  }

  int32_t last_res = 0;
  int32_t res = (x + 1)/2;
  while (last_res != res) {
    last_res = res;
    res = (last_res + x / last_res) / 2;
  }
  return res;
}

void rot_bitmap_set_src_ic(RotBitmapLayer *image, GPoint ic) {
  image->src_ic = ic;

  // adjust the frame so the whole image will still be visible
  const int32_t horiz = MAX(ic.x, abs(image->bitmap->bounds.size.w - ic.x));
  const int32_t vert = MAX(ic.y, abs(image->bitmap->bounds.size.h - ic.y));

  GRect r = layer_get_frame(&image->layer);
  //// const int32_t new_dist = integer_sqrt(horiz*horiz + vert*vert) * 2;
  const int32_t new_dist = (integer_sqrt(horiz*horiz + vert*vert) * 2) + 1; //// Fudge to deal with non-even dimensions--to ensure right-most and bottom-most edges aren't cut off.

  r.size.w = new_dist;
  r.size.h = new_dist;
  layer_set_frame(&image->layer, r);

  r.origin = GPoint(0, 0);
  ////layer_set_bounds(&image->layer, r);
  image->layer.bounds = r;

  image->dest_ic = GPoint(new_dist / 2, new_dist / 2);

  layer_mark_dirty(&(image->layer));
}

/* ------------------------------------------------------------------------- */

void set_hand_angle_top(RotBmpContainer *hand_image_container, unsigned int hand_angle) {

  signed short x_fudge = 0;
  signed short y_fudge = 0;


  hand_image_container->layer.rotation =  TRIG_MAX_ANGLE * hand_angle / 360;

  //
  // Due to rounding/centre of rotation point/other issues of fitting
  // square pixels into round holes by the time hands get to 6 and 9
  // o'clock there's off-by-one pixel errors.
  //
  // The `x_fudge` and `y_fudge` values enable us to ensure the hands
  // look centred on the minute marks at those points. (This could
  // probably be improved for intermediate marks also but they're not
  // as noticable.)
  //
  // I think ideally we'd only ever calculate the rotation between
  // 0-90 degrees and then rotate again by 90 or 180 degrees to
  // eliminate the error.
  //
  if (hand_angle == 180) {
    x_fudge = -1;
  } else if (hand_angle == 270) {
    y_fudge = -1;
  }

  // (144 = screen width, 168 = screen height)
  hand_image_container->layer.layer.frame.origin.x = (144/2) - (hand_image_container->layer.layer.frame.size.w/2) + x_fudge;
  hand_image_container->layer.layer.frame.origin.y = (0) - (hand_image_container->layer.layer.frame.size.h/2) + y_fudge;

  layer_mark_dirty(&hand_image_container->layer.layer);
}


void set_hand_angle_bottom(RotBmpContainer *hand_image_container, unsigned int hand_angle) {

  signed short x_fudge = 0;
  signed short y_fudge = 0;


  hand_image_container->layer.rotation =  TRIG_MAX_ANGLE * hand_angle / 360;

  //
  // Due to rounding/centre of rotation point/other issues of fitting
  // square pixels into round holes by the time hands get to 6 and 9
  // o'clock there's off-by-one pixel errors.
  //
  // The `x_fudge` and `y_fudge` values enable us to ensure the hands
  // look centred on the minute marks at those points. (This could
  // probably be improved for intermediate marks also but they're not
  // as noticable.)
  //
  // I think ideally we'd only ever calculate the rotation between
  // 0-90 degrees and then rotate again by 90 or 180 degrees to
  // eliminate the error.
  //
  if (hand_angle == 180) {
    x_fudge = -1;
  } else if (hand_angle == 270) {
    y_fudge = -1;
  }

  // (144 = screen width, 168 = screen height)
  hand_image_container->layer.layer.frame.origin.x = (144/2) - (hand_image_container->layer.layer.frame.size.w/2) + x_fudge;
  hand_image_container->layer.layer.frame.origin.y = (168) - (hand_image_container->layer.layer.frame.size.h/2) + y_fudge;

  layer_mark_dirty(&hand_image_container->layer.layer);
}


void updateHands() {
  PblTm t;

  get_time(&t);

  int hourAngle = ((t.tm_hour % 12) * 30) + (t.tm_min / 2);
  int minAngle = t.tm_min * 6;

  set_hand_angle_top(&tophourImage, hourAngle);
  set_hand_angle_top(&topminuteImage, minAngle); 

  set_hand_angle_bottom(&bottomhourImage, hourAngle);
  set_hand_angle_bottom(&bottomminuteImage, minAngle); 
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;
  (void)t;

  updateHands();
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Split Infinity");
  window_stack_push(&window, true /* Animated */);

  resource_init_current_app(&RESOURCES);

  bmp_init_container(RESOURCE_ID_IMAGE_FACE, &faceImage);
  layer_add_child(&window.layer, &faceImage.layer.layer);

  rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR, &tophourImage);
  rot_bitmap_set_src_ic(&tophourImage.layer, GPoint(4, 37));
  layer_add_child(&window.layer, &tophourImage.layer.layer);

  rotbmp_init_container(RESOURCE_ID_IMAGE_MINUTE, &topminuteImage);
  rot_bitmap_set_src_ic(&topminuteImage.layer, GPoint(2, 58));
  layer_add_child(&window.layer, &topminuteImage.layer.layer);

  rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR, &bottomhourImage);
  rot_bitmap_set_src_ic(&bottomhourImage.layer, GPoint(4, 37));
  layer_add_child(&window.layer, &bottomhourImage.layer.layer);

  rotbmp_init_container(RESOURCE_ID_IMAGE_MINUTE, &bottomminuteImage);
  rot_bitmap_set_src_ic(&bottomminuteImage.layer, GPoint(2, 58));
  layer_add_child(&window.layer, &bottomminuteImage.layer.layer);

  updateHands();
}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  bmp_deinit_container(&faceImage);
  rotbmp_deinit_container(&topminuteImage);
  rotbmp_deinit_container(&tophourImage);
  rotbmp_deinit_container(&bottomminuteImage);
  rotbmp_deinit_container(&bottomhourImage);
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
