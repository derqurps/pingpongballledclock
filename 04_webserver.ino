void webSetup() {

  /*AsyncCallbackJsonWebHandler* jsonHandler = new AsyncCallbackJsonWebHandler("/json", [](AsyncWebServerRequest *request, JsonVariant &json) {

    JsonObject object = json.as<JsonObject>();

    
    String retjson = getCurrentInputValues();
    request->send(200, "application/json", retjson);
    retjson = String();
    
  });

  server.addHandler(jsonHandler);*/

  AsyncElegantOTA.begin(&server);
  server.begin();
  
  #ifdef DEBUGGING
    Serial.println("webserver setup finished");
  #endif
}
