#include <Arduino.h>
#include <SPIFFS.h>
#include <Preferences.h>

Preferences preferences;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=== Credential Clearing Test ===\n");

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS mounted successfully");

  // Show current credentials before clearing
  Serial.println("\n--- Current Stored Credentials ---");
  preferences.begin("credentials", true);
  String savedSsid = preferences.getString("wifi_ssid", "");
  String savedPassword = preferences.getString("wifi_password", "");
  String savedApn = preferences.getString("gsm_apn", "");
  preferences.end();

  Serial.println("WiFi SSID: " + (savedSsid.length() > 0 ? savedSsid : "(empty)"));
  Serial.println("WiFi Password: " + (savedPassword.length() > 0 ? savedPassword : "(empty)"));
  Serial.println("GSM APN: " + (savedApn.length() > 0 ? savedApn : "(empty)"));

  // Check certificate files
  Serial.println("\n--- Certificate Files ---");
  bool deviceCertExists = SPIFFS.exists("/cert/device.pem");
  bool privateKeyExists = SPIFFS.exists("/cert/private.pem");

  Serial.println("Device cert exists: " + String(deviceCertExists ? "YES" : "NO"));
  Serial.println("Private key exists: " + String(privateKeyExists ? "YES" : "NO"));

  if (deviceCertExists) {
    File f = SPIFFS.open("/cert/device.pem", FILE_READ);
    Serial.println("Device cert size: " + String(f.size()) + " bytes");
    f.close();
  }

  if (privateKeyExists) {
    File f = SPIFFS.open("/cert/private.pem", FILE_READ);
    Serial.println("Private key size: " + String(f.size()) + " bytes");
    f.close();
  }

  // Clear preferences
  Serial.println("\n--- Clearing Preferences ---");
  preferences.begin("credentials", false);

  if (preferences.clear()) {
    Serial.println("✓ Preferences cleared successfully");
  } else {
    Serial.println("✗ Failed to clear preferences");
  }

  preferences.end();

  // Delete certificate files
  Serial.println("\n--- Deleting Certificate Files ---");

  if (deviceCertExists) {
    if (SPIFFS.remove("/cert/device.pem")) {
      Serial.println("✓ Deleted /cert/device.pem");
    } else {
      Serial.println("✗ Failed to delete /cert/device.pem");
    }
  }

  if (privateKeyExists) {
    if (SPIFFS.remove("/cert/private.pem")) {
      Serial.println("✓ Deleted /cert/private.pem");
    } else {
      Serial.println("✗ Failed to delete /cert/private.pem");
    }
  }

  // Verify clearing
  Serial.println("\n--- Verification ---");
  preferences.begin("credentials", true);
  savedSsid = preferences.getString("wifi_ssid", "");
  savedPassword = preferences.getString("wifi_password", "");
  savedApn = preferences.getString("gsm_apn", "");
  preferences.end();

  Serial.println("WiFi SSID: " + (savedSsid.length() > 0 ? savedSsid : "(empty)"));
  Serial.println("WiFi Password: " + (savedPassword.length() > 0 ? savedPassword : "(empty)"));
  Serial.println("GSM APN: " + (savedApn.length() > 0 ? savedApn : "(empty)"));

  deviceCertExists = SPIFFS.exists("/cert/device.pem");
  privateKeyExists = SPIFFS.exists("/cert/private.pem");

  Serial.println("Device cert exists: " + String(deviceCertExists ? "YES" : "NO"));
  Serial.println("Private key exists: " + String(privateKeyExists ? "YES" : "NO"));

  // Summary
  Serial.println("\n=== Summary ===");
  if (savedSsid.length() == 0 && savedPassword.length() == 0 && savedApn.length() == 0 &&
      !deviceCertExists && !privateKeyExists) {
    Serial.println("✓ All credentials and certificates cleared successfully!");
  } else {
    Serial.println("⚠ Some data may still remain");
  }

  Serial.println("\nTest complete. Device will restart in 5 seconds...");
  delay(5000);
  ESP.restart();
}

void loop() {
  // Nothing to do here
}
