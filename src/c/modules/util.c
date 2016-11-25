#include "util.h"
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>


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

int32_t get_angle(int value, int total) {
	return (value * 360) / total;
}
