#ifndef TEXTS_H
#define TEXTS_H

#include <Arduino.h>

// TEXTOS DEL SISTEMA - ESP32 IoT Noise Monitor

// Pantallas de inicio y configuración
const char TEXT_CONNECTING[] PROGMEM = "Connecting...";
const char TEXT_WIFI_NOT_CONNECTED[] PROGMEM = "Wifi not connected";
const char TEXT_WIFI_CONNECTED[] PROGMEM = "Wifi connected!";
const char TEXT_LOCAL_IP[] PROGMEM = "Local IP: ";

// Pantalla principal del proyecto
const char TEXT_NOISE_MONITOR[] PROGMEM = "NOISE MONITOR";
const char TEXT_SST_IOT[] PROGMEM = "SST-IoT";
const char TEXT_DEVELOPED_BY[] PROGMEM = "Developed by:";
const char TEXT_AUTHOR[] PROGMEM = "Gonzalo Novoa";

// Estados de medición
const char TEXT_LEVEL_HIGH[] PROGMEM = "Level: HIGH";
const char TEXT_LEVEL_LOW[] PROGMEM = "Level: LOW";
const char TEXT_NO_DATA[] PROGMEM = "No Data";
const char TEXT_DISCONNECTED[] PROGMEM = "Disconnected";

// Unidades y promedios
const char TEXT_DB_UNIT[] PROGMEM = " dB";
const char TEXT_AVG_ALL[] PROGMEM = "Avg(All):";
const char TEXT_AVG_1MIN[] PROGMEM = "Avg(1min):";
const char TEXT_DB_SUFFIX[] PROGMEM = "dB";

// Mensajes de consola/debug
const char TEXT_SETUP_I2S[] PROGMEM = "Setup I2S ...";
const char TEXT_SSD1306_FAILED[] PROGMEM = "SSD1306 allocation failed";
const char TEXT_WIFI_NOT_CONNECTED_DEBUG[] PROGMEM = "Wifi not connected";
const char TEXT_WIFI_CONNECTED_DEBUG[] PROGMEM = "Wifi connected !";
const char TEXT_LOCAL_IP_DEBUG[] PROGMEM = "Local IP: ";
const char TEXT_DATA_PUSHED[] PROGMEM = "Data pushed successfully";
const char TEXT_PUSH_ERROR[] PROGMEM = "Push error";
const char TEXT_NO_VALID_SAMPLES[] PROGMEM = "No valid readings";

// Formato para printf
const char TEXT_DB_AVERAGE_FORMAT[] PROGMEM = "Average dB (2s): %.1f dB\n";

#endif