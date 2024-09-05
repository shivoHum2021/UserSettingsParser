# UserSettingsParser

The UserSettingsParser class is a thread-safe, singleton utility for managing user settings from a configuration file. The settings are stored as key-value pairs and can be retrieved or updated with automatic type conversion support for int, double, and bool values.

The class ensures that a settings file is created automatically if it doesn't exist when loading settings for the first time.
Features

- Singleton Design: Ensures only one instance of UserSettingsParser is used throughout the application.
- Thread Safety: Internal operations are protected by a mutex to avoid race conditions.
- File Handling: Automatically creates the settings file if it doesn't exist when loading settings.
- Template-Based Type Conversion: Supports retrieving and setting values of various types such as int, double, and bool.
- File Saving: Allows saving the current settings to a specified file or the last loaded file.

## Getting Started

### Prerequisites
- Ensure your development environment supports C++17 or higher to use modern C++ features like std::filesystem.

### Installation
- Simply include the UserSettingsParser.hpp file in your project:

```cpp
#include "UserSettingsParser.hpp"
```
### Usage
- Load Settings from a File

Before accessing or modifying settings, load them from a file. The file is automatically created if it doesn’t exist.
```cpp
UserSettingsParser& parser = UserSettingsParser::getInstance();
parser.loadSettings("config/settings.conf");
```
- Retrieve Settings as String

You can retrieve settings using the getValue method, which returns the value as a string.
```cpp
std::string username = parser.getValue("username");
```
- Retrieve Settings with Type Conversion

Use getValueAs<T>() to retrieve values with automatic type conversion. The supported types are int, double, and bool.
```cpp
int maxConnections = parser.getValueAs<int>("max_connections");
double timeout = parser.getValueAs<double>("timeout");
bool isActive = parser.getValueAs<bool>("active");
```
- Set Settings

You can set settings directly as a string using setValue.
```cpp
parser.setValue("theme", "dark");
```
You can also use type conversion when setting values with setValueAs<T>().
```cpp
parser.setValueAs<int>("max_connections", 100);
parser.setValueAs<bool>("notifications_enabled", true);
```
- Save Settings
After modifying the settings, you can save them back to the file.
```cpp
parser.saveSettings();  // Saves to the last loaded file
```
You can also specify a different file to save the settings to:
```cpp
parser.saveSettingsAs("config/backup_settings.conf");
```

### Example

```cpp
#include "UserSettingsParser.hpp"

int main() {
    UserSettingsParser& parser = UserSettingsParser::getInstance();

    // Load settings, will create the file if it doesn't exist
    parser.loadSettings("settings.conf");

    // Get some values
    std::string username = parser.getValue("username");
    int maxConnections = parser.getValueAs<int>("max_connections");

    // Set some values
    parser.setValue("theme", "light");
    parser.setValueAs<int>("max_connections", 200);

    // Save the updated settings
    parser.saveSettings();

    return 0;
}
```
### Exception Handling

- If a file cannot be opened or created, a std::runtime_error is thrown.
- If a requested setting is not found, a std::runtime_error is thrown.

Example of catching exceptions:

```cpp
try {
    parser.loadSettings("settings.conf");
} catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

### Thread Safety

All public methods that modify or access settings are thread-safe, ensuring that multiple threads can safely load, modify, and save settings.

### License

This project is licensed under the MIT License - see the LICENSE file for details.
