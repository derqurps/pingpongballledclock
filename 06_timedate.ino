
String formattedTime;
String timeStamp;

timeInt getNewTime()
{
  int seconds = LocalTimezone.second();
  bool showsec = false;
  if (seconds%2==0) {
    showsec = true;
  }
  return (timeInt){LocalTimezone.hour(), LocalTimezone.minute(), showsec};
}

void printLocalTime()
{
  #ifdef DEBUGGING
    Serial.println("Local time: " + LocalTimezone.dateTime());
  #endif
}

void timeSetup() {
  //Needed for time functions
  LocalTimezone.setPosix(localTimezoneString);
  setInterval(60);
  #ifdef SECRET_NTP_SERVER
  setServer(SECRET_NTP_SERVER);
  #endif
  waitForSync();
  #ifdef DEBUGGING
    Serial.println("timezone initialized");
  #endif
}
