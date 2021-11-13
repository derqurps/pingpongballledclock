void mqttSetup() {

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASSWORD);
  
  #ifdef DEBUGGING
    Serial.println("mqtt setup finished");
  #endif
}

void mqttLoop() {
  EVERY_N_SECONDS( 10 ) {
    sendStatus();
    sendDimmer();
    sendTimeStatus();
    sendItalicStatus();
  }
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  sendStatus();
  sendDimmer();
  sendType();
  sendTimeStatus();
  sendItalicStatus();
  #ifdef DEBUGGING
    Serial.println(MQTT_RECEIVER_POWER);
    Serial.println(MQTT_RECEIVER_DIMMER);
    Serial.println(MQTT_RECEIVER_TYPE);
  #endif
  mqttClient.subscribe(MQTT_RECEIVER_POWER, 2);
  mqttClient.subscribe(MQTT_RECEIVER_DIMMER, 2);
  mqttClient.subscribe(MQTT_RECEIVER_TYPE, 2);
  mqttClient.subscribe(MQTT_RECEIVER_TIME, 2);
  mqttClient.subscribe(MQTT_RECEIVER_ITALIC, 2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  #ifdef DEBUGGING
    Serial.println("Disconnected from MQTT.");
  #endif
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  #ifdef DEBUGGING
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
  #endif
}

void onMqttUnsubscribe(uint16_t packetId) {
  #ifdef DEBUGGING
    Serial.println("Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
  #endif
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  String myPayload = String((char *)payload);
  String cutPayload = myPayload.substring(0,len);
  #ifdef DEBUGGING
    Serial.println(topic);
    
    for (size_t i = 0; i < len; ++i) {
      Serial.print(payload[i]);
    }
  #endif
  
  if(strcmp(topic, MQTT_RECEIVER_POWER) == 0) {
    setStatus(cutPayload);
  } else if(strcmp(topic, MQTT_RECEIVER_DIMMER) == 0) {
    setDimmer(cutPayload);
  } else if(strcmp(topic, MQTT_RECEIVER_TYPE) == 0) {
    setType(cutPayload);
  } else if(strcmp(topic, MQTT_RECEIVER_TIME) == 0) {
    setTimeOnOff(cutPayload);
  } else if(strcmp(topic, MQTT_RECEIVER_ITALIC) == 0) {
    setItalicOnOff(cutPayload);
  }
}

void onMqttPublish(uint16_t packetId) {
  #ifdef DEBUGGING
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
  #endif
}

void setItalicOnOff(String cutPayload) {
  if (cutPayload == "ON") {
    italic = true;
  } else if(cutPayload == "OFF") {
    italic = false;
  }
  sendItalicStatus();
}

void sendItalicStatus() {
  if (mqttClient.connected()) {
    mqttClient.publish(MQTT_PUBLISH_ITALIC, 2, true, italic?"ON":"OFF");
  }
}

void setTimeOnOff(String cutPayload) {
  if (cutPayload == "ON") {
    timeOnOff = true;
  } else if(cutPayload == "OFF") {
    timeOnOff = false;
  }
  sendTimeStatus();
}

void sendTimeStatus() {
  if (mqttClient.connected()) {
    mqttClient.publish(MQTT_PUBLISH_TIME, 2, true, timeOnOff?"ON":"OFF");
  }
}

void setStatus(String cutPayload) {
  if (cutPayload == "ON") {
    running = true;
  } else if(cutPayload == "OFF") {
    running = false;
  }
  sendStatus();
}

void sendStatus() {
  if (mqttClient.connected()) {
    mqttClient.publish(MQTT_PUBLISH_POWER, 2, true, running?"ON":"OFF");
  }
}

void setDimmer(String cutPayload) {
  try {
    dimming = cutPayload.toInt();
  } catch(...) {
    #ifdef DEBUGGING
      Serial.println("parsing to int has not worked");
    #endif
  }
  sendDimmer();
}

void sendDimmer() {
  if (mqttClient.connected()) {
    String d = String(dimming);
    mqttClient.publish(MQTT_PUBLISH_DIMMER, 2, true, (char*) d.c_str());
  }
}

void setType(String cutPayload) {
  try {
    gCurrentPatternNumber = cutPayload.toInt();
  } catch(...) {
    #ifdef DEBUGGING
      Serial.println("parsing to int has not worked");
    #endif
  }
  sendDimmer();
}

void sendType() {
  if (mqttClient.connected()) {
    char *s = (char*) String((char) gCurrentPatternNumber).c_str(); 
    mqttClient.publish(MQTT_PUBLISH_TYPE, 2, true, s);
  }
}
