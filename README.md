# ESP32 IoT Noise Monitor

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Project Status](https://img.shields.io/badge/status-active-success.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## Overview

The ESP32 IoT Noise Monitor is a sophisticated environmental monitoring solution that measures ambient noise levels in real-time. The system provides continuous sound level monitoring with cloud connectivity for data logging and analysis through ThingSpeak integration.

## Features

The system provides comprehensive noise monitoring capabilities including:

- Real-time sound level measurement in decibels (dB)
- High-precision audio sampling using INMP441 I2S microphone
- Live data visualization via OLED display
- WiFi connectivity for remote monitoring
- Cloud data logging through ThingSpeak
- Rolling average calculations (1-minute and all-time)
- Visual status indicators for connectivity and operation

## Hardware Requirements

### Core Components
- ESP32 Development Board
- INMP441 I2S MEMS Microphone
- SSD1306 OLED Display (128x64)

### Pin Configuration

#### INMP441 Microphone
- WS (Word Select/LRCLK): GPIO 15
- SD (Serial Data): GPIO 13
- SCK (Serial Clock): GPIO 2

#### OLED Display
- MOSI: GPIO 23
- CLK: GPIO 18
- DC: GPIO 16
- CS: GPIO 5
- RESET: GPIO 17

## Software Dependencies

The project relies on the following libraries:

```cpp
#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include "ThingSpeak.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
```

## Installation

1. Clone this repository
2. Install PlatformIO in your development environment
3. Create a `secrets.h` file with your credentials:

```cpp
#define SECRET_WIFI_NAME "your_wifi_ssid"
#define SECRET_WIFI_PASSWORD "your_wifi_password"
#define SECRET_myChannelNumber your_channel_number
#define SECRET_myApiKey "your_api_key"
```

4. Configure the project in PlatformIO
5. Upload the code to your ESP32

## Configuration

### Audio Sampling Parameters
```cpp
#define SAMPLE_RATE 44100
#define REFERENCE_LEVEL 8192.0
#define SCALE_FACTOR 2.5
#define DB_THRESHOLD 60.0
```

### Display Settings
```cpp
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
```

## Operation

The system operates through several key stages:

1. **Initialization**
   - Configures I2S interface for audio sampling
   - Initializes OLED display
   - Establishes WiFi connection
   - Sets up ThingSpeak client

2. **Main Operation**
   - Samples audio data continuously
   - Calculates real-time dB levels
   - Updates rolling averages
   - Displays current readings and status
   - Uploads data to ThingSpeak

3. **Data Display**
   - Current dB level
   - Noise level status (HIGH/LOW)
   - Connection status indicators
   - Rolling averages (1-minute and all-time)

## Data Analysis

The system calculates several metrics:

- Instantaneous dB levels
- One-minute rolling average
- Overall average since startup
- Threshold-based noise level classification

## ThingSpeak Integration

Data is uploaded to ThingSpeak for long-term storage and analysis:

1. Connects to ThingSpeak server
2. Uploads dB readings every sampling cycle
3. Provides visual confirmation of successful uploads
4. Maintains error handling for failed uploads

## Troubleshooting

Common issues and solutions:

1. WiFi Connection Issues
   - Check network credentials
   - Verify network stability
   - Monitor WiFi status indicator on display

2. Sensor Reading Issues
   - Verify I2S connections
   - Check microphone orientation
   - Ensure proper power supply

3. Display Problems
   - Verify I2C connections
   - Check display address configuration
   - Monitor serial output for errors

## Development

To modify or extend this project:

1. Fork the repository
2. Make your changes
3. Test thoroughly
4. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Author

Developed by Gonzalo Novoa

## Acknowledgments

- ThingSpeak for cloud data storage
- Adafruit for display libraries
- ESP32 community for I2S implementation examples

For additional information or support, please open an issue in the repository.


