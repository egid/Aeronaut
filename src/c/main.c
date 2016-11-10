#include "main.h"

#include "pebble.h"

static Window *s_window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_num_label;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow, *s_gmt_arrow;
static char s_num_buffer[4];

bool UseSeconds = false;

static int32_t get_angle_for_hour(int hour) {
  // Progress through 12 hours, out of 360 degrees
  return (hour * 360) / 12;
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
	graphics_context_set_fill_color(ctx, GColorWhite);
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
		const int x_offset = PBL_IF_ROUND_ELSE(18, 0);
		const int y_offset = PBL_IF_ROUND_ELSE(6, 0);
		gpath_move_to(s_tick_paths[i], GPoint(x_offset, y_offset));
		gpath_draw_filled(ctx, s_tick_paths[i]);
	}
	
	// create all but 1200
	for (int i = 1; i < 12; i++) {
		int hour_angle = get_angle_for_hour(i);
		GRect frame = grect_inset(bounds, GEdgeInsets(5));
		GPoint pos = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(hour_angle));

		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_circle(ctx, pos, 2);
	}
	for (int i = 0; i < 12; i++) {
		int hour_angle = get_angle_for_hour(i);
		GRect frame = grect_inset(bounds, GEdgeInsets(4));
		GPoint pos = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(hour_angle + 15));

		graphics_context_set_fill_color(ctx, GColorLightGray);
		graphics_fill_circle(ctx, pos, 1);
	}

}

static void hands_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);

	const int16_t second_hand_length = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 19, bounds.size.w / 2);

	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	struct tm *gmt = gmtime(&now);

	int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
	GPoint second_hand_point = {
		.x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
	};
	GPoint second_hand = {
		.x = (int16_t)(sin_lookup(second_angle) * ((int32_t)second_hand_length - 15) / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(second_angle) * ((int32_t)second_hand_length - 15) / TRIG_MAX_RATIO) + center.y,
	};


	// gmt hand
	int32_t gmt_angle = (TRIG_MAX_ANGLE * (((gmt->tm_hour) * 6) + (gmt->tm_min / 10))) / (24 * 6);

	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_fill_color(ctx, GColorOrange );
	gpath_rotate_to(s_gmt_arrow, gmt_angle);
	gpath_draw_filled(ctx, s_gmt_arrow);

	//   graphics_context_set_stroke_color(ctx, GColorWhite);


	graphics_context_set_stroke_width(ctx, 2);
	// hour hand
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_context_set_stroke_color(ctx, GColorLightGray);

	gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
	gpath_draw_filled(ctx, s_hour_arrow);
	gpath_draw_outline(ctx, s_hour_arrow);

	// minute hand
	graphics_context_set_fill_color(ctx, GColorWhite );
	graphics_context_set_stroke_color(ctx, GColorLightGray);

	gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
	gpath_draw_filled(ctx, s_minute_arrow);
	gpath_draw_outline(ctx, s_minute_arrow);

	// second hand
	if (UseSeconds == true) {	
		graphics_context_set_stroke_color(ctx, GColorRed);
		graphics_draw_line(ctx, second_hand_point, center);
		graphics_context_set_stroke_color(ctx, GColorBlack);
		graphics_draw_line(ctx, second_hand, center);
	}
}

static void date_update_proc(Layer *layer, GContext *ctx) {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	strftime(s_num_buffer, sizeof(s_num_buffer), "%d", t);
	text_layer_set_text(s_num_label, s_num_buffer);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	layer_mark_dirty(window_get_root_layer(s_window));
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	s_simple_bg_layer = layer_create(bounds);
	layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
	layer_add_child(window_layer, s_simple_bg_layer);

	s_date_layer = layer_create(bounds);
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(window_layer, s_date_layer);

	s_num_label = text_layer_create(PBL_IF_ROUND_ELSE(
		GRect(140, 77, 18, 20),
		GRect(73, 114, 18, 20)));
	text_layer_set_text(s_num_label, s_num_buffer);
	text_layer_set_background_color(s_num_label, GColorBlack);
	text_layer_set_text_color(s_num_label, GColorLightGray);
	text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

	layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));

	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(window_layer, s_hands_layer);
}

static void window_unload(Window *window) {
	layer_destroy(s_simple_bg_layer);
	layer_destroy(s_date_layer);

	text_layer_destroy(s_num_label);

	layer_destroy(s_hands_layer);
}

static void init() {
	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(s_window, true);

	s_num_buffer[0] = '\0';
			
	// init hand paths
	s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
	s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
	s_gmt_arrow = gpath_create(&GMT_HAND_POINTS);

	Layer *window_layer = window_get_root_layer(s_window);
	GRect bounds = layer_get_bounds(window_layer);
	GPoint center = grect_center_point(&bounds);
	gpath_move_to(s_minute_arrow, center);
	gpath_move_to(s_hour_arrow, center);
	gpath_move_to(s_gmt_arrow, center);

	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
		s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
	}

	if (UseSeconds == false) {
		tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	} else {
		tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
	}
}

static void deinit() {
	gpath_destroy(s_minute_arrow);
	gpath_destroy(s_hour_arrow);
	gpath_destroy(s_gmt_arrow);

	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
		gpath_destroy(s_tick_paths[i]);
	}

	tick_timer_service_unsubscribe();
	window_destroy(s_window);
}

int main() {
	init();
	app_event_loop();
	deinit();
}
