#pragma once

#include <pebble.h>

#define NUM_CLOCK_TICKS 11

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

static const GPathInfo GMT_HAND_POINTS = {
	7, (GPoint []) {
		{  7, -70 },
		{  2, -77 },
		{  2, -85 },
		{ -2, -85 },
		{ -2, -77 },
		{ -7, -70 },
		{  7, -70 },
	}
};
