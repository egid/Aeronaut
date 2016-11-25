# Aeronaut

Aeronaut is a watch face inspired by pilot's watches. It's got a minimal bezel, large (yet precise) hands, and a 24-hour UTC (GMT) pointer. It is optimized for the Pebble Time Round, but works on all Pebble types (except classic) and is [available on the Pebble appstore](https://apps.getpebble.com/en_US/application/5830a01b00355a2f3a00004f?hardware=chalk).

![](aeronaut.png)

It's in very early stages right now; basically none of the planned features are in yet.

# Tasks

- [x] GMT pointer
- [x] Support platforms other than Chalk
- [x] Settings via Clay
	- [x] light/dark themes
	- [ ] colors
	- [ ] GMT hand time zone offset
	- [ ] active complications
	- [ ] low-power quiet time mode
	- [x] second hand (on/off)
	- [ ] tap/shake for second hand
- [ ] complications
	- [x] Date
	- [ ] Battery / Bluetooth
	- [ ] Health
	- [ ] Weather
	- [ ] Aviation weather (METAR)  - data from location or a locked station
- [x] `fctx` for better antialiasing
- [x] svg-derived assets


Aeronaut used [SimpleAnalog](https://github.com/pebble-examples/simple-analog/blob/master/LICENSE) as a starting point.
