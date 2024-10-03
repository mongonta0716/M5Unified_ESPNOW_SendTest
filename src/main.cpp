#include <Arduino.h>
#include <M5Unified.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

esp_now_peer_info_t esp_ap;
const uint8_t *peer_addr = esp_ap.peer_addr;
const esp_now_peer_info_t *peer = &esp_ap;


// 同報したい端末数と端末のMACアドレスを指定します。
// 受信専門の場合は設定は不要
#define MAX_CLIENT 1
uint8_t mac[][6] = {
  {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa}, // 端末1
//  {0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb}, // 端末2
//  {0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc}, // 端末3
};

#define WIFI_DEFAULT_CHANNEL 1
#define WIFI_SECONDORY_CHANNEL 2

uint8_t send_data[250]; // ESP-NOWの送信バッファ(250Byteまで)

String send_str[2] = {
    "テスト",
    "Hello World"
};

// コールバック関数実行時に排他制御をする。
SemaphoreHandle_t xMutex = NULL;

void sendData(const uint8_t *data, size_t len) {
  BaseType_t xStatus;
  const TickType_t xTicksToWait = 1000U;
  xStatus = xSemaphoreTake(xMutex, xTicksToWait);
  if (xStatus == pdTRUE) {
    for (int i=0; i< MAX_CLIENT; i++) {
      for (int j=0; j<6; j++) {
        esp_ap.peer_addr[j] = (uint8_t)mac[i][j];
      }
      esp_now_send(peer_addr, data, len); 
      M5.Display.println("Send Data");
    } 
    // gfx.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", peer_addr[0], peer_addr[1], peer_addr[2], peer_addr[3], peer_addr[4], peer_addr[5]);
  }
  xSemaphoreGive(xMutex);
}

void peerClients() {
  BaseType_t xStatus;
  const TickType_t xTicksToWait = 1000U;
  xStatus = xSemaphoreTake(xMutex, xTicksToWait);
  if (xStatus == pdTRUE) {
    for (int i=0; i< MAX_CLIENT; i++) {
      for (int j=0; j<6; j++) {
        esp_ap.peer_addr[j] = (uint8_t)mac[i][j];
      }
      esp_err_t error = esp_now_add_peer(peer);
      if (error == ESP_OK){
        M5.Display.println("Success Peer");
      } else if (error == ESP_ERR_ESPNOW_EXIST) {
        M5.Display.println("Already added");
      } else {
        M5.Display.println("Failed to add peer");
      }
    }
  }
  xSemaphoreGive(xMutex);
}

void setup() {
  M5.begin();
  M5.Display.println("ESP_NOW Test");
  uint8_t mac_addr[6];
  esp_read_mac(mac_addr, ESP_MAC_WIFI_STA);
//  gfx.printf("%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  WiFi.mode(WIFI_STA);
  ESP_ERROR_CHECK(esp_wifi_set_channel(WIFI_DEFAULT_CHANNEL, WIFI_SECOND_CHAN_ABOVE));
  if (esp_now_init() == 0) {
    Serial.println("esp now init");
  } else {
    Serial.println("esp now init failed");
    M5.Display.println("esp now init failed");
  }


  xMutex = xSemaphoreCreateMutex();
}

void convertString2uint8(String param_str) {
    M5_LOGI("convertString2uint8");
    uint8_t buf[250];
    memcpy(&buf, param_str.c_str(), sizeof(param_str));
    peerClients();
    sendData(buf, sizeof(param_str));
}

void loop() {
    M5.update();
    if (M5.BtnA.wasClicked()) {
        convertString2uint8(send_str[0]);
    }
    if (M5.BtnB.wasClicked()) {
        convertString2uint8(send_str[1]);
    }
    
}