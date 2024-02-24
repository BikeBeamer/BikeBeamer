#include <Adafruit_NeoPixel.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <MPU6050_tockn.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <uri/UriBraces.h>

// LED and WiFi related constants
const int NEOPIXELS_PIN = 16;
const int STORAGE_SLOT_COUNT = 8;
const int LED_COUNT = 32;
const char* SSID = "BikeBeamer";
const char* WIFI_PASSWORD = "BikeBeamer";
const IPAddress IP(192, 168, 0, 1);

// MPU, NeoPixel and server objects
MPU6050 mpu(Wire);
Adafruit_NeoPixel neopixels(LED_COUNT * 8, NEOPIXELS_PIN, NEO_GRB + NEO_KHZ800);
DNSServer dns;
WebServer server(80);

// Other variables
byte images[2][360 * LED_COUNT][3];
int brightness = 51;
bool mirrorImage = true;
int displayMode = 0;
unsigned long animationInterval = 1000000;
int internalWheelDiameter = 600;
int hubDiameter = 80;
int strip1Position = 90;
int strip2Position = 180;
int strip3Position = 270;
int samplingThreshold = 8;
int sampleCount = 1;
int avgdRevolutionPeriods = 80;
unsigned long currentMicros = 0;
unsigned long revolutionPeriod = 0;
unsigned long lastSample = 0;
unsigned long lastSlotChange = 0;
int currentMemorySlot = 0;
int angle = 0;
int virtualAngle = 0;
int lastVirtualAngle = -1;
bool isReceiving = false;
bool isPaused = true;
unsigned long revolutionOffset = 0;
unsigned long oldRevolutionPeriods[99];
int emptyRevolutionPeriods = 0;
unsigned long avgRevolutionPeriod = 0;

// Function to load the settings from storage into RAM
void loadSettings();

// Function to load an image from storage into RAM
void loadImage(int storageSlot, int memorySlot);

// Function to save the settings from RAM into storage
void saveSettings();

// Function to save an image name into storage
void saveImageName(int storageSlot, String newName);

// Setup function
void setup() {
    // Confirm successful boot
    /*
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    */
    // Initialize MPU
    Wire.begin();
    mpu.begin();
    mpu.calcGyroOffsets();
    // Initialize filesystem and memory
    LittleFS.begin();
    loadSettings();
    if (displayMode < STORAGE_SLOT_COUNT) {
        loadImage(displayMode, 0);
    } else {
        loadImage(displayMode - STORAGE_SLOT_COUNT, 0);
        loadImage((displayMode - (STORAGE_SLOT_COUNT - 1)) % STORAGE_SLOT_COUNT, 1);
    }
    // Initiliaze NeoPixels
    neopixels.begin();
    neopixels.setBrightness(brightness);
    neopixels.clear();
    neopixels.show();
    // Initialize WiFi network
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IP, IP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(SSID, WIFI_PASSWORD);
    // Initialize DNS
    dns.start(53, "*", IP);
    // Initialize web server
    server.enableCORS(true);
    server.on("/api/settings", HTTP_GET, [&]() {
        server.sendHeader("brightness", String(brightness));
        server.sendHeader("mirror-image", String(mirrorImage));
        server.sendHeader("display-mode", String(displayMode));
        server.sendHeader("animation-interval", String(animationInterval));
        server.sendHeader("internal-wheel-diameter", String(internalWheelDiameter));
        server.sendHeader("hub-diameter", String(hubDiameter));
        server.sendHeader("strip-1-position", String(strip1Position));
        server.sendHeader("strip-2-position", String(strip2Position));
        server.sendHeader("strip-3-position", String(strip3Position));
        server.sendHeader("sampling-threshold", String(samplingThreshold));
        server.sendHeader("sample-count", String(sampleCount));
        server.sendHeader("averaged-revolution-periods", String(avgdRevolutionPeriods));
        server.send(200);
    });
    server.on("/api/settings", HTTP_PUT, [&]() {
    });
    server.on("/api/images", HTTP_GET, [&]() {
        File file = LittleFS.open("/data/images.csv", "r");
        while (file.available()) {
            String line = file.readStringUntil('\n');
            String slot = line.substring(0, line.indexOf(','));
            int slotInt = slot.toInt();
            line.remove(0, line.indexOf(',') + 2);
            String name = line;
            if (slot.length() <= String(STORAGE_SLOT_COUNT).length() && slotInt >= 0 && slotInt < STORAGE_SLOT_COUNT) {
                server.sendHeader("slot-" + slot, name);
            }
        }
        file.close();
        server.send(200);
    });
    server.on(UriBraces("/api/images/{}"), HTTP_PUT, []() {
        int imageId = server.pathArg(0).toInt();
    });

    server.on("/", HTTP_POST, [&]() {
        // Check for necessary arguments
        bool hasSettingsArgs = server.hasArg("brightness") && server.hasArg("mirror-image") &&
                               server.hasArg("display-mode") && server.hasArg("animation-interval") &&
                               server.hasArg("internal-wheel-diameter") && server.hasArg("hub-diameter") &&
                               server.hasArg("strip-1-position") && server.hasArg("strip-2-position") &&
                               server.hasArg("strip-3-position") && server.hasArg("sampling-threshold") &&
                               server.hasArg("sample-count") && server.hasArg("averaged-revolution-periods");
        bool hasImageArgs = server.hasArg("slot") && server.hasArg("name") && server.hasArg("angle");
        for (int i = 0; i < LED_COUNT; i++) {
            if (!hasImageArgs) {
                break;
            }
            hasImageArgs = server.hasArg("led-" + String(i) + "-r");
        }
        for (int i = 0; i < LED_COUNT; i++) {
            if (!hasImageArgs) {
                break;
            }
            hasImageArgs = server.hasArg("led-" + String(i) + "-g");
        }
        for (int i = 0; i < LED_COUNT; i++) {
            if (!hasImageArgs) {
                break;
            }
            hasImageArgs = server.hasArg("led-" + String(i) + "-b");
        }
        // Extract and store the received data
        if (hasSettingsArgs) {
            isReceiving = true;
            neopixels.clear();
            neopixels.show();
            for (int i = 0; i < avgdRevolutionPeriods - 1; i++) {
                oldRevolutionPeriods[i] = 0;
            }
            brightness = server.arg("brightness").toInt();
            mirrorImage = server.arg("mirror-image").toInt();
            displayMode = server.arg("display-mode").toInt();
            animationInterval = server.arg("animation-interval").toInt();
            internalWheelDiameter = server.arg("internal-wheel-diameter").toInt();
            hubDiameter = server.arg("hub-diameter").toInt();
            strip1Position = server.arg("strip-1-position").toInt();
            strip2Position = server.arg("strip-2-position").toInt();
            strip3Position = server.arg("strip-3-position").toInt();
            samplingThreshold = server.arg("sampling-threshold").toInt();
            sampleCount = server.arg("sample-count").toInt();
            avgdRevolutionPeriods = server.arg("averaged-revolution-periods").toInt();
            neopixels.setBrightness(brightness);
            neopixels.show();
            if (displayMode < STORAGE_SLOT_COUNT) {
                loadImage(displayMode, currentMemorySlot);
            } else {
                loadImage(displayMode - STORAGE_SLOT_COUNT, 0);
                loadImage((displayMode - (STORAGE_SLOT_COUNT - 1)) % STORAGE_SLOT_COUNT, 1);
            }
            saveSettings();
            isReceiving = false;
            server.send(200);
        } else if (hasImageArgs) {
            int storageSlot = server.arg("slot").toInt();
            int memorySlot = -1;
            int angle = server.arg("angle").toInt();
            String filename = "/data/image-" + String(storageSlot) + ".csv";
            File file;
            if (angle == 0) {
                isReceiving = true;
                neopixels.clear();
                neopixels.show();
                for (int i = 0; i < avgdRevolutionPeriods - 1; i++) {
                    oldRevolutionPeriods[i] = 0;
                }
                saveImageName(storageSlot, server.arg("name"));
                file = LittleFS.open(filename, "w");
                file.print("R:, G:, B:\n");
            } else {
                file = LittleFS.open(filename, "a");
            }
            if (storageSlot == displayMode) {
                memorySlot = currentMemorySlot;
            } else if (storageSlot == displayMode - STORAGE_SLOT_COUNT) {
                memorySlot = 0;
            } else if (storageSlot == (displayMode - (STORAGE_SLOT_COUNT - 1)) % STORAGE_SLOT_COUNT) {
                memorySlot = 1;
            }
            for (int i = 0; i < LED_COUNT; i++) {
                byte r = server.arg("led-" + String(i) + "-r").toInt();
                byte g = server.arg("led-" + String(i) + "-g").toInt();
                byte b = server.arg("led-" + String(i) + "-b").toInt();
                if (memorySlot != -1) {
                    images[memorySlot][(angle * LED_COUNT) + i][0] = r;
                    images[memorySlot][(angle * LED_COUNT) + i][1] = g;
                    images[memorySlot][(angle * LED_COUNT) + i][2] = b;
                }
                file.print(String(r) + ", " + String(g) + ", " + String(b) + "\n");
            }
            file.close();
            if (angle == 359) {
                isReceiving = false;
            }
            server.send(200);
        } else {
            server.send(404, "text/plain", "Invalid request");
        }
    });
    server.serveStatic("/", LittleFS, "/index.html");
    server.serveStatic("/index.html", LittleFS, "/index.html");
    server.serveStatic("/data/image-0.csv", LittleFS, "/data/image-0.csv");
    server.serveStatic("/data/image-1.csv", LittleFS, "/data/image-1.csv");
    server.serveStatic("/data/image-2.csv", LittleFS, "/data/image-2.csv");
    server.serveStatic("/data/image-3.csv", LittleFS, "/data/image-3.csv");
    server.serveStatic("/data/image-4.csv", LittleFS, "/data/image-4.csv");
    server.serveStatic("/data/image-5.csv", LittleFS, "/data/image-5.csv");
    server.serveStatic("/data/image-6.csv", LittleFS, "/data/image-6.csv");
    server.serveStatic("/data/image-7.csv", LittleFS, "/data/image-7.csv");
    server.serveStatic("/data/images.csv", LittleFS, "/data/images.csv");
    server.serveStatic("/data/settings.csv", LittleFS, "/data/settings.csv");
    server.serveStatic("/scripts/index.js", LittleFS, "/scripts/index.js");
    server.serveStatic("/styles/dark.min.css", LittleFS, "/styles/dark.min.css");
    server.onNotFound([&]() {
        server.send(404, "text/plain", "Resource not found");
    });
    server.begin();
    lastSample = micros();
    lastSlotChange = micros();
}

// Main function
void loop() {
    if (!isReceiving) {
        mpu.update();
        angle = ((int) round(mpu.getAccAngleX()) + 360) % 360;
        currentMicros = micros();
        // Calculate revolution period and update other data on every rotation when the user is pedalling
        if ((angle >= (360 - samplingThreshold) || angle <= samplingThreshold) &&
            ((sampleCount == 0 && currentMicros - lastSample >= 150000 && currentMicros - lastSample <= 1500000) ||
             (sampleCount == 1 && currentMicros - lastSample >= 75000 && currentMicros - lastSample <= 750000) ||
             (sampleCount == 2 && currentMicros - lastSample >= 37500 && currentMicros - lastSample <= 375000))) {
            if (isPaused) {
                isPaused = false;
            }
            switch (sampleCount) {
                case 0:
                    revolutionPeriod = currentMicros - lastSample;
                    break;
                case 1:
                    revolutionPeriod = (currentMicros - lastSample) * 2;
                    break;
                case 2:
                    revolutionPeriod = (currentMicros - lastSample) * 4;
                    break;
            }
            avgRevolutionPeriod = revolutionPeriod;
            emptyRevolutionPeriods = 0;
            for (int i = 0; i < avgdRevolutionPeriods - 1; i++) {
                avgRevolutionPeriod += oldRevolutionPeriods[i];
                if (oldRevolutionPeriods[i] == 0) {
                    emptyRevolutionPeriods++;
                }
            }
            avgRevolutionPeriod =
                (unsigned long) round(avgRevolutionPeriod / (double) (avgdRevolutionPeriods - emptyRevolutionPeriods));
            for (int i = avgdRevolutionPeriods - 2; i > 0; i--) {
                oldRevolutionPeriods[i] = oldRevolutionPeriods[i - 1];
            }
            oldRevolutionPeriods[0] = revolutionPeriod;
            lastSample = currentMicros;
            virtualAngle = 0;
            lastVirtualAngle = -1;
            revolutionOffset = 0;
        }
        if (angle >= (90 - samplingThreshold) && angle <= (90 + samplingThreshold) && sampleCount == 2 &&
            currentMicros - lastSample >= 37500 && currentMicros - lastSample <= 375000) {
            if (isPaused) {
                isPaused = false;
            }
            revolutionPeriod = (currentMicros - lastSample) * 4;
            emptyRevolutionPeriods = 0;
            for (int i = 0; i < avgdRevolutionPeriods - 1; i++) {
                avgRevolutionPeriod += oldRevolutionPeriods[i];
                if (oldRevolutionPeriods[i] == 0) {
                    emptyRevolutionPeriods++;
                }
            }
            avgRevolutionPeriod =
                (unsigned long) round(avgRevolutionPeriod / (double) (avgdRevolutionPeriods - emptyRevolutionPeriods));
            for (int i = avgdRevolutionPeriods - 2; i > 0; i--) {
                oldRevolutionPeriods[i] = oldRevolutionPeriods[i - 1];
            }
            oldRevolutionPeriods[0] = revolutionPeriod;
            lastSample = currentMicros;
            virtualAngle = 0;
            lastVirtualAngle = -1;
            revolutionOffset = avgRevolutionPeriod * .75;
        }
        if (angle >= (180 - samplingThreshold) && angle <= (180 + samplingThreshold) &&
            ((sampleCount == 1 && currentMicros - lastSample >= 75000 && currentMicros - lastSample <= 750000) ||
             (sampleCount == 2 && currentMicros - lastSample >= 37500 && currentMicros - lastSample <= 375000))) {
            if (isPaused) {
                isPaused = false;
            }
            switch (sampleCount) {
                case 1:
                    revolutionPeriod = (currentMicros - lastSample) * 2;
                    break;
                case 2:
                    revolutionPeriod = (currentMicros - lastSample) * 4;
                    break;
            }
            emptyRevolutionPeriods = 0;
            for (int i = 0; i < avgdRevolutionPeriods - 1; i++) {
                avgRevolutionPeriod += oldRevolutionPeriods[i];
                if (oldRevolutionPeriods[i] == 0) {
                    emptyRevolutionPeriods++;
                }
            }
            avgRevolutionPeriod =
                (unsigned long) round(avgRevolutionPeriod / (double) (avgdRevolutionPeriods - emptyRevolutionPeriods));
            for (int i = avgdRevolutionPeriods - 2; i > 0; i--) {
                oldRevolutionPeriods[i] = oldRevolutionPeriods[i - 1];
            }
            oldRevolutionPeriods[0] = revolutionPeriod;
            lastSample = currentMicros;
            virtualAngle = 0;
            lastVirtualAngle = -1;
            revolutionOffset = avgRevolutionPeriod * .5;
        }
        if (angle >= (270 - samplingThreshold) && angle <= (270 + samplingThreshold) && sampleCount == 2 &&
            currentMicros - lastSample >= 37500 && currentMicros - lastSample <= 375000) {
            if (isPaused) {
                isPaused = false;
            }
            revolutionPeriod = (currentMicros - lastSample) * 4;
            emptyRevolutionPeriods = 0;
            for (int i = 0; i < avgdRevolutionPeriods - 1; i++) {
                avgRevolutionPeriod += oldRevolutionPeriods[i];
                if (oldRevolutionPeriods[i] == 0) {
                    emptyRevolutionPeriods++;
                }
            }
            avgRevolutionPeriod =
                (unsigned long) round(avgRevolutionPeriod / (double) (avgdRevolutionPeriods - emptyRevolutionPeriods));
            for (int i = avgdRevolutionPeriods - 2; i > 0; i--) {
                oldRevolutionPeriods[i] = oldRevolutionPeriods[i - 1];
            }
            oldRevolutionPeriods[0] = revolutionPeriod;
            lastSample = currentMicros;
            virtualAngle = 0;
            lastVirtualAngle = -1;
            revolutionOffset = avgRevolutionPeriod * .25;
        }
        // Pause display when user stops pedalling
        if ((sampleCount == 0 && currentMicros - lastSample > 1500000) ||
            (sampleCount == 1 && currentMicros - lastSample > 750000) ||
            (sampleCount == 2 && currentMicros - lastSample > 375000)) {
            if (!isPaused) {
                neopixels.clear();
                neopixels.show();
                for (int i = 0; i < avgdRevolutionPeriods - 1; i++) {
                    oldRevolutionPeriods[i] = 0;
                }
                isPaused = true;
            }
            lastSample = currentMicros;
        }
        if (!isPaused) {
            virtualAngle =
                360 -
                (((int) round(((currentMicros - lastSample) + revolutionOffset) / (avgRevolutionPeriod / 360.0))) %
                 360);
            // Animate through images if enabled
            if (displayMode >= STORAGE_SLOT_COUNT && currentMicros - lastSlotChange > animationInterval) {
                currentMemorySlot = (currentMemorySlot + 1) % 2;
                lastSlotChange = currentMicros;
            }
            // Display an angle of the image
            if (virtualAngle != lastVirtualAngle) {
                // Left side
                for (int i = 0; i < LED_COUNT; i++) {
                    byte r = images[currentMemorySlot][((virtualAngle % 360) * LED_COUNT) + i][0];
                    byte g = images[currentMemorySlot][((virtualAngle % 360) * LED_COUNT) + i][1];
                    byte b = images[currentMemorySlot][((virtualAngle % 360) * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i, neopixels.Color(r, g, b));
                    r = images[currentMemorySlot][(((virtualAngle + strip1Position) % 360) * LED_COUNT) + i][0];
                    g = images[currentMemorySlot][(((virtualAngle + strip1Position) % 360) * LED_COUNT) + i][1];
                    b = images[currentMemorySlot][(((virtualAngle + strip1Position) % 360) * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i + (LED_COUNT * 2), neopixels.Color(r, g, b));
                    r = images[currentMemorySlot][(((virtualAngle + strip2Position) % 360) * LED_COUNT) + i][0];
                    g = images[currentMemorySlot][(((virtualAngle + strip2Position) % 360) * LED_COUNT) + i][1];
                    b = images[currentMemorySlot][(((virtualAngle + strip2Position) % 360) * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i + (LED_COUNT * 4), neopixels.Color(r, g, b));
                    r = images[currentMemorySlot][(((virtualAngle + strip3Position) % 360) * LED_COUNT) + i][0];
                    g = images[currentMemorySlot][(((virtualAngle + strip3Position) % 360) * LED_COUNT) + i][1];
                    b = images[currentMemorySlot][(((virtualAngle + strip3Position) % 360) * LED_COUNT) + i][2];
                    neopixels.setPixelColor(i + (LED_COUNT * 6), neopixels.Color(r, g, b));
                }
                // Right side
                for (int i = 0; i < LED_COUNT; i++) {
                    if (mirrorImage) {
                        // Display mirrored image (left becomes right, so both sides face in driving direction =>
                        // matching LED strips 100% identical, just show the same data)
                        byte r = images[currentMemorySlot][((virtualAngle % 360) * LED_COUNT) + i][0];
                        byte g = images[currentMemorySlot][((virtualAngle % 360) * LED_COUNT) + i][1];
                        byte b = images[currentMemorySlot][((virtualAngle % 360) * LED_COUNT) + i][2];
                        neopixels.setPixelColor(i + LED_COUNT, neopixels.Color(r, g, b));
                        r = images[currentMemorySlot][(((virtualAngle + strip1Position) % 360) * LED_COUNT) + i][0];
                        g = images[currentMemorySlot][(((virtualAngle + strip1Position) % 360) * LED_COUNT) + i][1];
                        b = images[currentMemorySlot][(((virtualAngle + strip1Position) % 360) * LED_COUNT) + i][2];
                        neopixels.setPixelColor(i + (LED_COUNT * 3), neopixels.Color(r, g, b));
                        r = images[currentMemorySlot][(((virtualAngle + strip2Position) % 360) * LED_COUNT) + i][0];
                        g = images[currentMemorySlot][(((virtualAngle + strip2Position) % 360) * LED_COUNT) + i][1];
                        b = images[currentMemorySlot][(((virtualAngle + strip2Position) % 360) * LED_COUNT) + i][2];
                        neopixels.setPixelColor(i + (LED_COUNT * 5), neopixels.Color(r, g, b));
                        r = images[currentMemorySlot][(((virtualAngle + strip3Position) % 360) * LED_COUNT) + i][0];
                        g = images[currentMemorySlot][(((virtualAngle + strip3Position) % 360) * LED_COUNT) + i][1];
                        b = images[currentMemorySlot][(((virtualAngle + strip3Position) % 360) * LED_COUNT) + i][2];
                        neopixels.setPixelColor(i + (LED_COUNT * 7), neopixels.Color(r, g, b));
                    } else {
                        // Display normal image (right remains right, so only one side faces in driving direction =>
                        // => correspoding LED strips 0% identical, calculate data separately)
                        byte r = images[currentMemorySlot][(((360 - virtualAngle) % 360) * LED_COUNT) + i][0];
                        byte g = images[currentMemorySlot][(((360 - virtualAngle) % 360) * LED_COUNT) + i][1];
                        byte b = images[currentMemorySlot][(((360 - virtualAngle) % 360) * LED_COUNT) + i][2];
                        neopixels.setPixelColor(i + LED_COUNT, neopixels.Color(r, g, b));
                        r = images[currentMemorySlot]
                                  [((((360 - virtualAngle) + (360 - strip1Position)) % 360) * LED_COUNT) + i][0];
                        g = images[currentMemorySlot]
                                  [((((360 - virtualAngle) + (360 - strip1Position)) % 360) * LED_COUNT) + i][1];
                        b = images[currentMemorySlot]
                                  [((((360 - virtualAngle) + (360 - strip1Position)) % 360) * LED_COUNT) + i][2];
                        neopixels.setPixelColor(i + (LED_COUNT * 3), neopixels.Color(r, g, b));
                        r = images[currentMemorySlot]
                                  [((((360 - virtualAngle) + (360 - strip2Position)) % 360) * LED_COUNT) + i][0];
                        g = images[currentMemorySlot]
                                  [((((360 - virtualAngle) + (360 - strip2Position)) % 360) * LED_COUNT) + i][1];
                        b = images[currentMemorySlot]
                                  [((((360 - virtualAngle) + (360 - strip2Position)) % 360) * LED_COUNT) + i][2];
                        neopixels.setPixelColor(i + (LED_COUNT * 5), neopixels.Color(r, g, b));
                        r = images[currentMemorySlot]
                                  [((((360 - virtualAngle) + (360 - strip3Position)) % 360) * LED_COUNT) + i][0];
                        g = images[currentMemorySlot]
                                  [((((360 - virtualAngle) + (360 - strip3Position)) % 360) * LED_COUNT) + i][1];
                        b = images[currentMemorySlot]
                                  [((((360 - virtualAngle) + (360 - strip3Position)) % 360) * LED_COUNT) + i][2];
                        neopixels.setPixelColor(i + (LED_COUNT * 7), neopixels.Color(r, g, b));
                    }
                }
                neopixels.show();
                lastVirtualAngle = virtualAngle;
            }
        }
    }
    dns.processNextRequest();
    server.handleClient();
}

// Function to load the settings from storage into RAM
void loadSettings() {
    File file = LittleFS.open("/data/settings.csv", "r");
    while (file.available()) {
        String line = file.readStringUntil('\n');
        String key = line.substring(0, line.indexOf(','));
        line.remove(0, line.indexOf(',') + 2);
        unsigned long value = line.toInt();
        if (key == "brightness") {
            brightness = value;
        } else if (key == "mirror-image") {
            mirrorImage = value;
        } else if (key == "display-mode") {
            displayMode = value;
        } else if (key == "animation-interval") {
            animationInterval = value;
        } else if (key == "internal-wheel-diameter") {
            internalWheelDiameter = value;
        } else if (key == "hub-diameter") {
            hubDiameter = value;
        } else if (key == "strip-1-position") {
            strip1Position = value;
        } else if (key == "strip-2-position") {
            strip2Position = value;
        } else if (key == "strip-3-position") {
            strip3Position = value;
        } else if (key == "sampling-threshold") {
            samplingThreshold = value;
        } else if (key == "sample-count") {
            sampleCount = value;
        } else if (key == "averaged-revolution-periods") {
            avgdRevolutionPeriods = value;
        }
    }
    file.close();
}

// Function to load an image from storage into RAM
void loadImage(int storageSlot, int memorySlot) {
    String filename = "/data/image-" + String(storageSlot) + ".csv";
    File file;
    // Clear the image
    for (int i = 0; i < 360 * LED_COUNT; i++) {
        images[memorySlot][i][0] = 0;
        images[memorySlot][i][1] = 0;
        images[memorySlot][i][2] = 0;
    }
    // Load the new image
    file = LittleFS.open(filename, "r");
    for (int i = -1; i < 360 * LED_COUNT; i++) {
        String line;
        if (!file.available()) {
            break;
        }
        line = file.readStringUntil('\n');
        if (i > -1) {
            images[memorySlot][i][0] = line.substring(0, line.indexOf(',')).toInt();
            line.remove(0, line.indexOf(',') + 1);
            images[memorySlot][i][1] = line.substring(0, line.indexOf(',')).toInt();
            line.remove(0, line.indexOf(',') + 1);
            images[memorySlot][i][2] = line.toInt();
        }
    }
    file.close();
}

// Function to save the settings from RAM into storage
void saveSettings() {
    File file = LittleFS.open("/data/settings.csv", "w");
    file.print("Key:, Value:\n");
    file.print("brightness, " + String(brightness) + "\n");
    file.print("mirror-image, " + String(mirrorImage) + "\n");
    file.print("display-mode, " + String(displayMode) + "\n");
    file.print("animation-interval, " + String(animationInterval) + "\n");
    file.print("internal-wheel-diameter, " + String(internalWheelDiameter) + "\n");
    file.print("hub-diameter, " + String(hubDiameter) + "\n");
    file.print("strip-1-position, " + String(strip1Position) + "\n");
    file.print("strip-2-position, " + String(strip2Position) + "\n");
    file.print("strip-3-position, " + String(strip3Position) + "\n");
    file.print("sampling-threshold, " + String(samplingThreshold) + "\n");
    file.print("sample-count, " + String(sampleCount) + "\n");
    file.print("averaged-revolution-periods, " + String(avgdRevolutionPeriods) + "\n");
    file.close();
}

// Function to save an image name into storage
void saveImageName(int storageSlot, String newName) {
    String buffer[STORAGE_SLOT_COUNT];
    File file = LittleFS.open("/data/images.csv", "r");
    while (file.available()) {
        String line = file.readStringUntil('\n');
        String slot = line.substring(0, line.indexOf(','));
        int slotInt = slot.toInt();
        line.remove(0, line.indexOf(',') + 2);
        String name = line;
        if (slot.length() <= String(STORAGE_SLOT_COUNT).length() && slotInt >= 0 && slotInt < STORAGE_SLOT_COUNT) {
            if (slotInt != storageSlot) {
                buffer[slotInt] = name;
            } else {
                buffer[slotInt] = newName;
            }
        }
    }
    file.close();
    file = LittleFS.open("/data/images.csv", "w");
    file.print("Slot:, Name:\n");
    for (int i = 0; i < STORAGE_SLOT_COUNT; i++) {
        file.print(String(i) + ", " + buffer[i] + "\n");
    }
    file.close();
}