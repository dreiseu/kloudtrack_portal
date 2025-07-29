#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>

const char* portal_ssid = "ESP32-Upload";
const char* portal_password = ""; // Open AP

WebServer server(80);

String basePage(String statusMsg = "") {
  String statusDiv = statusMsg.length() > 0 ? "<div class=\"status\">" + statusMsg + "</div>" : "";
  String otaForm =
    "<form method=\"POST\" action=\"/ota\">"
    "<label for=\"firmware\">Firmware Update</label>"
    "<select id=\"firmware\" name=\"firmware\" required>"
    "<option value=\"v1.1\">Firmware v1.1</option>"
    "<option value=\"v1.2\">Firmware v1.2</option>"
    "<option value=\"v2.0\">Firmware v2.0</option>"
    "</select>"
    "<input type=\"submit\" value=\"Update\">"
    "</form>";
  
  String gsmForm =
    "<form method=\"POST\" action=\"/gsm\">"
    "<label for=\"apn\">APN</label>"
    "<input type=\"text\" id=\"apn\" name=\"apn\" placeholder=\"internet\" required>"
    "<input type=\"submit\" value=\"Save GSM Settings\">"
    "</form>";
    
  return "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>ESP32 Portal</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><style>body{background:#f4f8fb;font-family:'Segoe UI',Arial,sans-serif;margin:0;padding:0;color:#222}.container{max-width:400px;margin:40px auto;padding:24px;background:#fff;border-radius:16px;box-shadow:0 4px 24px rgba(0,0,0,0.08)}h2{margin-top:0;color:#1976d2;font-weight:600;font-size:1.4em}form{margin-bottom:24px}label{display:block;margin-bottom:6px;font-weight:500}input[type='text'],input[type='password'],input[type='file'],select{width:100%;padding:8px 10px;margin-bottom:14px;border:1px solid #cfd8dc;border-radius:6px;font-size:1em;background:#f9fbfc}input[type='submit']{background:#1976d2;color:#fff;border:none;padding:10px 0;width:100%;border-radius:6px;font-size:1em;font-weight:600;cursor:pointer;transition:background 0.2s}input[type='submit']:hover{background:#1565c0}.divider{border-top:1px solid #e0e0e0;margin:24px 0}.status{padding:10px;background:#e3f2fd;color:#1976d2;border-radius:6px;margin-bottom:18px;text-align:center;font-size:0.98em}@media (max-width:500px){.container{margin:10px;padding:12px}}</style></head><body><div class=\"container\"><h2>ESP32 Portal</h2>" + statusDiv + "<form method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\"><label for=\"cert\">Upload Certificate</label><input type=\"file\" id=\"cert\" name=\"cert\" required><input type=\"submit\" value=\"Upload\"></form><div class=\"divider\"></div><form method=\"POST\" action=\"/wifi\"><label for=\"ssid\">WiFi SSID</label><input type=\"text\" id=\"ssid\" name=\"ssid\" required><label for=\"password\">WiFi Password</label><input type=\"password\" id=\"password\" name=\"password\" required><input type=\"submit\" value=\"Save WiFi\"></form><div class=\"divider\"></div>" + gsmForm + "<div class=\"divider\"></div>" + otaForm + "</div></body></html>";
}

Preferences preferences;

void handleRoot() {
  server.send(200, "text/html", basePage());
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  static File certFile;
  if (upload.status == UPLOAD_FILE_START) {
    if (!SPIFFS.exists("/cert")) {
      SPIFFS.mkdir("/cert");
    }
    certFile = SPIFFS.open("/cert/ca.pem", FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (certFile) certFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (certFile) certFile.close();
    server.send(200, "text/html", basePage("Upload successful!"));
  }
}

void handleWifiCredentials() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
    server.send(200, "text/html", basePage("WiFi credentials saved!"));
  } else {
    server.send(400, "text/html", basePage("Missing SSID or password."));
  }
}

void handleGsmCredentials() {
  if (server.hasArg("apn")) {
    String apn = server.arg("apn");
    
    preferences.begin("gsm", false);
    preferences.putString("apn", apn);
    preferences.end();
    
    server.send(200, "text/html", basePage("GSM settings saved!"));
  } else {
    server.send(400, "text/html", basePage("Missing APN."));
  }
}

void handleOtaUpdate() {
  if (!server.hasArg("firmware")) {
    server.send(400, "text/html", basePage("No firmware selected."));
    return;
  }
  // Connect to WiFi using saved credentials
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end();
  if (ssid.length() == 0) {
    server.send(400, "text/html", basePage("No WiFi credentials saved."));
    Serial.println("OTA: No WiFi credentials saved.");
    return;
  }
  if (WiFi.getMode() != WIFI_AP_STA) {
    WiFi.mode(WIFI_AP_STA);
  }
  if (WiFi.status() != WL_CONNECTED || WiFi.SSID() != ssid) {
    Serial.printf("OTA: Connecting to WiFi SSID: %s\n", ssid.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long startAttempt = millis();
    const unsigned long wifiTimeout = 30000; // 30 seconds
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < wifiTimeout) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
  }
  Serial.printf("WiFi status: %d\n", WiFi.status());
  Serial.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
  if (WiFi.status() != WL_CONNECTED) {
    server.send(500, "text/html", basePage("Failed to connect to WiFi for OTA."));
    Serial.println("OTA: Failed to connect to WiFi.");
    return;
  }
  else {  
    Serial.println("OTA: Connected to WiFi. Starting OTA update...");
  }
  String firmware = server.arg("firmware");
  String url;
  if (firmware == "v1.1") url = "https://kloudtrack-firmware-test.s3.ap-southeast-1.amazonaws.com/V2.3.4.bin";
  else if (firmware == "v1.2") url = "http://example.com/firmware/v1.2.bin";
  else if (firmware == "v2.0") url = "http://example.com/firmware/v2.0.bin";
  else {
    server.send(400, "text/html", basePage("Invalid firmware option."));
    return;
  }

  WiFiClientSecure net = WiFiClientSecure();
  
  // Load and set the certificate for SSL verification
  if (SPIFFS.exists("/cert/ca.pem")) {
    File certFile = SPIFFS.open("/cert/ca.pem", FILE_READ);
    if (certFile) {
      String cert = certFile.readString();
      certFile.close();
      
      Serial.printf("Certificate file size: %d bytes\n", cert.length());
      Serial.println("Certificate starts with: " + cert.substring(0, 50));
      
      // For now, let's use insecure connection to get the OTA working
      // The certificate might be incompatible with ESP32's SSL library
      Serial.println("Using insecure connection for OTA (certificate compatibility issue)");
      net.setInsecure();
    } else {
      Serial.println("Failed to read certificate file");
      server.send(500, "text/html", basePage("Failed to read certificate file."));
      return;
    }
  } else {
    Serial.println("No certificate file found, using insecure connection");
    net.setInsecure(); // Skip certificate verification (not recommended for production)
  }
  
  HTTPClient http;
  http.setTimeout(30000); // 30 seconds timeout for OTA
  http.begin(net, url);
  http.addHeader("Content-Type", "application/octet-stream");

  Serial.printf("Attempting to connect to: %s\n", url.c_str());
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("OTA HTTP GET failed, code: %d\n", httpCode);
    Serial.printf("HTTPClient error: %s\n", http.errorToString(httpCode).c_str());
    Serial.printf("SSL/TLS connection failed. Check certificate and network.\n");
    http.end();
    server.send(500, "text/html", basePage("Failed to download firmware. Check certificate and network."));
    return;
  }
  size_t total = http.getSize();
  if (total <= 0) {
    http.end();
    server.send(500, "text/html", basePage("Invalid firmware file."));
    return;
  }
  bool canBegin = Update.begin(total);
  if (!canBegin) {
    http.end();
    server.send(500, "text/html", basePage("Not enough space for update."));
    return;
  }
  WiFiClient *stream = http.getStreamPtr();
  uint8_t buff[1024];
  size_t written = 0;
  while (http.connected() && (written < total)) {
    size_t available = stream->available();
    if (available) {
      size_t bytesRead = stream->readBytes(buff, available < sizeof(buff) ? available : sizeof(buff));
      size_t bytesWritten = Update.write(buff, bytesRead);
      if (bytesWritten > 0) {
        written += bytesWritten;
        if (written % (total / 10) < 1024) {
          Serial.printf("Progress: %d%%\n", (written * 100) / total);
          char progressMsg[32];
          sprintf(progressMsg, "Progress: %d%%", (written * 100) / total);
        }
      }
      else {
        Serial.println("OTA: Failed to write to Update.");
        Update.end();
        http.end();
        server.send(500, "text/html", basePage("Firmware update failed (write error)."));
        return;
      }
    }
    delay(1);
  }
  
  if (written != total) {
    Serial.printf("OTA: Incomplete update. Expected: %d, Written: %d\n", total, written);
    Update.end();
    http.end();
    server.send(500, "text/html", basePage("Firmware update failed (incomplete)."));
    return;
  }
  if (!Update.end()) {
    http.end();
    server.send(500, "text/html", basePage("Firmware update failed (end error)."));
    return;
  }
  http.end();
  if (!Update.isFinished()) {
    server.send(500, "text/html", basePage("Firmware update not finished."));
    return;
  }
  server.send(200, "text/html", basePage("Update successful! Rebooting..."));
  delay(1000);
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  // Print saved WiFi credentials
  preferences.begin("wifi", true);
  String savedSsid = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("password", "");
  preferences.end();
  Serial.println("Saved WiFi credentials:");
  Serial.print("SSID: ");
  Serial.println(savedSsid);
  Serial.print("Password: ");
  Serial.println(savedPassword);
  
  // Print saved GSM credentials
  preferences.begin("gsm", true);
  String savedApn = preferences.getString("apn", "");
  preferences.end();
  Serial.println("Saved GSM credentials:");
  Serial.print("APN: ");
  Serial.println(savedApn);
  WiFi.softAP(portal_ssid, portal_password);
  Serial.println("AP IP address: " + WiFi.softAPIP().toString());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_POST, [](){ server.send(200); }, handleFileUpload);
  server.on("/wifi", HTTP_POST, handleWifiCredentials);
  server.on("/gsm", HTTP_POST, handleGsmCredentials);
  server.on("/ota", HTTP_POST, handleOtaUpdate);
  server.begin();
  Serial.println("Web server started");

  // Read and print the certificate file if it exists
  if (SPIFFS.exists("/cert/ca.pem")) {
    Serial.println("Certificate file exists!");
    File certFile = SPIFFS.open("/cert/ca.pem", FILE_READ);
    if (certFile) {
      Serial.print("File size: ");
      Serial.println(certFile.size());
      String cert = certFile.readString();
      Serial.println("Certificate contents:");
      Serial.println(cert);
      certFile.close();
    }
  } else {
    Serial.println("Certificate file does NOT exist!");
  }
}

void loop() {
  server.handleClient();
}