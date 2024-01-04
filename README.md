# BikeBeamer

> This is a **very early work in progress**. Things aren't final and are due to change over time. **Don't waste your time to build it in its current state**.

## Overview

![Overview image](overview.png)

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
- [x] Add LED strips to both sides

### Electronics

- [ ] Improve battery runtime
- [ ] Make charging easier
- [ ] Develop a properly sized power supply unit
- [x] Use custom-made circuit boards
- [ ] Switch to all SMT components
- [ ] Off-hand to production to a specialized vendor

### Software

- [x] Implement customizable LED strip positions (relative to control board)
- [x] Allow the rotation direction to be reversed (when the control board back faces right)
- [x] Use LED trips on both sides to display the image
- [x] Allow the image to be mirrored for the second side (useful for symbols and graphics, but not text)
- [ ] Reduce storage size of images (ideally via file compression)
- [ ] Add the ability to make animations with more than 2 images
- [x] Improve image stability (more exact rotation and speed change detection, smooth measuremets)
- [x] Implement customizable sampling threshold (allowed variation to trigger rotation detection)

And more to come...

## How to build it

TODO

| Quantity: | Item: | Source: |
| --- | --- | --- |
| 1 | ESP32 D1 Mini | [Amazon](https://www.amazon.de/dp/B08BTLYSTM) |
| 1 | MPU-6050 | [Amazon](https://www.amazon.de/AZDelivery-MPU-6050-3-Achsen-Gyroskop-Beschleunigungssensor-Arduino/dp/B07TKLYBD6) |
| 2 | LED strip (144 LEDs/m) | [Amazon](https://www.amazon.de/dp/B08JJ6S1HC) |
| 12 | Battery holder | [RS Components](https://de.rs-online.com/web/p/batteriehalter/6119576) OR [Amazon](https://www.amazon.de/AA-Batteriehalter-KUOQIY-Batteriehalter-Drahtkabeln-Schraubendreher/dp/B0C2Q6JV88) |
| 4 | Power switch | [RS Components](https://de.rs-online.com/web/p/schiebeschalter/7347334) OR [Amazon](https://www.amazon.de/RUNCCI-YUN-Schiebeschalter-Positionen-Vertikal-Schalter/dp/B09TVDZ8P2) |
| 4 | Reverse-voltage diode (1N5820) | [RS Components](https://de.rs-online.com/web/p/schottky-dioden-und-gleichrichter/6547224) OR [Amazon](https://www.amazon.de/ALLECIN-1N5820-Schottky-Gleichrichterdiode-Siliziumdioden/dp/B0CKSH71TP) |
| 12 | Power smoothing capacitor (1000μF) | [RS Components](https://de.rs-online.com/web/p/aluminium-elektrolytkondensatoren/1815420) OR [Amazon](https://www.amazon.de/Kondensator-radial-1000%C2%B5F-RJF-10V102MH4-1000uF/dp/B07H9BFK31) |
| 6 | Screw terminal | [RS Components](https://de.rs-online.com/web/p/leiterplatten-printklemmen/1731613) OR [Amazon](https://www.amazon.de/RUNCCI-St%C3%BCcke-Terminal-Schraubklemme-Steckverbinder/dp/B07RR7D267) |
| 6 | JST SM 3 pin connectors | [Amazon](https://www.amazon.de/YIXISI-M%C3%A4nnlichen-Weiblichen-Steckverbinder-Elektrisch/dp/B08JV8TJ9N) |
| 12 | Cable ties | [Amazon](https://www.amazon.de/Kabelbinder-Rscolila-Hochleistungs-Kabelmanagement-300mmx5mm/dp/B08ZC7PBSD) |

## How to use it

TODO

## Developer notes

- All angles used are in counted in degrees clockwise from 12 o'clock looking at the left side of the bike
- All coordinate systems used have their origin at the bottom left
- The PCBs are mounted with the back side visible when looking at the left side of the bike and connected in clockwise direction
- All indexes in the code are zero-based, while all user interfaces and visualisations use regular, one-based numbering

### Mounting visualisation

![Mounting visualisation](mounting-visualisation.png)

### Image display visualisation

![Image display visualidation](image-display-visualisation.png)

## Similiar projects

- https://subethasoftware.com/bikelights/
- https://www.kickstarter.com/projects/minimonkey/monkey-light-pro-bicycle-wheel-display-system/description
- https://learn.adafruit.com/spokepov/overview
- https://www.ladyada.net/make/spokepov/
- https://learn.adafruit.com/bike-wheel-pov-display/overview
- https://bicycletouringpro.com/hokey-spokes-bicycle-lights-review/