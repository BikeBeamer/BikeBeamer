# BikeBeamer

> This is a **very early work in progress**. Things aren't final and are due to change over time. **Don't waste your time to build it in its current state**.

## Overview

This is my next generation of bike POV displays based on [bkpvdsply](https://github.com/locxter/bkpvdsply). With much extended functionality, a more polished design and an easier user-experience, the goal for it is to extend my capabilities and experiment with building a "real product" that lives up to the expectations one might have.

## Dependencies

I generally try to minimize dependencies, but I'm a one man crew and can therefore only support Debian-based Linux distributions as I'm running one myself. Anyway, you need to have the following packages installed for everything to work properly:

- Arduino IDE as a way to compile the Arduino code. Install it with `sudo apt install arduino`.
- Arduino ESP32 LittleFS uploader for uploading the data. Install it via the [installation guide](https://github.com/lorol/arduino-esp32fs-plugin).
- Adafruit NeoPixel library for controlling the LEDs. Install it via the integrated library manager.
- MPU6050_tockn library for communicating with the gyroscope. Install it via the integrated library manager.

## Improvements over bkpvdsply

### Hardware

- [x] Reduce the overall height to fit slimmer wheels
- [x] Rethink the mounting system to be easier for the user
- [ ] Switch to a reusable cable tie alternative
- [ ] Improve the water resistance (ideally water proof)
- [ ] Make everything more balanced (equal weight distribution)
- [ ] Add LED strips to both sides
- [ ] 

### Electronics

- [ ] Improve battery runtime
- [ ] Make charging easier
- [ ] Develop a properly sized power supply unit
- [ ] Use custom-made circuit boards
- [ ] Switch to all SMT components
- [ ] Off-hand to production to a specialized vendor

### Software

- [ ] Implement customizable LED strip positions (relative to control board)
- [ ] Allow the rotation direction to be reversed (when mounting on the other side)
- [ ] Use LED trips on both sides to display the image
- [ ] Allow the image to be mirror for the second side (useful for symbols and graphics, but not text)
- [ ] Reduce storage size of images (ideally via file compression)
- [ ] Add the ability to make animations with more than 2 images
- [ ] Improve image stability (more exact rotation and speed change detection)

And more to come...

## How to build it

TODO

| Quantity: | Item: | Source: |
| --- | --- | --- |
| 1 | ESP32 D1 Mini | [Amazon](https://www.amazon.de/dp/B08BTLYSTM) |
| 2 | LED strip | [Amazon](https://www.amazon.de/dp/B08JJ6S1HC) |
| 16 | JST SM 3 pin connectors | [Amazon](https://www.amazon.de/YIXISI-M%C3%A4nnlichen-Weiblichen-Steckverbinder-Elektrisch/dp/B08JV8TJ9N) |
| 1 | MPU-6050 | [Amazon](https://www.amazon.de/AZDelivery-MPU-6050-3-Achsen-Gyroskop-Beschleunigungssensor-Arduino/dp/B07TKLYBD6) |
| 1 | Cable ties | [Amazon](https://www.amazon.de/Kabelbinder-Rscolila-Hochleistungs-Kabelmanagement-300mmx5mm/dp/B08ZC7PBSD) |

## How to use it

TODO
