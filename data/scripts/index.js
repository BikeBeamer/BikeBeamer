'use strict';

// DOM references
let internalWheelDiameterInput = document.getElementById('internal-wheel-diameter');
let hubDiameterInput = document.getElementById('hub-diameter');
let imageInput = document.getElementById('image');
let convertImageButton = document.getElementById('convert-image');
let originalImageCanvas = document.getElementById('original-image');
let originalImageContext = originalImageCanvas.getContext('2d');
let bikeBeamerImageCanvas = document.getElementById('bikebeamer-image');
let bikeBeamerImageContext = bikeBeamerImageCanvas.getContext('2d');
let slotInput = document.getElementById('slot');
let uploadImageButton = document.getElementById('upload-image');
let progressLabel = document.getElementById('progress');
let brightnessInput = document.getElementById('brightness');
let displayModeInput = document.getElementById('display-mode');
let animationIntervalInput = document.getElementById('animation-interval');
let strip0PositionInput = document.getElementById('stip-0-position');
let strip1PositionInput = document.getElementById('stip-1-position');
let strip2PositionInput = document.getElementById('stip-2-position');
let strip3PositionInput = document.getElementById('stip-3-position');
let saveSettingsButton = document.getElementById('save-settings');

// LED related constants
const LED_STRIP_LENGTH = 225;
const LED_COUNT = 32;
const LED_SPACING = LED_STRIP_LENGTH / LED_COUNT;

// Other variables
let internalWheelDiameter = 600;
let hubDiameter = 80;
let margin = (internalWheelDiameter / 2 - hubDiameter / 2 - LED_STRIP_LENGTH) / 2;
let originalImage = new Image();
let bikeBeamerImage = new Array(360 * LED_COUNT);
for (let i = 0; i < bikeBeamerImage.length; i++) {
    bikeBeamerImage[i] = new Array(3);
}
let slot = 0;
let brightness = 50;
let displayMode = 0;
let animationInterval = 1000;
let strip0Position = 0;
let strip1Position = 90;
let strip2Position = 180;
let strip3Position = 270;

// Function to upload the image angle by angle
async function uploadImage(slot, image) {
    for (let i = 0; i < 360; i++) {
        let data = new URLSearchParams();
        let response;
        let responseText;
        data.append('slot', slot);
        data.append('angle', i);
        for (let j = 0; j < LED_COUNT; j++) {
            data.append('led-' + j + '-r', image[i * LED_COUNT + j][0]);
            data.append('led-' + j + '-g', image[i * LED_COUNT + j][1]);
            data.append('led-' + j + '-b', image[i * LED_COUNT + j][2]);
        }
        response = await fetch('http://192.168.0.1', {
            method: 'POST',
            body: data,
        });
        responseText = await response.text();
        if (response.ok) {
            if (i < 359) {
                progressLabel.innerText = 'Progress: ' + Math.round((i / 360) * 100) + '%';
            } else {
                progressLabel.innerText = '';
            }
        } else {
            throw new Error('\nStatus: ' + response.status + '\nMessage: ' + responseText);
        }
    }
    return true;
}

// Function to save the settings
async function saveSettings(
    brightness,
    displayMode,
    animationInterval,
    strip0Position,
    strip1Position,
    strip2Position,
    strip3Position
) {
    let data = new URLSearchParams();
    let response;
    let responseText;
    data.append('brightness', Math.round(brightness * (255 / 100.0)));
    data.append('display-mode', displayMode);
    data.append('animation-interval', animationInterval * 1000);
    data.append('strip-0-position', strip0Position);
    data.append('strip-1-position', strip1Position);
    data.append('strip-2-position', strip2Position);
    data.append('strip-3-position', strip3Position);
    response = await fetch('http://192.168.0.1', {
        method: 'POST',
        body: data,
    });
    responseText = await response.text();
    if (response.ok) {
        return true;
    } else {
        throw new Error('\nStatus: ' + response.status + '\nMessage: ' + responseText);
    }
}

// Handle convert image button
convertImageButton.addEventListener('click', () => {
    if (internalWheelDiameterInput.value && internalWheelDiameterInput.value && imageInput.files[0]) {
        internalWheelDiameter = parseInt(internalWheelDiameterInput.value);
        hubDiameter = parseInt(hubDiameterInput.value);
        margin = (internalWheelDiameter / 2 - hubDiameter / 2 - LED_STRIP_LENGTH) / 2;
        originalImage.src = URL.createObjectURL(imageInput.files[0]);
    } else {
        alert('Please enter internal wheel/hub diameter and select an image before converting it.');
    }
});
// Handle image load
originalImage.addEventListener('load', () => {
    // Update canvas size
    originalImageCanvas.width = internalWheelDiameter;
    originalImageCanvas.height = internalWheelDiameter;
    bikeBeamerImageCanvas.width = internalWheelDiameter;
    bikeBeamerImageCanvas.height = internalWheelDiameter;
    // Draw original image
    originalImageContext.drawImage(originalImage, 0, 0, internalWheelDiameter, internalWheelDiameter);
    // Draw BikeBeamer background
    bikeBeamerImageContext.fillStyle = '#202b38';
    bikeBeamerImageContext.fillRect(0, 0, internalWheelDiameter, internalWheelDiameter);
    // Draw BikeBeamer image and store the data for upload
    for (let i = 0; i < 360; i++) {
        for (let j = 0; j < LED_COUNT; j++) {
            let x = Math.round(
                internalWheelDiameter / 2 +
                    (hubDiameter / 2 + margin + LED_SPACING / 2 + j * LED_SPACING) * Math.sin(i * (Math.PI / 180.0))
            );
            let y = Math.round(
                internalWheelDiameter / 2 +
                    (hubDiameter / 2 + margin + LED_SPACING / 2 + j * LED_SPACING) * Math.cos(i * (Math.PI / 180.0))
            );
            let pixel = originalImageContext.getImageData(x, internalWheelDiameter - y, 1, 1);
            pixel.data[3] = 255;
            bikeBeamerImageContext.putImageData(pixel, x, internalWheelDiameter - y);
            bikeBeamerImage[i * LED_COUNT + j][0] = pixel.data[0];
            bikeBeamerImage[i * LED_COUNT + j][1] = pixel.data[1];
            bikeBeamerImage[i * LED_COUNT + j][2] = pixel.data[2];
        }
    }
    URL.revokeObjectURL(this.src);
});
// Handle upload image button
uploadImageButton.addEventListener('click', () => {
    if (slotInput.value && bikeBeamerImage[0][0]) {
        slot = parseInt(slotInput.value);
        alert('Please be patient, the upload will take about 2 minutes.');
        uploadImage(slot, bikeBeamerImage)
            .then(() => {
                alert('Image successfully uploaded.');
            })
            .catch((error) => {
                alert(error);
            });
    } else {
        alert('Please select a slot and convert an image before uploading it.');
    }
});
// Handle save settings button
saveSettingsButton.addEventListener('click', () => {
    if (
        brightnessInput.value &&
        displayModeInput.value &&
        animationIntervalInput.value &&
        strip0PositionInput.value >= 0 &&
        strip1PositionInput.value &&
        strip2PositionInput.value &&
        strip3PositionInput.value
    ) {
        brightness = parseInt(brightnessInput.value);
        displayMode = parseInt(displayModeInput.value);
        animationInterval = parseInt(animationIntervalInput.value);
        strip0Position = parseInt(strip0PositionInput.value);
        strip1Position = parseInt(strip1PositionInput.value);
        strip2Position = parseInt(strip2PositionInput.value);
        strip3Position = parseInt(strip3PositionInput.value);
        saveSettings(
            brightness,
            displayMode,
            animationInterval,
            strip0Position,
            strip1Position,
            strip2Position,
            strip3Position
        )
            .then(() => {
                alert('Settings successfully saved.');
            })
            .catch((error) => {
                alert(error);
            });
    } else {
        alert('Please set a brightness, choose a display mode and enter an animation interval.');
    }
});
