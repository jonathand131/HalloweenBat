# HaloweenBat

This is a small DIY project to create a IoT Haloween bat.

## Project description

The thing integrates LEDs, a NodeMCU and a 9V battery in a cardboard tube.
It uses Blynk (http://www.blynk.cc/) to receive commands from a smartphone.

The Haloween bat has different operating modes:
 * Off
 * Fuzzy: random changes of brightness level
 * Manual: lighting sets using the brightness level manually configured
 * Blink: the two LEDs blink simulataneously
 * Blink LR: alternates the two LEDs

The brightness can be changed from the smartphone and applies to all modes (except Off ;-) ).

## Software

The NodeMCU firmware uses ESP8266 Arduino framework (https://github.com/esp8266/Arduino) and building is done using Platform IO (http://platformio.org/).
It integrates the Blynk library (https://github.com/blynkkk/blynk-library) to communicate with the Blynk public cloud to receive commands from the smartphone.
Additionally, it support OTA updates via WiFi which is pretty handy when the thing is closed and currently used as decoration.

## Demo

[![IMAGE ALT TEXT HERE](http://img.youtube.com/vi/8HkiVJLH2uw/0.jpg)](http://www.youtube.com/watch?v=8HkiVJLH2uw)
(Click on the picture to watch the video on Youtube)
