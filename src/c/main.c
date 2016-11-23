#include "main.h"

#include <pebble.h>
// #include <pebble-fctx/fctx.h>

#include "modules/util.h"

static Window *s_window;
static Layer *bg_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_num_label;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_gmt_arrow;
static char s_num_buffer[4];

enum Palette {
	BEZEL_COLOR,
	FACE_COLOR,
	HAND_COLOR,
	SECOND_HAND_COLOR,
	GMT_HAND_COLOR,
	COMPLICATION_COLOR,
	PALETTE_SIZE
};

GColor g_palette[PALETTE_SIZE];

bool use_seconds = false;
bool hour_ticks = true;
bool minute_ticks = true;


// --------------------------------------------------------------------------
// Main functions.
// --------------------------------------------------------------------------

static void bg_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	// FPoint center = FPointI(bounds.size.w / 2, bounds.size.h / 2);
	GPoint center = grect_center_point(&bounds);

	// minute ticks
	// create all but 1200
	if (minute_ticks) {
		graphics_context_set_fill_color(ctx, g_palette[BEZEL_COLOR]);
		graphics_context_set_stroke_width(ctx, 1);
		graphics_context_set_stroke_color(ctx, g_palette[BEZEL_COLOR]);
		for (int i = 1; i < 60; i++) {
			GRect frame = grect_inset(bounds, GEdgeInsets(3));
			GRect frame2 = grect_inset(bounds, GEdgeInsets(6));
			GPoint pos = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE( get_angle(i, 60) ));
			GPoint pos2 = gpoint_from_polar(frame2, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE( get_angle(i, 60) ));

			graphics_draw_line(ctx, pos, pos2);
		}
	}

	// 12h ticks
	// create all but 1200
	if (hour_ticks) {
		// Double-tick first
		graphics_context_set_fill_color(ctx, g_palette[BEZEL_COLOR]);
		for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
			const int x_offset = PBL_IF_ROUND_ELSE(18, 0);
			const int y_offset = PBL_IF_ROUND_ELSE(6, 0);
			gpath_move_to(s_tick_paths[i], GPoint(x_offset, y_offset));
			gpath_draw_filled(ctx, s_tick_paths[i]);
		}

		graphics_context_set_stroke_width(ctx, 3);
		graphics_context_set_stroke_color(ctx, g_palette[BEZEL_COLOR]);

		for (int i = 1; i < 12; i++) {
			GRect frame = grect_inset(bounds, GEdgeInsets(3));
			GRect frame2 = grect_inset(bounds, GEdgeInsets(7));
			GPoint pos = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE( get_angle(i, 12) ));
			GPoint pos2 = gpoint_from_polar(frame2, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE( get_angle(i, 12) ));

			graphics_draw_line(ctx, pos, pos2);
		}
	}
}

static GPoint radial_gpoint(GPoint center, int16_t length_from_center, int32_t angle) {
	return (GPoint) {
		.x = (int16_t)(sin_lookup(angle) * (int32_t)length_from_center / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(angle) * (int32_t)length_from_center / TRIG_MAX_RATIO) + center.y,
	};
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);

	// time stuff
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	struct tm *gmt = gmtime(&now);

	// angles
	int32_t gmt_angle = (TRIG_MAX_ANGLE * (((gmt->tm_hour) * 6) + (gmt->tm_min / 10))) / (24 * 6);
	int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
	int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
	int32_t hour_angle = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);

	// lengths
	int16_t minute_length = 75;
	int16_t hour_length = 50;

	// ends
	GPoint minute_hand_point = radial_gpoint(center, minute_length, minute_angle);
	GPoint hour_hand_point = radial_gpoint(center, hour_length, hour_angle);

	// init fctx
	// FContext fctx;
	// fctx_init_context(&fctx, ctx);
	// FPoint f_center = FPointI(bounds.size.w / 2, bounds.size.h / 2);
	// fixed_t f_scale = 16;


	////////////////////////////////////////////////////////////////////////////
	// GMT pointer
	////////////////////////////////////////////////////////////////////////////

	graphics_context_set_fill_color(ctx, g_palette[GMT_HAND_COLOR]);
	gpath_rotate_to(s_gmt_arrow, gmt_angle);
	gpath_draw_filled(ctx, s_gmt_arrow);


	// fctx_set_fill_color(&fctx, g_palette[GMT_HAND_COLOR]);
	// fctx_set_offset(&fctx, f_center);
	// fctx_set_scale(&fctx, FPointOne, FPointOne);
	// fctx_set_rotation(&fctx, gmt_angle);
	//
	// fctx_begin_fill(&fctx);
	// fctx_move_to (&fctx, FPointI( 6, -70));
	// fctx_line_to (&fctx, FPointI( 1, -77));
	// fctx_line_to (&fctx, FPointI( 1, -85));
	// fctx_line_to (&fctx, FPointI(-1, -85));
	// fctx_line_to (&fctx, FPointI(-1, -77));
	// fctx_line_to (&fctx, FPointI(-6, -70));
	// fctx_line_to (&fctx, FPointI( 6, -70));
	// fctx_end_fill(&fctx);


	////////////////////////////////////////////////////////////////////////////
	// hour hand
	////////////////////////////////////////////////////////////////////////////

	graphics_context_set_stroke_width(ctx, 6);
	graphics_context_set_stroke_color(ctx, g_palette[HAND_COLOR]);
	graphics_draw_line(ctx, center, hour_hand_point);

	// fixed_t h_bez = 2;
	// fixed_t h_ins = 6;
	//
	// fctx_set_fill_color(&fctx, g_palette[HAND_COLOR]);
	// fctx_set_offset(&fctx, f_center);
	// fctx_set_scale(&fctx, FPointOne, FPointOne);
	// fctx_set_rotation(&fctx, hour_angle);
	//
	// fctx_begin_fill(&fctx);
	// fctx_move_to (&fctx, FPointI( 3,		 -h_ins));
	// fctx_line_to (&fctx, FPointI( 3 + h_bez, -h_bez - h_ins ));
	// fctx_line_to (&fctx, FPointI( 3,		 -45));
	// fctx_line_to (&fctx, FPointI( 0,		 -50));
	// fctx_line_to (&fctx, FPointI(-3,		 -45));
	// fctx_line_to (&fctx, FPointI(-3 - h_bez, -h_bez - h_ins ));
	// fctx_line_to (&fctx, FPointI(-3,		 -h_ins));
	// fctx_line_to (&fctx, FPointI( 3,		 -h_ins));
	// fctx_end_fill(&fctx);

	////////////////////////////////////////////////////////////////////////////
	// minute hand
	////////////////////////////////////////////////////////////////////////////

	graphics_context_set_stroke_width(ctx, 6);
	graphics_context_set_stroke_color(ctx, g_palette[HAND_COLOR]);
	graphics_draw_line(ctx, center, minute_hand_point);

	// fctx_set_fill_color(&fctx, g_palette[HAND_COLOR]);
	// fctx_set_offset(&fctx, f_center);
	// fctx_set_scale(&fctx, FPointOne, FPointOne);
	// fctx_set_rotation(&fctx, minute_angle);
	//
	// fctx_begin_fill(&fctx);
	// fctx_move_to (&fctx, FPointI( 3,		 -h_ins));
	// fctx_line_to (&fctx, FPointI( 3 + h_bez, -h_bez - h_ins ));
	// fctx_line_to (&fctx, FPointI( 3,		 -73));
	// fctx_line_to (&fctx, FPointI( 0,		 -78));
	// fctx_line_to (&fctx, FPointI(-3,		 -73));
	// fctx_line_to (&fctx, FPointI(-3 - h_bez, -h_bez - h_ins ));
	// fctx_line_to (&fctx, FPointI(-3,		 -h_ins));
	// fctx_line_to (&fctx, FPointI( 3,		 -h_ins));
	// fctx_end_fill(&fctx);
	//
	// fctx_set_fill_color(&fctx, GColorDarkGray);
	// fctx_begin_fill(&fctx);
	// fctx_plot_circle(&fctx, &f_center, 5*16);
	// fctx_end_fill(&fctx);

	////////////////////////////////////////////////////////////////////////////
	// second hand
	////////////////////////////////////////////////////////////////////////////

	if (use_seconds == true) {
		const int16_t second_hand_length = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 8, bounds.size.w / 2);
		GPoint second_hand = radial_gpoint(center, second_hand_length - 15, second_angle);
		GPoint second_hand_point = radial_gpoint(center, second_hand_length, second_angle);

		graphics_context_set_stroke_width(ctx, 1);
		graphics_context_set_stroke_color(ctx, g_palette[SECOND_HAND_COLOR]);
		graphics_draw_line(ctx, second_hand, second_hand_point);
	}


	////////////////////////////////////////////////////////////////////////////
	// center pivot
	////////////////////////////////////////////////////////////////////////////
	graphics_context_set_fill_color(ctx, g_palette[FACE_COLOR]);
	graphics_fill_circle(ctx, center, 7);


	// deinit fctx
	// fctx_deinit_context(&fctx);
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

	bg_layer = layer_create(bounds);
	layer_set_update_proc(bg_layer, bg_update_proc);
	layer_add_child(window_layer, bg_layer);

	s_date_layer = layer_create(bounds);
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(window_layer, s_date_layer);

	s_num_label = text_layer_create(PBL_IF_ROUND_ELSE(
		GRect(140, 77, 18, 20),
		GRect(73, 114, 18, 20)));
	text_layer_set_text(s_num_label, s_num_buffer);
	text_layer_set_background_color(s_num_label, g_palette[FACE_COLOR]);
	text_layer_set_text_color(s_num_label, g_palette[COMPLICATION_COLOR]);
	text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

	layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));

	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(window_layer, s_hands_layer);
}

static void window_unload(Window *window) {
	layer_destroy(bg_layer);
	layer_destroy(s_date_layer);

	text_layer_destroy(s_num_label);

	layer_destroy(s_hands_layer);
}

static void init() {
	g_palette[       BEZEL_COLOR] = GColorWhite;
	g_palette[        FACE_COLOR] = GColorBlack;
	g_palette[        HAND_COLOR] = GColorWhite;
	g_palette[ SECOND_HAND_COLOR] = GColorRed;
	g_palette[    GMT_HAND_COLOR] = GColorChromeYellow;
	g_palette[COMPLICATION_COLOR] = GColorLightGray;

	s_window = window_create();
	window_set_background_color(s_window, g_palette[FACE_COLOR]);
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(s_window, true);

	s_num_buffer[0] = '\0';

	// init hand paths
	s_gmt_arrow = gpath_create(&GMT_HAND_POINTS);

	Layer *window_layer = window_get_root_layer(s_window);
	GRect bounds = layer_get_bounds(window_layer);
	GPoint center = grect_center_point(&bounds);
	gpath_move_to(s_gmt_arrow, center);

	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
		s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
	}

	if (use_seconds == false) {
		tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	} else {
		tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
	}
}

static void deinit() {
	gpath_destroy(s_gmt_arrow);

	tick_timer_service_unsubscribe();
	window_destroy(s_window);
}

int main() {
	init();
	app_event_loop();
	deinit();
}
