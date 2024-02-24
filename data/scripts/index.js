'use strict';

// DOM references
let imageInput = document.getElementById('image');
let convertImageButton = document.getElementById('convert-image');
let originalImageCanvas = document.getElementById('original-image');
let originalImageContext = originalImageCanvas.getContext('2d', { willReadFrequently: true });
let bikeBeamerImageCanvas = document.getElementById('bikebeamer-image');
let bikeBeamerImageContext = bikeBeamerImageCanvas.getContext('2d');
let slotInput = document.getElementById('slot');
let nameInput = document.getElementById('name');
let uploadImageButton = document.getElementById('upload-image');
let progressLabel = document.getElementById('progress');
let brightnessInput = document.getElementById('brightness');
let mirrorImageInput = document.getElementById('mirror-image');
let displayModeInput = document.getElementById('display-mode');
let animationIntervalInput = document.getElementById('animation-interval');
let internalWheelDiameterInput = document.getElementById('internal-wheel-diameter');
let hubDiameterInput = document.getElementById('hub-diameter');
let strip1PositionInput = document.getElementById('strip-1-position');
let strip2PositionInput = document.getElementById('strip-2-position');
let strip3PositionInput = document.getElementById('strip-3-position');
let samplingThresholdInput = document.getElementById('sampling-threshold');
let sampleCountInput = document.getElementById('sample-count');
let avgdRevolutionPeriodsInput = document.getElementById('averaged-revolution-periods');
let saveSettingsButton = document.getElementById('save-settings');

// LED related constants
const STORAGE_SLOT_COUNT = 8;
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

// Function to load the image names
async function loadImageNames() {
    let response = await fetch('http://192.168.0.1/api/images', {
        method: 'GET',
    });
    let responseText = await response.text();
    if (response.ok) {
        let responseHeaders = response.headers;
        for (let i = 0; i < STORAGE_SLOT_COUNT; i++) {
            slotInput.options[i].label = 'Slot ' + (i + 1) + ' "' + responseHeaders.get('slot-' + i) + '"';
        }
        for (let i = 0; i < STORAGE_SLOT_COUNT; i++) {
            displayModeInput.options[i].label =
                'Only show slot ' + (i + 1) + ' "' + responseHeaders.get('slot-0') + '"';
            displayModeInput.options[i + STORAGE_SLOT_COUNT].label =
                'Animate through slots ' +
                (i + 1) +
                ' "' +
                responseHeaders.get('slot-' + i) +
                '" and ' +
                (((i + 1) % STORAGE_SLOT_COUNT) + 1) +
                ' "' +
                responseHeaders.get('slot-' + ((i + 1) % STORAGE_SLOT_COUNT)) +
                '"';
        }
        return true;
    } else {
        throw new Error('\nStatus: ' + response.status + '\nMessage: ' + responseText);
    }
}

// Function to load the settings
async function loadSettings() {
    let response = await fetch('http://192.168.0.1/api/settings', {
        method: 'GET',
    });
    let responseText = await response.text();
    if (response.ok) {
        let responseHeaders = response.headers;
        brightnessInput.value = Math.round((parseInt(responseHeaders.get('brightness')) / 255.0) * 100);
        mirrorImageInput.options.selectedIndex = parseInt(responseHeaders.get('mirror-image'));
        displayModeInput.options.selectedIndex = parseInt(responseHeaders.get('display-mode'));
        animationIntervalInput.value = Math.round(parseInt(responseHeaders.get('animation-interval')) / 1000.0);
        internalWheelDiameterInput.value = parseInt(responseHeaders.get('internal-wheel-diameter'));
        hubDiameterInput.value = parseInt(responseHeaders.get('hub-diameter'));
        strip1PositionInput.value = parseInt(responseHeaders.get('strip-1-position'));
        strip2PositionInput.value = parseInt(responseHeaders.get('strip-2-position'));
        strip3PositionInput.value = parseInt(responseHeaders.get('strip-3-position'));
        samplingThresholdInput.value = parseInt(responseHeaders.get('sampling-threshold'));
        sampleCountInput.options.selectedIndex = parseInt(responseHeaders.get('sample-count'));
        avgdRevolutionPeriodsInput.value = parseInt(responseHeaders.get('averaged-revolution-periods'));
        return true;
    } else {
        throw new Error('\nStatus: ' + response.status + '\nMessage: ' + responseText);
    }
}

// Function to upload the image angle by angle
async function uploadImage(slot, name, image) {
    for (let i = 0; i < 360; i++) {
        let data = new URLSearchParams();
        let response;
        let responseText;
        data.append('name', name);
        data.append('angle', i);
        for (let j = 0; j < LED_COUNT; j++) {
            data.append('led-' + j + '-r', image[i * LED_COUNT + j][0]);
            data.append('led-' + j + '-g', image[i * LED_COUNT + j][1]);
            data.append('led-' + j + '-b', image[i * LED_COUNT + j][2]);
        }
        response = await fetch('http://192.168.0.1/api/images/' + slot, {
            method: 'PUT',
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
    mirrorImage,
    displayMode,
    animationInterval,
    internalWheelDiameter,
    hubDiameter,
    strip1Position,
    strip2Position,
    strip3Position,
    samplingThreshold,
    sampleCount,
    avgdRevolutionPeriods
) {
    let data = new URLSearchParams();
    let response;
    let responseText;
    data.append('brightness', Math.round(brightness * (255 / 100.0)));
    data.append('mirror-image', mirrorImage === true ? 1 : 0);
    data.append('display-mode', displayMode);
    data.append('animation-interval', animationInterval * 1000);
    data.append('internal-wheel-diameter', internalWheelDiameter);
    data.append('hub-diameter', hubDiameter);
    data.append('strip-1-position', strip1Position);
    data.append('strip-2-position', strip2Position);
    data.append('strip-3-position', strip3Position);
    data.append('sampling-threshold', samplingThreshold);
    data.append('sample-count', sampleCount);
    data.append('averaged-revolution-periods', avgdRevolutionPeriods);
    response = await fetch('http://192.168.0.1/api/settings', {
        method: 'PUT',
        body: data,
    });
    responseText = await response.text();
    if (response.ok) {
        return true;
    } else {
        throw new Error('\nStatus: ' + response.status + '\nMessage: ' + responseText);
    }
}

// Handle lazy loading of data
window.addEventListener('load', async () => {
    try {
        await loadImageNames();
        await loadSettings();
    } catch (error) {
        console.error(error);
        alert(error);
    }
});
// Handle convert image button
convertImageButton.addEventListener('click', () => {
    if (internalWheelDiameterInput.value && internalWheelDiameterInput.value && imageInput.files[0]) {
        internalWheelDiameter = parseInt(internalWheelDiameterInput.value);
        hubDiameter = parseInt(hubDiameterInput.value);
        margin = (internalWheelDiameter / 2 - hubDiameter / 2 - LED_STRIP_LENGTH) / 2;
        originalImage.src = URL.createObjectURL(imageInput.files[0]);
    } else {
        alert('Please select an image before converting it and enter values for all settings.');
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
uploadImageButton.addEventListener('click', async () => {
    if (slotInput.value && nameInput.value && bikeBeamerImage[0][0] >= 0) {
        alert('Please be patient, the upload will take about 2 minutes.');
        try {
            await uploadImage(parseInt(slotInput.value), nameInput.value, bikeBeamerImage);
            await loadImageNames();
            alert('Image successfully uploaded.');
        } catch (error) {
            alert(error);
            console.error(error);
        }
    } else {
        alert('Please select a slot and convert an image before uploading it.');
    }
});
// Handle save settings button
saveSettingsButton.addEventListener('click', async () => {
    if (
        brightnessInput.value &&
        mirrorImageInput.value &&
        displayModeInput.value &&
        animationIntervalInput.value &&
        internalWheelDiameterInput.value &&
        hubDiameterInput.value &&
        strip1PositionInput.value &&
        strip2PositionInput.value &&
        strip3PositionInput.value &&
        samplingThresholdInput.value &&
        sampleCountInput.value &&
        avgdRevolutionPeriodsInput.value
    ) {
        try {
            await saveSettings(
                parseInt(brightnessInput.value),
                parseInt(mirrorImageInput.value) ? true : false,
                parseInt(displayModeInput.value),
                parseInt(animationIntervalInput.value),
                parseInt(internalWheelDiameterInput.value),
                parseInt(hubDiameterInput.value),
                parseInt(strip1PositionInput.value),
                parseInt(strip2PositionInput.value),
                parseInt(strip3PositionInput.value),
                parseInt(samplingThresholdInput.value),
                parseInt(sampleCountInput.value),
                parseInt(avgdRevolutionPeriodsInput.value)
            );
            await loadSettings();
            alert('Settings successfully saved.');
        } catch (error) {
            console.error(error);
            alert(error);
        }
    } else {
        alert('Please enter values for all settings');
    }
});
