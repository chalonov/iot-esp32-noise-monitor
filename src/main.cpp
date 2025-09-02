// Usar PlatformIO
#include <Arduino.h>

// Audio I2S
#include <driver/i2s.h>
// Para la calculos matematicos
#include <math.h>
// WIFI y ThingSpeak
#include <WiFi.h>
#include "ThingSpeak.h"
// OLED Display SSD1306
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// WiFi & Thinkspeak login
#include <secrets.h>
// Textos del sistema
#include "texts.h"

// Parametros OLED Display SSD1306
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// PinOut OLED Display SSD1306
#define OLED_MOSI   23
#define OLED_CLK   18
#define OLED_DC    16
#define OLED_CS    5
#define OLED_RESET 17
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// PinOut microfono INMP441
#define I2S_WS 15  // Word Select (LRCLK)                                            
#define I2S_SD 13  // Serial Data
#define I2S_SCK 2  // Serial Clock
#define I2S_PORT I2S_NUM_0  // Puerto I2S

// Nuevos parámetros para ajustar la sensibilidad                                                         
#define REFERENCE_LEVEL 8192.0  // Nivel de referencia para el cálculo de dB
#define SCALE_FACTOR 2.5         // Factor de escala para las muestras
#define DB_THRESHOLD 60.0        // Umbral para determinar nivel ALTO/BAJO

float overallAverageDB = 40;
float lastMinuteAverageDB = 0;
const int SAMPLES_PER_MINUTE = 60;  // Asumiendo que tomamos una muestra por segundo
float lastMinuteReadings[SAMPLES_PER_MINUTE] = {0};
int lastMinuteIndex = 0;
unsigned long totalSamples = 0;

// Parametros de conexion WIFI y ThingSpeak
const char* WIFI_NAME = SECRET_WIFI_NAME;
const char* WIFI_PASSWORD = SECRET_WIFI_PASSWORD;
const int myChannelNumber = SECRET_myChannelNumber;
const char* myApiKey = SECRET_myApiKey;
const char* server = "api.thingspeak.com";


// 'wifi', 16x16px
const unsigned char wifi_icon [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x3f, 0xfc, 0x70, 0x0e, 0xe7, 0xe7, 0x1f, 0xf8, 0x38, 0x1c, 
	0x03, 0xc0, 0x0f, 0xf0, 0x04, 0x20, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00
};

// 'wifi_disconnected', 16x16px
const unsigned char wifi_disconnected_icon [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x07, 0xf0, 0x1c, 0x60, 0x30, 0xcc, 0x27, 0x84, 
	0x0d, 0x30, 0x03, 0x00, 0x06, 0x60, 0x0c, 0x00, 0x09, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00
};

// 'upload', 16x16px
const unsigned char upload_icon [] PROGMEM = {
	0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1e, 0x78, 0x3c, 0x3c, 0x78, 0x1e, 0xfc, 0x3f, 
	0xfc, 0x3f, 0x7c, 0x3e, 0x1c, 0x38, 0x1c, 0x38, 0x1c, 0x38, 0x1f, 0xf8, 0x1f, 0xf8, 0x0f, 0xf0
};

// 'mic', 16x16px
const unsigned char mic_icon [] PROGMEM = {
	0x03, 0xc0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x17, 0xe8, 0x37, 0xec, 
	0x37, 0xec, 0x17, 0xe8, 0x1b, 0xd8, 0x0c, 0x30, 0x07, 0xe0, 0x01, 0x80, 0x01, 0x80, 0x0f, 0xf0
};

const int SAMPLE_RATE = 44100;  // Frecuencia de muestreo
const int SAMPLES_PER_SECOND = SAMPLE_RATE;  // Número de muestras por segundo
const int BUFFER_SIZE = 64;  // Tamaño del buffer para la lectura de muestras

// Declarar cliente WIFI
WiFiClient client;

// i2s configuracion
void i2s_install() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S, // Actualizado
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

// Custom functions display
void displayText(uint8_t textSize, uint16_t textColor, int16_t x, int16_t y, const String& message) {
  display.setTextSize(textSize);
  display.setTextColor(textColor);
  display.setCursor(x, y);
  display.println(message);
}

// Función helper para mostrar texto desde PROGMEM
void displayTextProgmem(uint8_t textSize, uint16_t textColor, int16_t x, int16_t y, const char* text) {
  display.setTextSize(textSize);
  display.setTextColor(textColor);
  display.setCursor(x, y);
  display.println((__FlashStringHelper*)text);
}

// main setup
void setup() {
  Serial.begin(115200);
  Serial.println((__FlashStringHelper*)TEXT_SETUP_I2S);

  // Instalar y configurar I2S
  i2s_install();
  i2s_setpin();

  i2s_start(I2S_PORT); 

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println((__FlashStringHelper*)TEXT_SSD1306_FAILED);
    for(;;);
  }

  // Screen 1 - Conectando
  display.clearDisplay();
  displayTextProgmem(1, SSD1306_WHITE, 24, 24, TEXT_CONNECTING);
  display.display();
  delay(1500); 

  // Configurar conexion WIFI
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println((__FlashStringHelper*)TEXT_WIFI_NOT_CONNECTED_DEBUG);

    display.clearDisplay();
    displayTextProgmem(1, SSD1306_WHITE, 0, 24, TEXT_WIFI_NOT_CONNECTED);
    display.display();
  }
  Serial.println((__FlashStringHelper*)TEXT_WIFI_CONNECTED_DEBUG);
  Serial.print((__FlashStringHelper*)TEXT_LOCAL_IP_DEBUG);
  Serial.println(WiFi.localIP());

  // Screen 2 - WiFi conectado
  display.clearDisplay();
  displayTextProgmem(1, SSD1306_WHITE, 0, 24, TEXT_WIFI_CONNECTED);
  displayText(1, SSD1306_WHITE, 0, 46, String((__FlashStringHelper*)TEXT_LOCAL_IP) + String(WiFi.localIP()));
  display.display();
  delay(1500); 

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  // Screen 3 - Pantalla de presentación
  display.clearDisplay();
  displayTextProgmem(1, SSD1306_WHITE, 30, 0, TEXT_NOISE_MONITOR);
  displayTextProgmem(2, SSD1306_WHITE, 24, 24, TEXT_SST_IOT);
  displayTextProgmem(1, SSD1306_WHITE, 0, 46, TEXT_DEVELOPED_BY);
  displayTextProgmem(1, SSD1306_WHITE, 0, 56, TEXT_AUTHOR);
  display.display();
  delay(2000);
}

void loop() {
  int32_t samples[BUFFER_SIZE];
  size_t bytes_read;
  int sample_count = 0;
  float total_db = 0;

  unsigned long start_time = millis();

  while (millis() - start_time < 1000) {
    i2s_read(I2S_PORT, &samples, sizeof(samples), &bytes_read, portMAX_DELAY);
    
    if (bytes_read > 0) {
      for (int i = 0; i < bytes_read / sizeof(int32_t); i++) {
        float sampleAbs = abs(samples[i]) * SCALE_FACTOR;

        if (sampleAbs > 0) {
          float dB = 20.0 * log10(sampleAbs / REFERENCE_LEVEL);
          total_db += dB;
          sample_count++;
        }
      }
    }
  }

  display.clearDisplay();

  if (sample_count > 0) {
    float currentDB = round((total_db / sample_count) * 10.0) / 10.0;
    Serial.printf(TEXT_DB_AVERAGE_FORMAT, currentDB);
    
    // Actualizar promedio general
    totalSamples++;
    overallAverageDB = ((overallAverageDB * (totalSamples - 1)) + currentDB) / totalSamples;
    
    // Actualizar promedio del último minuto
    lastMinuteReadings[lastMinuteIndex] = currentDB;
    lastMinuteIndex = (lastMinuteIndex + 1) % SAMPLES_PER_MINUTE;
    
    float lastMinuteTotal = 0;
    for (int i = 0; i < SAMPLES_PER_MINUTE; i++) {
      lastMinuteTotal += lastMinuteReadings[i];
    }
    lastMinuteAverageDB = lastMinuteTotal / SAMPLES_PER_MINUTE;
    
    display.setTextSize(3);
    display.setCursor(24, 20);
    display.print(currentDB, 1);

    display.setTextSize(2);
    display.setCursor(84, 26);
    display.print((__FlashStringHelper*)TEXT_DB_UNIT);

    display.setTextSize(1);
    display.setCursor(0, 4);
    display.print((__FlashStringHelper*)(currentDB >= DB_THRESHOLD ? TEXT_LEVEL_HIGH : TEXT_LEVEL_LOW));

    // Verificar la conexión WiFi
    if (WiFi.status() == WL_CONNECTED) {
      display.drawBitmap(112, 0, wifi_icon, 16, 16, WHITE);
      
      // Enviar datos a ThingSpeak
      ThingSpeak.setField(1, currentDB);
    
      int x = ThingSpeak.writeFields(myChannelNumber,myApiKey);

      if(x == 200){
        Serial.println((__FlashStringHelper*)TEXT_DATA_PUSHED);
        display.drawBitmap(80, 0, upload_icon, 16, 16, WHITE);
      } else {
        Serial.print((__FlashStringHelper*)TEXT_PUSH_ERROR);
        Serial.println(String(x));
      }    
    } else {
      display.drawBitmap(112, 0, wifi_disconnected_icon, 16, 16, WHITE);
    }

    // Siempre mostramos el icono del micrófono
    display.drawBitmap(96, 0, mic_icon, 16, 16, WHITE);

    // Mostrar promedio general y promedio del último minuto
    display.setTextSize(1);
    display.setCursor(0, SCREEN_HEIGHT - 20);
    display.print((__FlashStringHelper*)TEXT_AVG_ALL);
    display.print(overallAverageDB, 1);
    display.print((__FlashStringHelper*)TEXT_DB_SUFFIX);
    
    display.setCursor(0, SCREEN_HEIGHT - 10);
    display.print((__FlashStringHelper*)TEXT_AVG_1MIN);
    display.print(lastMinuteAverageDB, 1);
    display.print((__FlashStringHelper*)TEXT_DB_SUFFIX);

  } else {
    Serial.println((__FlashStringHelper*)TEXT_NO_VALID_SAMPLES);

    display.setTextSize(1);
    display.setCursor(36, 0);
    display.print((__FlashStringHelper*)TEXT_DISCONNECTED);

    display.setTextSize(2);
    display.setCursor(24, 16);
    display.print((__FlashStringHelper*)TEXT_NO_DATA);

    if (WiFi.status() != WL_CONNECTED) {
      display.drawBitmap(112, 0, wifi_disconnected_icon, 16, 16, WHITE);
    }
  }

  display.display();
  delay(1000);  
}