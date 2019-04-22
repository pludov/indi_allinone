# INDI AllInOne Arduino driver

## Introduction

This project provides software + hardware instructions to build a powerfull astronomy tool for astrophotography, including focuser, filterwheel, dew heater, humidity sensor, ...

All that hardware is driven by a single Teensy 3.2 board (any 3.3v Arduino with > 32k memory and enough PINs should do it. Mega2560 is also possible with small adaptations to the schematics for 5v).

Contrary to most similar projects, the logic is fully operated within the micro-controler, so it is very easy to add/modify features. There is no ping-pong between firmware and driver. That also means that you can quickly write code for new hardware without knowing too much about INDI driver. Exposes an INDI property, write code that reacts to its change and you're done. And thanks to the INDI protocol, you'll end up with a UI integration within your favorite astro software !

It is intended to be used together with an ARM board, via serial TTL. However, it can be quickly adapted to run over usb serial as well.

I personnaly use it with [Mobindi](https://github.com/pludov/mobindi), my other Astrophoto project, making a full lightweight astro setup, controlled from a single mobile phone.

## Quick start

### Building/Customizing the firmware

The firmware can be compiled using the Arduino IDE, and source code is very modular, making it easy to add your own hardware (more/less dewheater, ...) If you want to do so, open ucontroler.ino and locate the function "declareHardware".

The setup includes:
* Focuser (unipolar motor)
* Fitlerwheel (unipolar motor with hall sensor)
* DHT22 sensor (temperature/humidity)
* 2x DewHeater (PWM with temperature sensor)
* Watt/Current metter to monitor power consumption
* A clock to report uptime (& detect reset/crash)

You can tweak motor acceleration/speed for focuser and filterwheel. The default is conservative, with lower speed for focuser retractation (usefull if you have a refractor)

If you want to adapt for an unsupported hardware, you'll need to write an event driven class (ie avoid blocking for more than about 1 ms, to keep filterwheel/focuser happy). A good starting point is Status.h/Status.cpp that just reports a numeric value.

Focuser.cpp also has a simple example of how to control something from INDI property.

### Building the indi driver

The indi driver is just a relay to the micro-controler, meaning you probably won't have to modify it.

To build, issue the commands:
```shell
cd indi_driver
mkdir build
cd build
cmake ..
make
sudo make install
```

You'll now have a indi_allinone driver installed.

### Building the hardware

The electronics schematic are provided for Teensy 3.2.

![Schematics](focuser_arduino_v6_teensy.png?raw=true "Schematics")

The resulting board has roughtly the size of a raspberry pi, meaning you can probably stack them in a small box.

The filterwheel can be made from a manual filterwheel, adding a BYJ48 stepper and a hall sensor + magnet for calibration.
More details here (french) : http://pludov.blogspot.com/2016/08/roue-filtre-motorisee-la-mecanique.html


