# ESP32 Portal - IoT Device Management Portal

A comprehensive ESP32-based web portal for IoT device management, featuring WiFi configuration, GSM settings, certificate management, and secure OTA (Over-The-Air) firmware updates.

## 🚀 Features

- **📡 WiFi Configuration** - Save and manage WiFi credentials
- **📱 GSM Settings** - Configure APN settings for cellular connectivity
- **🔐 Certificate Management** - Upload SSL certificates for secure connections
- **🔄 OTA Updates** - Secure firmware updates with progress tracking
- **📱 Responsive Web Interface** - Modern, mobile-friendly UI
- **💾 Persistent Storage** - Settings saved across reboots

## 📋 Requirements

### Hardware
- ESP32 development board
- Micro USB cable for programming

### Software
- PlatformIO IDE or Arduino IDE
- ESP32 board support package

## 🛠️ Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/esp32-portal.git
   cd esp32-portal
   ```

2. **Open in PlatformIO**
   - Open the project in PlatformIO IDE
   - Or use Arduino IDE with ESP32 board support

3. **Upload to ESP32**
   - Connect your ESP32 via USB
   - Upload the code to your device

## 🔧 Configuration

### Initial Setup
1. **Power on the ESP32**
2. **Connect to WiFi network** `ESP32-Upload` (open network)
3. **Access the portal** at `192.168.4.1`

### Portal Features

#### 📡 WiFi Configuration
- Enter your WiFi SSID and password
- Settings are saved permanently
- Device will connect to saved WiFi for OTA updates

#### 📱 GSM Settings
- Configure APN for cellular connectivity
- Useful when WiFi is not available
- Common APNs:
  - AT&T: `internet`
  - Verizon: `vzwinternet`
  - T-Mobile: `fast.t-mobile.com`
  - Most carriers: `internet`

#### 🔐 Certificate Upload
- Upload SSL certificates for secure HTTPS connections
- Required for secure OTA updates
- Supports PEM format certificates

#### 🔄 OTA Firmware Updates
- Select firmware version from dropdown
- Secure download from HTTPS servers
- Progress tracking during update
- Automatic reboot after successful update

## 📁 Project Structure

```
esp32-portal/
├── src/
│   └── main.cpp          # Main application code
├── platformio.ini        # PlatformIO configuration
├── README.md            # This file
└── LICENSE              # License information
```

## 🔧 Usage Examples

### Accessing GSM Settings in Your Code
```cpp
#include <Preferences.h>

Preferences preferences;
preferences.begin("gsm", true);
String apn = preferences.getString("apn", "");
preferences.end();

// Use APN with your GSM module
```

### Accessing WiFi Settings
```cpp
Preferences preferences;
preferences.begin("wifi", true);
String ssid = preferences.getString("ssid", "");
String password = preferences.getString("password", "");
preferences.end();
```

## 🚨 Security Notes

- The portal uses an open WiFi network for initial configuration
- SSL certificates are required for secure OTA updates
- Consider implementing authentication for production use
- GSM credentials are stored securely in ESP32's NVS

## 🔧 Troubleshooting

### Common Issues

1. **Portal not accessible**
   - Ensure you're connected to `ESP32-Upload` network
   - Check if ESP32 is powered on
   - Try accessing `192.168.4.1` in browser

2. **OTA update fails**
   - Verify SSL certificate is uploaded
   - Check WiFi connection
   - Ensure firmware URL is accessible

3. **GSM not connecting**
   - Verify APN settings for your carrier
   - Check SIM card is properly inserted
   - Ensure GSM module is powered

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- ESP32 Arduino core developers
- PlatformIO team
- ESP32 community for libraries and examples

## 📞 Support

If you encounter any issues or have questions:
- Open an issue on GitHub
- Check the troubleshooting section above
- Review ESP32 documentation

---

**Made with ❤️ for the IoT community** 