#pragma once

#include "pebble.h"

#define NUM_CLOCK_TICKS 11

static const struct GPathInfo ANALOG_BG_POINTS[] = {
	{
		5, (GPoint []) {
			{66, -2},
			{71, -2},
			{71, 2},
			{68, 2},
			{66, 1}
		}
	},
	{
		5, (GPoint []) {
			{77, -2},
			{72, -2},
			{72, 2},
			{75, 2},
			{77, 1}
		}
	},
};

static const GPathInfo MINUTE_HAND_POINTS = {
	5, (GPoint []) {
		{ -6, -10 },
		{ 6, -10 },
		{ 4, -76 },
		{ 0, -80 },
		{ -4, -76 },
	}
};

static const GPathInfo HOUR_HAND_POINTS = {
	5, (GPoint []){
		{ -5, -10 },
		{ 5, -10 },
		{ 4, -46 },
		{ 0, -50 },
		{ -4, -46 },
	}
};

static const GPathInfo GMT_HAND_POINTS = {
	7, (GPoint []) {
		{ 0, -70 },
		{ 8, -70 },
		{ 2, -76 },
		{ 2, -85 },
		{ -2, -85 },
		{ -2, -76 },
		{ -8, -70 },
	}
};
