#include "main.h"

#include <pebble.h>
#include <pebble-fctx/fctx.h>

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
	HAND_ACCENT_COLOR,
	GMT_HAND_COLOR,
	HOUR_TEXT_COLOR,
	PALETTE_SIZE
};

GColor g_palette[PALETTE_SIZE];

bool use_seconds = false;
bool hour_ticks = true;
bool minute_ticks = true;


// --------------------------------------------------------------------------
// Utility functions.
// --------------------------------------------------------------------------
#define FIXED_POINT_SCALE 16
#define INT_TO_FIXED(a) ((a) * FIXED_POINT_SCALE)
#define FIXED_TO_INT(a) ((a) / FIXED_POINT_SCALE)
#define FIXED_MULTIPLY(a, b) (((a) * (b)) / FIXED_POINT_SCALE)

#define FPoint(x, y) ((FPoint){(x), (y)})
#define FPointI(x, y) ((FPoint){INT_TO_FIXED(x), INT_TO_FIXED(y)})


static inline FPoint clockToCartesian(FPoint center, fixed_t radius, int32_t angle) {
	FPoint pt;
	int32_t c = cos_lookup(angle);
	int32_t s = sin_lookup(angle);
	pt.x = center.x + s * radius / TRIG_MAX_RATIO;
	pt.y = center.y - c * radius / TRIG_MAX_RATIO;
	return pt;
}

static int32_t get_angle(int value, int total) {
	return (value * 360) / total;
}


// --------------------------------------------------------------------------
// Main functions.
// --------------------------------------------------------------------------

static void bg_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	FPoint center = FPointI(bounds.size.w / 2, bounds.size.h / 2);

	// minute ticks
	// create all but 1200
	if (minute_ticks) {
		graphics_context_set_fill_color(ctx, g_palette[BEZEL_COLOR]);
		graphics_context_set_stroke_width(ctx, 1);
		graphics_context_set_stroke_color(ctx, GColorDarkGray);
		for (int i = 1; i < 60; i++) {
			GRect frame = grect_inset(bounds, GEdgeInsets(3));
			GRect frame2 = grect_inset(bounds, GEdgeInsets(5));
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
			GRect frame2 = grect_inset(bounds, GEdgeInsets(6));
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

	// init fctx
	FContext fctx;
	fctx_init_context(&fctx, ctx);
	FPoint f_center = FPointI(bounds.size.w / 2, bounds.size.h / 2);
	fixed_t f_scale = 16;


	////////////////////////////////////////////////////////////////////////////
	// GMT pointer
	////////////////////////////////////////////////////////////////////////////

	fctx_set_fill_color(&fctx, g_palette[GMT_HAND_COLOR]);
	fctx_set_offset(&fctx, f_center);
	fctx_set_scale(&fctx, FPointOne, FPointOne);
	fctx_set_rotation(&fctx, gmt_angle);

	fctx_begin_fill(&fctx);
	fctx_move_to (&fctx, FPointI( 6, -70));
	fctx_line_to (&fctx, FPointI( 1, -77));
	fctx_line_to (&fctx, FPointI( 1, -85));
	fctx_line_to (&fctx, FPointI(-1, -85));
	fctx_line_to (&fctx, FPointI(-1, -77));
	fctx_line_to (&fctx, FPointI(-6, -70));
	fctx_line_to (&fctx, FPointI( 6, -70));
	fctx_end_fill(&fctx);


	////////////////////////////////////////////////////////////////////////////
	// hour hand
	////////////////////////////////////////////////////////////////////////////

	fctx_set_fill_color(&fctx, g_palette[HAND_COLOR]);
	fctx_set_offset(&fctx, f_center);
	fctx_set_scale(&fctx, FPointOne, FPointOne);
	fctx_set_rotation(&fctx, hour_angle);

	fctx_begin_fill(&fctx);
	fctx_move_to (&fctx, FPointI( 3, -10));
	fctx_line_to (&fctx, FPointI( 5, -12));
	fctx_line_to (&fctx, FPointI( 3, -45));
	fctx_line_to (&fctx, FPointI( 0, -50));
	fctx_line_to (&fctx, FPointI(-3, -45));
	fctx_line_to (&fctx, FPointI(-5, -12));
	fctx_line_to (&fctx, FPointI(-3, -10));
	fctx_line_to (&fctx, FPointI( 3, -10));
	fctx_end_fill(&fctx);

	////////////////////////////////////////////////////////////////////////////
	// minute hand
	////////////////////////////////////////////////////////////////////////////

	fctx_set_fill_color(&fctx, g_palette[HAND_COLOR]);
	fctx_set_offset(&fctx, f_center);
	fctx_set_scale(&fctx, FPointOne, FPointOne);
	fctx_set_rotation(&fctx, minute_angle);

	fctx_begin_fill(&fctx);
	fctx_move_to (&fctx, FPointI( 3, -10));
	fctx_line_to (&fctx, FPointI( 5, -12));
	fctx_line_to (&fctx, FPointI( 3, -77));
	fctx_line_to (&fctx, FPointI( 0, -82));
	fctx_line_to (&fctx, FPointI(-3, -77));
	fctx_line_to (&fctx, FPointI(-5, -12));
	fctx_line_to (&fctx, FPointI(-3, -10));
	fctx_line_to (&fctx, FPointI( 3, -10));
	fctx_end_fill(&fctx);

	fctx_set_fill_color(&fctx, GColorDarkGray);
	fctx_begin_fill(&fctx);
	fctx_plot_circle(&fctx, &f_center, 6*16);
	fctx_end_fill(&fctx);

	////////////////////////////////////////////////////////////////////////////
	// second hand
	////////////////////////////////////////////////////////////////////////////

	if (use_seconds == true) {
		const int16_t second_hand_length = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 8, bounds.size.w / 2);
		GPoint second_hand = radial_gpoint(center, second_hand_length - 15, second_angle);
		GPoint second_hand_point = radial_gpoint(center, second_hand_length, second_angle);

		graphics_context_set_stroke_width(ctx, 1);
		graphics_context_set_stroke_color(ctx, GColorRed);
		graphics_draw_line(ctx, second_hand, second_hand_point);
		graphics_context_set_stroke_color(ctx, GColorDarkGray);
		graphics_draw_line(ctx, second_hand, center);
	}

	// deinit fctx
	fctx_deinit_context(&fctx);
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
	text_layer_set_background_color(s_num_label, GColorBlack);
	text_layer_set_text_color(s_num_label, GColorLightGray);
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
	g_palette[      BEZEL_COLOR] = GColorWhite;
	g_palette[       FACE_COLOR] = GColorBlack;
	g_palette[		 HAND_COLOR] = GColorWhite;
	g_palette[HAND_ACCENT_COLOR] = GColorLightGray;
	g_palette[   GMT_HAND_COLOR] = GColorChromeYellow;
	g_palette[  HOUR_TEXT_COLOR] = PBL_IF_COLOR_ELSE(GColorBlack, GColorBlack);

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
