# Contributing to ESP32 Portal

Thank you for your interest in contributing to the ESP32 Portal project! This document provides guidelines and information for contributors.

## 🤝 How to Contribute

### Reporting Bugs

1. **Check existing issues** - Search the issues to see if the bug has already been reported
2. **Create a new issue** - Use the bug report template and provide:
   - Clear description of the problem
   - Steps to reproduce
   - Expected vs actual behavior
   - Hardware/software environment details

### Suggesting Features

1. **Check existing issues** - Search for similar feature requests
2. **Create a feature request** - Use the feature request template and include:
   - Clear description of the feature
   - Use case and benefits
   - Implementation suggestions (if any)

### Code Contributions

1. **Fork the repository**
2. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes**:
   - Follow the coding standards below
   - Add tests if applicable
   - Update documentation
4. **Commit your changes**:
   ```bash
   git commit -m "Add: brief description of changes"
   ```
5. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```
6. **Create a Pull Request** with a clear description

## 📋 Coding Standards

### C++ Code Style

- Use **4 spaces** for indentation (no tabs)
- Use **snake_case** for variables and functions
- Use **PascalCase** for classes
- Use **UPPER_CASE** for constants
- Add comments for complex logic
- Keep functions focused and under 50 lines when possible

### Example:
```cpp
void handle_wifi_credentials() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        
        // Save credentials to persistent storage
        preferences.begin("wifi", false);
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.end();
        
        server.send(200, "text/html", basePage("WiFi credentials saved!"));
    } else {
        server.send(400, "text/html", basePage("Missing SSID or password."));
    }
}
```

### Commit Message Format

Use conventional commit format:
- `Add:` for new features
- `Fix:` for bug fixes
- `Update:` for improvements
- `Remove:` for deletions
- `Docs:` for documentation changes

Examples:
```
Add: GSM configuration support
Fix: SSL certificate validation issue
Update: Improve error handling in OTA updates
Docs: Add troubleshooting section to README
```

## 🧪 Testing

### Before Submitting

1. **Test your changes** on actual ESP32 hardware
2. **Verify all features work**:
   - WiFi configuration
   - GSM settings
   - Certificate upload
   - OTA updates
3. **Check for memory leaks** and stability
4. **Test edge cases** and error conditions

### Testing Checklist

- [ ] Code compiles without errors
- [ ] All existing features still work
- [ ] New features work as expected
- [ ] Error handling is appropriate
- [ ] Documentation is updated
- [ ] No memory leaks detected

## 📚 Documentation

### Code Documentation

- Add comments for complex functions
- Document function parameters and return values
- Explain any non-obvious logic

### User Documentation

- Update README.md for new features
- Add usage examples
- Update troubleshooting section if needed

## 🔒 Security Considerations

- Never commit sensitive data (passwords, API keys, etc.)
- Follow security best practices
- Test for common vulnerabilities
- Consider privacy implications of new features

## 🚀 Release Process

1. **Version bump** in platformio.ini
2. **Update CHANGELOG.md** with new features/fixes
3. **Create release tag**:
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```
4. **Create GitHub release** with release notes

## 📞 Getting Help

- **GitHub Issues** - For bugs and feature requests
- **GitHub Discussions** - For questions and general discussion
- **ESP32 Documentation** - For hardware-specific questions

## 🙏 Recognition

Contributors will be recognized in:
- README.md contributors section
- Release notes
- GitHub contributors page

Thank you for contributing to the ESP32 Portal project! 🚀 