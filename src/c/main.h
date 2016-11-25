#pragma once

#include <pebble.h>

#define NUM_CLOCK_TICKS 2

static const struct GPathInfo ANALOG_BG_POINTS[] = {
	{
		5, (GPoint []) {
			{67, -3},
			{71, -3},
			{71, 3},
			{68, 3},
			{67, 2}
		}
	},
	{
		5, (GPoint []) {
			{76, -3},
			{72, -3},
			{72, 3},
			{75, 3},
			{76, 2}
		}
	},
};
