// Phase 0 spike: report chip/flash/PSRAM/heap, then start WiFi+Matter with one outlet.
#include <Arduino.h>
#include <Matter.h>
#include <WiFi.h>

MatterOnOffPlugin outlet;

static void report(const char* tag) {
  Serial.printf("[%s] heap free=%u largest=%u | psram size=%u free=%u\n", tag,
                heap_caps_get_free_size(MALLOC_CAP_8BIT),
                heap_caps_get_largest_free_block(MALLOC_CAP_8BIT), ESP.getPsramSize(),
                ESP.getFreePsram());
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.printf("Chip: %s rev %d, %d cores, flash=%u bytes, sketch=%u bytes\n", ESP.getChipModel(),
                ESP.getChipRevision(), ESP.getChipCores(), ESP.getFlashChipSize(),
                ESP.getSketchSize());
  report("boot");

  outlet.begin();
  outlet.onChange([](bool state) {
    Serial.printf("Outlet -> %s\n", state ? "ON" : "OFF");
    return true;
  });
  Matter.begin();
  report("after Matter.begin");

  if (!Matter.isDeviceCommissioned()) {
    Serial.println("Not commissioned. Pair with Apple Home:");
    Serial.printf("  Manual code: %s\n", Matter.getManualPairingCode().c_str());
    Serial.printf("  QR URL:      %s\n", Matter.getOnboardingQRCodeUrl().c_str());
  }
}

void loop() {
  static uint32_t last;
  if (millis() - last > 5000) {
    last = millis();
    report(Matter.isDeviceConnected() ? "connected" : "waiting");
  }
}
