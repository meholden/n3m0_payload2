# n3m0_payload

Software for autonomous boat n3m0's payload computer.
Uses nodemcu lua board, arduino development software.

When connected to a ds18b20 temperature sensor and a pixhawk autopilot, this software will take temperature measurements when the drone reaches a waypoint.  The data files are stored locally and available through a webpage.  The board starts up in AP mode and you can give credentials for it to connect to a wifi network.

Written by Mike Holden.
www.holdentechnology.com

Here's the schematic:

![circuit diagram](https://github.com/meholden/n3m0_payload2/raw/master/payload.png)
