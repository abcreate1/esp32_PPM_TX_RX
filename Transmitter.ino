#include <WiFi.h>
#include <esp_now.h>

#define CH1 34
#define CH2 35
#define CH3 36
#define CH4 39
#define CH5 25
#define CH6 26

uint8_t receiverMac[] = {0x40, 0x91, 0x51, 0xFC, 0x87, 0x7C};

typedef struct {
  uint16_t channels[6];
  uint32_t packet_id;
} TransmitterData;

uint32_t packet_counter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(CH5, INPUT_PULLUP);
  pinMode(CH6, INPUT_PULLUP);
  
  analogReadResolution(12);
  
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW इनिशियलाइजेशन फेल");
    ESP.restart();
  }
  
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("पेयर एड फेल");
    ESP.restart();
  }
}

void loop() {
  TransmitterData data;
  
  data.channels[0] = map(analogRead(CH1), 0, 4095, 1000, 2000);
  data.channels[1] = map(analogRead(CH2), 0, 4095, 1000, 2000);
  data.channels[2] = map(analogRead(CH3), 0, 4095, 1000, 2000);
  data.channels[3] = map(analogRead(CH4), 0, 4095, 1000, 2000);
  data.channels[4] = digitalRead(CH5) ? 2000 : 1000;
  data.channels[5] = digitalRead(CH6) ? 2000 : 1000;
  
  data.packet_id = packet_counter++;
  
  esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&data, sizeof(data));
  
  if (result != ESP_OK) {
    Serial.println("ट्रांसमिशन फेल");
  }
  
  delay(20);
}
