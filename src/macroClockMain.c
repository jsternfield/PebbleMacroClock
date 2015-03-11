#include "pebble.h"

// This defines graphics path information to be loaded as a path later
static const GPathInfo LINE_PATH_POINTS = {
  // This is the amount of points
  4,
  // A path can be concave, but it should not twist on itself
  // The points should be defined in clockwise order due to the rendering
  // implementation. Counter-clockwise will work in older firmwares, but
  // it is not officially supported
  (GPoint []) {
    {-3, 125},
    {3, 125},
    {3, -125},
	{-3, -125}
  }
};

const int midWidth = 47;
const int midHeight = 59;

static Window *s_main_window;
static TextLayer * s_time_layer;
static Layer *s_path_layer;
static GPath *s_line_path;
static int s_path_angle;

// This is the layer update callback which is called on render updates
static void path_layer_update_callback(Layer *layer, GContext *ctx) {
	// You can rotate the path before rendering
	time_t tempTime = time(NULL);
	struct tm * timeStruct = localtime(&tempTime);

	s_path_angle = (((timeStruct->tm_hour % 12) * 60) + timeStruct->tm_min) * 360 / (12 * 60);

	gpath_rotate_to(s_line_path, (TRIG_MAX_ANGLE / 360) * s_path_angle);

	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_fill_color(ctx, GColorBlack);
	gpath_draw_outline(ctx, s_line_path);
	gpath_draw_filled(ctx, s_line_path);
}

static void update_time() {
	time_t tempTime = time(NULL);
	struct tm * tick_time = localtime(&tempTime);

	Layer * timeLayer = text_layer_get_layer(s_time_layer);
	
	static char buffer[] = "00";
	
	strftime(buffer, sizeof("00"), "%I", tick_time);
	
	layer_set_frame(timeLayer, GRect(midWidth,midHeight,50,50));
	text_layer_set_text(s_time_layer, buffer);
	
	layer_mark_dirty(timeLayer);
	
	s_path_angle = (((tick_time->tm_hour % 12) * 60) + tick_time->tm_min) * 360 / (12 * 60);
	gpath_rotate_to(s_line_path, (TRIG_MAX_ANGLE / 360) * s_path_angle);
	layer_mark_dirty(s_path_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

static void main_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
	s_time_layer = text_layer_create(GRect(47, 0, 50, 50));
	text_layer_set_background_color(s_time_layer, GColorBlack);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_text(s_time_layer, "12");

	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
	s_path_layer = layer_create(bounds);
	layer_set_update_proc(s_path_layer, path_layer_update_callback);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	layer_add_child(window_layer, s_path_layer);

	// Move all paths to the center of the screen
	gpath_move_to(s_line_path, GPoint(bounds.size.w/2, bounds.size.h/2));
}

static void main_window_unload(Window *window) {
	layer_destroy(s_path_layer);
}

static void init() {
	// Pass the corresponding GPathInfo to initialize a GPath
	s_line_path = gpath_create(&LINE_PATH_POINTS);

	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	// Create Window
	s_main_window = window_create();
	window_set_background_color(s_main_window, GColorBlack);
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload,
	});
	window_stack_push(s_main_window, true);
}

static void deinit() {
	window_destroy(s_main_window);

	gpath_destroy(s_line_path);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

