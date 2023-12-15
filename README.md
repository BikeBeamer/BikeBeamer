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
- [x] FAILED: ~~Switch to a reusable cable tie alternative~~
- [ ] Improve the water resistance (ideally water proof)
- [ ] Make everything more balanced (equal weight distribution)
- [ ] Add LED strips to both sides

### Electronics

- [ ] Improve battery runtime
- [ ] Make charging easier
- [ ] Develop a properly sized power supply unit
- [ ] Use custom-made circuit boards
- [ ] Switch to all SMT components
- [ ] Off-hand to production to a specialized vendor

### Software

- [x] Implement customizable LED strip positions (relative to control board)
- [x] Allow the rotation direction to be reversed (when the control board back faces right)
- [ ] Use LED trips on both sides to display the image
- [ ] Allow the image to be mirrored for the second side (useful for symbols and graphics, but not text)
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

## Developer notes

- All angles used are in counted in degrees clockwise from 12 o'clock looking at the left side of the bike
- All coordinate systems used have their origin at the bottom left
- The LED strips are connected in clockwise direction on the left side and counterclockweise on the right side

## Similiar projects

- https://subethasoftware.com/bikelights/
- MonkeyLight Pro
- https://learn.adafruit.com/spokepov/overview
- https://www.ladyada.net/make/spokepov/
- https://learn.adafruit.com/bike-wheel-pov-display/overview
- https://bicycletouringpro.com/hokey-spokes-bicycle-lights-review/

## Possible power supplies

- https://www.adafruit.com/product/770
- https://www.adafruit.com/product/771
- https://www.amazon.de/Power-Bank-Energy-Ladekabel-5000mAh/dp/B08G93D8LZ
- https://www.amazon.de/Intenso-Powerbank-Ladeger%C3%A4t-Smartphone-Digitalkamera-wei%C3%9F/dp/B07Z6N28RD
- https://www.amazon.de/Powerbank-EnergyQC-Kompakter-Tragbares-Smartphones-Schwarz/dp/B0B158K8KK
- https://www.amazon.de/Anker-PowerCore-Externer-Powerbank-Ladeger%C3%A4t-Schwarz/dp/B01CU1EC6Y
- https://www.amazon.de/Sourcingmap-Batterie-Halter-Speicher-Schwarz/dp/B07B4WVS7R
- https://www.amazon.de/GALDOEP-Batteriehalter-batteriehalter-Druckknopfanschlu%C3%9F-elektronische/dp/B094X2M6GC
- https://www.amazon.de/YIXISI-Kunststoff-Batteriehalter-Anschlusskabel-Halterung/dp/B08JV9S4XY
- https://www.amazon.de/St%C3%BCck-Batteriehalter-Mignon-mit-Anschlu%C3%9Fkabel/dp/B00EOKW77G

## Mounting visualisation

![Mounting visualisation](mounting-visualisation.png)

## Image display visualisation

![Image display visualidation](image-display-visualisation.png)