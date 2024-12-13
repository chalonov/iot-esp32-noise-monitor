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
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // Maestro y solo recepción
    .sample_rate = SAMPLE_RATE,         // Frecuencia de muestreo de 44.1kHz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // 16 bits por muestra
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,   // Solo el canal izquierdo
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,      // Prioridad de la interrupción
    .dma_buf_count = 8,                            // Cantidad de buffers DMA
    .dma_buf_len = 64,                             // Tamaño de cada buffer DMA
    .use_apll = false,                             // No usar APLL
    .tx_desc_auto_clear = false,                   // Deshabilitar auto-limpieza de TX
    .fixed_mclk = 0
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,    // Pin del reloj de bits
    .ws_io_num = I2S_WS,      // Pin del word select (LRCLK)
    .data_out_num = I2S_PIN_NO_CHANGE, // Deshabilitar salida de datos (no se usa)
    .data_in_num = I2S_SD     // Pin de entrada de datos
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

// main setup
void setup() {
  Serial.begin(115200);
  Serial.println("Setup I2S ...");

  // Instalar y configurar I2S
  i2s_install();
  i2s_setpin();

  i2s_start(I2S_PORT); 

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Screen 1
  display.clearDisplay();
  displayText(1, SSD1306_WHITE, 24, 24, F("Connecting..."));
  display.display();
  delay(1500); 

  // Configurar conexion WIFI
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Wifi not connected");

    display.clearDisplay();
    displayText(1, SSD1306_WHITE, 0, 24, F("Wifi not connected"));
    display.display();
  }
  Serial.println("Wifi connected !");
  Serial.println("Local IP: " + String(WiFi.localIP()));

  // Screen 2
  display.clearDisplay();
  displayText(1, SSD1306_WHITE, 0, 24, F("Wifi connected!"));
  displayText(1, SSD1306_WHITE, 0, 46, "Local IP: " + String(WiFi.localIP()));
  display.display();
  delay(1500); 

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  // Screen 3
  display.clearDisplay();
  displayText(1, SSD1306_WHITE, 30, 0, F("NOISE MONITOR"));
  displayText(2, SSD1306_WHITE, 24, 24, F("SST-IoT"));
  displayText(1, SSD1306_WHITE, 0, 46, F("Developed by:"));
  displayText(1, SSD1306_WHITE, 0, 56, F("Ing. Gonzalo Novoa"));
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
    Serial.printf("Promedio de dB (2s): %.1f dB\n", currentDB);
    
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
    display.print(" dB");

    display.setTextSize(1);
    display.setCursor(0, 4);
    display.print(currentDB >= DB_THRESHOLD ? "Lvl: HIGH" : "Lvl: LOW");

    // Verificar la conexión WiFi
    if (WiFi.status() == WL_CONNECTED) {
      display.drawBitmap(112, 0, wifi_icon, 16, 16, WHITE);
      
      // Enviar datos a ThingSpeak
      ThingSpeak.setField(1, currentDB);
    
      int x = ThingSpeak.writeFields(myChannelNumber,myApiKey);

      if(x == 200){
        Serial.println("Data pushed successfully");
        display.drawBitmap(80, 0, upload_icon, 16, 16, WHITE);
      } else {
        Serial.println("Push error" + String(x));
      }    
    } else {
      // Si no hay conexión WiFi, mostramos el icono de WiFi tachado
      display.drawBitmap(112, 0, wifi_disconnected_icon, 16, 16, WHITE);
    }

    // Siempre mostramos el icono del micrófono
    display.drawBitmap(96, 0, mic_icon, 16, 16, WHITE);

    // Mostrar promedio general y promedio del último minuto
    display.setTextSize(1);
    display.setCursor(0, SCREEN_HEIGHT - 20);
    display.print("Avg(All):");
    display.print(overallAverageDB, 1);
    display.print("dB");
    
    display.setCursor(0, SCREEN_HEIGHT - 10);
    display.print("Avg(1min):");
    display.print(lastMinuteAverageDB, 1);
    display.print("dB");

  } else {
    Serial.println("No se registraron muestras válidas");

    display.setTextSize(1);
    display.setCursor(36, 0);
    display.print("Disconnected");

    display.setTextSize(2);
    display.setCursor(24, 16);
    display.print("No Data");

    // Mostramos el icono de WiFi tachado si no hay conexión
    if (WiFi.status() != WL_CONNECTED) {
      display.drawBitmap(112, 0, wifi_disconnected_icon, 16, 16, WHITE);
    }
  }

  display.display();

  // Tiempo antes de la próxima lectura (ms)
  delay(1000);  
}