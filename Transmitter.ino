#include <WiFi.h>
#include <esp_now.h>

// Channel pin configuration
#define CH1 34  // Throttle (ADC1_CH6)
#define CH2 35  // Yaw (ADC1_CH7)
#define CH3 36  // Pitch (ADC1_CH4)
#define CH4 39  // Roll (ADC1_CH5)
#define CH5 25  // Arm switch
#define CH6 26  // Mode switch

// Receiver MAC address (update this)
uint8_t receiverMac[] = {0x40, 0x91, 0x51, 0xFC, 0x87, 0x7C};

// Packet structure
typedef struct {
  uint16_t channels[6];
  uint32_t packet_id;
} TransmitterData;

uint32_t packet_counter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Configure input pins
  pinMode(CH5, INPUT_PULLUP);
  pinMode(CH6, INPUT_PULLUP);
  
  // Set ADC resolution
  analogReadResolution(12);  // 0-4095 range
  
  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    ESP.restart();
  }
  
  // Configure peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    ESP.restart();
  }
}

void loop() {
  TransmitterData data;
  
  // Read channel values (with debouncing)
  data.channels[0] = map(analogRead(CH1), 0, 4095, 1000, 2000);  // Throttle
  data.channels[1] = map(analogRead(CH2), 0, 4095, 1000, 2000);  // Yaw
  data.channels[2] = map(analogRead(CH3), 0, 4095, 1000, 2000);  // Pitch
  data.channels[3] = map(analogRead(CH4), 0, 4095, 1000, 2000);  // Roll
  data.channels[4] = digitalRead(CH5) ? 2000 : 1000;             // Arm
  data.channels[5] = digitalRead(CH6) ? 2000 : 1000;             // Mode
  
  data.packet_id = packet_counter++;
  
  // Transmit data
  esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&data, sizeof(data));
  
  if (result != ESP_OK) {
    Serial.println("Transmission failed");
  }
  
  delay(20);  // 50Hz update rate
}
