#pragma once

#include "pebble.h"

#define NUM_CLOCK_TICKS 11

static const struct GPathInfo ANALOG_BG_POINTS[] = {
	{
		5, (GPoint []) {
			{66, -3},
			{71, -3},
			{71, 1},
			{68, 1},
			{66, 0}
		}
	},
	{
		5, (GPoint []) {
			{77, -3},
			{72, -3},
			{72, 1},
			{75, 1},
			{77, 0}
		}
	},
};

static const GPathInfo GMT_HAND_POINTS = {
	7, (GPoint []) {
		{ 0, -70 },
		{ 7, -70 },
		{ 2, -77 },
		{ 2, -85 },
		{ -3, -85 },
		{ -3, -77 },
		{ -8, -70 },
	}
};
