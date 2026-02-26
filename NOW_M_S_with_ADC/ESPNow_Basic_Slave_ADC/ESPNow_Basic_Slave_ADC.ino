/**
   ESPNOW - Basic communication - Slave
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Slave module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Slave >>

   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
   Step 3 : Once found, add Slave as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Slave

   Flow: Slave
   Step 1 : ESPNow Init on Slave
   Step 2 : Update the SSID of Slave with a prefix of `slave`
   Step 3 : Set Slave in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Master and Slave have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Slave.
         Any devices can act as master or salve.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define CHANNEL 1
#define SERIAL_SEND 0

esp_now_peer_info_t masterPeer;
bool masterKnown = false;

uint8_t slaveTx[4] = {1,2,3,4}; // example payload from slave to master

void rememberMasterAndAddPeer(const uint8_t *mac_addr) {
  if (masterKnown) return;

  memset(&masterPeer, 0, sizeof(masterPeer));
  memcpy(masterPeer.peer_addr, mac_addr, 6);
  masterPeer.channel = CHANNEL;
  masterPeer.encrypt = 0;
  masterPeer.ifidx = WIFI_IF_AP;   // ✅ important

  if (esp_now_add_peer(&masterPeer) == ESP_OK) {
    masterKnown = true;
    if (SERIAL_SEND == 1) {Serial.println("Master learned + peer added");}
  } else {
    if (SERIAL_SEND == 1) {Serial.println("Failed to add master as peer");}
  }
}



// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  if (SERIAL_SEND == 1) {Serial.print("Last Packet Recv from: ");  Serial.println(macStr);}
 
  if (SERIAL_SEND == 1) {Serial.print("Bytes (");}
  if (SERIAL_SEND == 1) {Serial.print(data_len);}
  if (SERIAL_SEND == 1) {Serial.print("): ");}

//  for (int i = 0; i < data_len; i++) {
//   Serial.print(data[i]); Serial.print("  ");
//    if (SERIAL_SEND == 1) {Serial.print(i == data_len - 1 ? '\n' : ' ');}
//  }

   Serial.print(data[1]);Serial.print(" ");
   Serial.print(data[2]);Serial.print(" ");
   Serial.print(data[3]);Serial.print(" ");
   Serial.print(data[4]);Serial.println();
   

  rememberMasterAndAddPeer(mac_addr);

  if (masterKnown) {
    if (SERIAL_SEND == 1) {Serial.println("masters found, sending data");}
    slaveTx[0]++; // change something so you see it updating
    esp_err_t r = esp_now_send(masterPeer.peer_addr, slaveTx, sizeof(slaveTx));
    if (SERIAL_SEND == 1) {Serial.print("slave esp_now_send ret: ");}
if (SERIAL_SEND == 1) {Serial.println((int)r);} // 0 means ESP_OK
  }


  
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (SERIAL_SEND == 1) {Serial.print("Slave send status: ");}
  if (SERIAL_SEND == 1) {Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");}
}

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    if (SERIAL_SEND == 1) {Serial.println("ESPNow Init Success");}
  }
  else {
    if (SERIAL_SEND == 1) {Serial.println("ESPNow Init Failed");}
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  const char *SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    if (SERIAL_SEND == 1) {Serial.println("AP Config failed.");}
  } else {
    if (SERIAL_SEND == 1) {Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));}
    if (SERIAL_SEND == 1) {Serial.print("AP CHANNEL "); Serial.println(WiFi.channel());}
  }
}

void setup() {
  Serial.begin(115200);
  if (SERIAL_SEND == 1) {Serial.println("ESPNow/Basic/Slave Example");}
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE); 
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode 
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
}


void loop() {
  // Chill
}
