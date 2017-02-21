IPStack for the Teensy 3.6

This is from https://github.com/manitou48/teensy3 where manitou48 has done all the hard work and recognition should go to them.
This is not forked as manitou48 repository has a lot of other Teensy libraries and files not relevent to this project.

Manitou48 build also require a make file setup, where as this copy should compile directly from the Arduino IDE.

The idea is to make this into an Ethernet Library where you can specify a define that will choose the underlying Hardware, be it a W5500, ENJ28j60 or LAN8720 transceiever on the T3.6