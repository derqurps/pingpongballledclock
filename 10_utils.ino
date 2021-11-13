void checkForRestart() {
  if (restartNow) {
    ESP.restart();
  }
}
