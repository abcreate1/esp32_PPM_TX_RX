#include <WiFi.h>
#include <esp_now.h>

// PPM output configuration
#define PPM_PIN 25
#define CHANNEL_COUNT 6
#define PPM_FRAME_LENGTH 22500  // 22.5ms
#define PPM_PULSE_WIDTH 400     // 400μs

// Default PPM values
uint16_t ppmValues[CHANNEL_COUNT] = {1500, 1500, 1500, 1500, 1000, 1000};
uint32_t last_received = 0;

// Packet structure
typedef struct {
  uint16_t channels[CHANNEL_COUNT];
  uint32_t packet_id;
} TransmitterData;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  TransmitterData receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  
  // Update channel values
  for(int i=0; i<CHANNEL_COUNT; i++) {
    ppmValues[i] = constrain(receivedData.channels[i], 1000, 2000);
  }
  last_received = millis();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // PPM output pin
  pinMode(PPM_PIN, OUTPUT);
  digitalWrite(PPM_PIN, LOW);
  
  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    ESP.restart();
  }
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("Receiver ready");
}

void loop() {
  // Check connection loss
  if(millis() - last_received > 1000) {
    // No data received for 1 second
    for(int i=0; i<CHANNEL_COUNT; i++) {
      ppmValues[i] = (i < 4) ? 1500 : 1000;  // Center/low values
    }
  }
  
  // Generate PPM signal
  digitalWrite(PPM_PIN, HIGH);
  delayMicroseconds(PPM_PULSE_WIDTH);
  digitalWrite(PPM_PIN, LOW);
  
  // Channel pulses
  for(int i=0; i<CHANNEL_COUNT; i++) {
    delayMicroseconds(ppmValues[i] - PPM_PULSE_WIDTH);
    digitalWrite(PPM_PIN, HIGH);
    delayMicroseconds(PPM_PULSE_WIDTH);
    digitalWrite(PPM_PIN, LOW);
  }
  
  // Complete the frame
  uint32_t frame_time = PPM_FRAME_LENGTH;
  for(int i=0; i<CHANNEL_COUNT; i++) frame_time -= ppmValues[i];
  delayMicroseconds(frame_time);
}
