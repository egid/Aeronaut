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
		{ 4, -75 },
		{ 0, -80 },
		{ -4, -75 },
	}
};

static const GPathInfo HOUR_HAND_POINTS = {
	5, (GPoint []){
		{ -5, -10 },
		{ 5, -10 },
		{ 4, -45 },
		{ 0, -50 },
		{ -4, -45 },
	}
};

static const GPathInfo GMT_HAND_POINTS = {
	7, (GPoint []) {
		{ 0, -70 },
		{ 9, -70 },
		{ 3, -76 },
		{ 3, -85 },
		{ -3, -85 },
		{ -3, -76 },
		{ -9, -70 },
	}
};

