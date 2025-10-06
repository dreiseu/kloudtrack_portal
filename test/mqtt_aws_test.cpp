#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <Preferences.h>

// AWS IoT endpoint - UPDATE THIS with your endpoint
const char* aws_iot_endpoint = "your-endpoint.iot.ap-southeast-1.amazonaws.com";
const int aws_iot_port = 8883;

// MQTT topics
const char* publish_topic = "kloudtrack/test/outbound";
const char* subscribe_topic = "kloudtrack/test/inbound";

WiFiClientSecure net;
PubSubClient mqttClient(net);
Preferences preferences;

unsigned long lastPublish = 0;
int messageCount = 0;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

bool loadCertificates() {
  if (!SPIFFS.exists("/cert/device.pem")) {
    Serial.println("ERROR: Device certificate not found!");
    return false;
  }

  if (!SPIFFS.exists("/cert/private.pem")) {
    Serial.println("ERROR: Private key not found!");
    return false;
  }

  File certFile = SPIFFS.open("/cert/device.pem", FILE_READ);
  File keyFile = SPIFFS.open("/cert/private.pem", FILE_READ);

  if (!certFile || !keyFile) {
    Serial.println("ERROR: Failed to open certificate files!");
    return false;
  }

  String cert = certFile.readString();
  String key = keyFile.readString();

  certFile.close();
  keyFile.close();

  Serial.println("Certificate loaded: " + String(cert.length()) + " bytes");
  Serial.println("Private key loaded: " + String(key.length()) + " bytes");

  net.setCertificate(cert.c_str());
  net.setPrivateKey(key.c_str());

  // For AWS IoT, you may need to skip server verification or add root CA
  net.setInsecure(); // TODO: Add Amazon Root CA for production

  Serial.println("Certificates configured successfully");
  return true;
}

bool connectToWiFi() {
  preferences.begin("credentials", true);
  String ssid = preferences.getString("wifi_ssid", "");
  String password = preferences.getString("wifi_password", "");
  preferences.end();

  if (ssid.length() == 0) {
    Serial.println("ERROR: No WiFi credentials saved!");
    Serial.println("Please configure WiFi first");
    return false;
  }

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("ERROR: WiFi connection failed!");
    return false;
  }

  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  return true;
}

bool connectToAWS() {
  Serial.println("\n--- Connecting to AWS IoT ---");
  Serial.print("Endpoint: ");
  Serial.println(aws_iot_endpoint);
  Serial.print("Port: ");
  Serial.println(aws_iot_port);

  // Generate unique client ID based on MAC address
  String clientId = "ESP32_" + WiFi.macAddress();
  clientId.replace(":", "");

  Serial.print("Client ID: ");
  Serial.println(clientId);

  mqttClient.setServer(aws_iot_endpoint, aws_iot_port);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);

  Serial.println("Attempting MQTT connection...");

  if (mqttClient.connect(clientId.c_str())) {
    Serial.println("✓ MQTT connected successfully!");

    // Subscribe to test topic
    if (mqttClient.subscribe(subscribe_topic)) {
      Serial.print("✓ Subscribed to topic: ");
      Serial.println(subscribe_topic);
    } else {
      Serial.println("✗ Failed to subscribe");
    }

    return true;
  } else {
    Serial.print("✗ MQTT connection failed, rc=");
    Serial.println(mqttClient.state());

    switch(mqttClient.state()) {
      case -4: Serial.println("  MQTT_CONNECTION_TIMEOUT"); break;
      case -3: Serial.println("  MQTT_CONNECTION_LOST"); break;
      case -2: Serial.println("  MQTT_CONNECT_FAILED"); break;
      case -1: Serial.println("  MQTT_DISCONNECTED"); break;
      case 1: Serial.println("  MQTT_CONNECT_BAD_PROTOCOL"); break;
      case 2: Serial.println("  MQTT_CONNECT_BAD_CLIENT_ID"); break;
      case 3: Serial.println("  MQTT_CONNECT_UNAVAILABLE"); break;
      case 4: Serial.println("  MQTT_CONNECT_BAD_CREDENTIALS"); break;
      case 5: Serial.println("  MQTT_CONNECT_UNAUTHORIZED"); break;
    }

    return false;
  }
}

void publishTestMessage() {
  if (!mqttClient.connected()) {
    Serial.println("MQTT not connected, attempting reconnect...");
    if (!connectToAWS()) {
      return;
    }
  }

  messageCount++;

  String payload = "{";
  payload += "\"device_id\":\"" + WiFi.macAddress() + "\",";
  payload += "\"message_count\":" + String(messageCount) + ",";
  payload += "\"uptime\":" + String(millis() / 1000) + ",";
  payload += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  payload += "\"free_heap\":" + String(ESP.getFreeHeap());
  payload += "}";

  Serial.print("Publishing to ");
  Serial.print(publish_topic);
  Serial.print(": ");
  Serial.println(payload);

  if (mqttClient.publish(publish_topic, payload.c_str())) {
    Serial.println("✓ Message published successfully");
  } else {
    Serial.println("✗ Publish failed");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=== AWS IoT MQTT Test ===\n");

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("ERROR: SPIFFS mount failed!");
    while(1) delay(1000);
  }
  Serial.println("✓ SPIFFS mounted");

  // Load certificates
  if (!loadCertificates()) {
    Serial.println("\nERROR: Certificate loading failed!");
    Serial.println("Please upload certificates first");
    while(1) delay(1000);
  }

  // Connect to WiFi
  if (!connectToWiFi()) {
    Serial.println("\nERROR: WiFi connection failed!");
    while(1) delay(1000);
  }

  // Connect to AWS IoT
  if (!connectToAWS()) {
    Serial.println("\nERROR: AWS IoT connection failed!");
    Serial.println("Check your endpoint, certificates, and IoT policies");
    while(1) delay(5000);
  }

  Serial.println("\n=== Test Running ===");
  Serial.println("Publishing test messages every 10 seconds...");
  Serial.println("Listening for messages on: " + String(subscribe_topic));
  Serial.println();
}

void loop() {
  if (!mqttClient.connected()) {
    Serial.println("MQTT disconnected! Reconnecting...");
    if (connectToAWS()) {
      Serial.println("Reconnected successfully");
    } else {
      Serial.println("Reconnection failed, retrying in 5s...");
      delay(5000);
      return;
    }
  }

  mqttClient.loop();

  // Publish test message every 10 seconds
  if (millis() - lastPublish > 10000) {
    lastPublish = millis();
    publishTestMessage();
  }

  // Monitor WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    connectToWiFi();
  }
}
