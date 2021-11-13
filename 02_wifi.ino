

// Initialize WiFi
void wifiSetup() {

  WiFi.onEvent(WiFiEvent);
  
  connectToWifi();
  
  #ifdef DEBUGGING
    Serial.println("wifi setup finished");
  #endif
}

void connectToWifi() {
  WiFi.begin(ssid, password);
  #ifdef DEBUGGING
    Serial.print("Connecting to WiFi ..");
  #endif
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      #ifdef DEBUGGING
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
      #endif
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      #ifdef DEBUGGING
        Serial.println("Disconnected from Wi-Fi.");
      #endif
      mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      wifiReconnectTimer.once(2, connectToWifi);
    break;
  }
}
