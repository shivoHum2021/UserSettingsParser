#ifndef USER_SETTINGS_PARSER_HPP
#define USER_SETTINGS_PARSER_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>   // Added for file existence checks
#include <algorithm>    // For trim and transform
#include <cctype>       // For isspace/tolower

/**
 * @class UserSettingsParser
 * @brief Singleton class responsible for loading, retrieving, and saving user settings from a file.
 *
 * The UserSettingsParser class provides an interface to load settings from a file, retrieve values by key,
 * set new values, and save the settings back to a file. This class supports generic type conversion using
 * template functions for retrieving and setting values in different types (e.g., int, double, bool).
 */
class UserSettingsParser {
public:
    /**
     * @brief Retrieves the singleton instance of the UserSettingsParser.
     * @return A reference to the singleton UserSettingsParser instance.
     */
    static UserSettingsParser& getInstance() {
        static UserSettingsParser instance;
        return instance;
    }

    /**
     * @brief Creates the file if it doesn't exist.
     *
     * This method checks whether the specified file exists and creates an empty file
     * if it is not present.
     *
     * @param filename The name of the file to check and possibly create.
     * @throws std::runtime_error If the file cannot be created.
     */
    void createFileIfNotExists(const std::string& filename) {
        if (!std::filesystem::exists(filename)) {
            std::ofstream outfile(filename);
            if (!outfile) {
                throw std::runtime_error("Unable to create settings file: " + filename);
            }
            outfile.close();
        }
    }

    /**
     * @brief Loads the settings from a specified file.
     *
     * This function reads key-value pairs from the file, where each line should be formatted as 'key=value'.
     * The file is auto-created if missing. Existing in-memory settings are cleared before loading.
     *
     * @param filename The name of the file to load the settings from.
     * @throws std::runtime_error If the file cannot be opened.
     */
    void loadSettings(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);

        // Ensure file exists (auto-create if missing)
        createFileIfNotExists(filename);

        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open settings file: " + filename);
        }

        // Clear previous settings to avoid stale keys
        settings_.clear();

        std::string line;
        while (std::getline(file, line)) {
            trimInPlace(line);
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue; // skip blanks and comments
            }

            auto pos = line.find('=');
            if (pos == std::string::npos) {
                continue; // skip malformed lines
            }

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            trimInPlace(key);
            trimInPlace(value);

            if (!key.empty()) {
                settings_[key] = value;
            }
        }
        file.close();
        currentFilename_ = filename;
    }

    /**
     * @brief Retrieves the value associated with the specified key.
     *
     * @param key The key for which the value should be retrieved.
     * @return The value as a string.
     * @throws std::runtime_error If the key is not found in the settings.
     */
    std::string getValue(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = settings_.find(key);
        if (it != settings_.end()) {
            return it->second;
        } else {
            throw std::runtime_error("Setting not found: " + key);
        }
    }

    /**
     * @brief Retrieves the value associated with the specified key and converts it to the requested type.
     *
     * Supported types include int, double, and bool (with true/false or 1/0).
     *
     * @tparam T The type to which the value should be converted.
     * @param key The key for which the value should be retrieved.
     * @return The value converted to the specified type.
     * @throws std::runtime_error If the key is not found or if conversion fails.
     */
    template<typename T>
    T getValueAs(const std::string& key);

    /**
     * @brief Sets a value for the specified key.
     *
     * @param key The key to set the value for.
     * @param value The value to be associated with the key.
     */
    void setValue(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        settings_[key] = value;
    }

    /**
     * @brief Sets a value for the specified key and converts it from the specified type.
     *
     * Supported types include int, double, and bool.
     *
     * @tparam T The type of the value to be set.
     * @param key The key to set the value for.
     * @param value The value to be associated with the key, which will be converted to a string.
     */
    template<typename T>
    void setValueAs(const std::string& key, const T& value);

    /**
     * @brief Saves the current settings to the last loaded file.
     *
     * @throws std::runtime_error If no file was previously loaded.
     */
    void saveSettings() {
        // Copy filename under lock, then release before save to avoid self-deadlock
        std::string filenameCopy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (currentFilename_.empty()) {
                throw std::runtime_error("No filename specified for saving settings.");
            }
            filenameCopy = currentFilename_;
        }
        saveSettingsAs(filenameCopy); // locks internally
    }

    /**
     * @brief Saves the current settings to a specified file.
     *
     * @param filename The name of the file to save the settings to.
     * @throws std::runtime_error If the file cannot be opened.
     */
    void saveSettingsAs(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open settings file: " + filename);
        }

        for (const auto& setting : settings_) {
            file << setting.first << "=" << setting.second << "\n";
        }
        file.close();

        // Adopt the new path as the current file
        currentFilename_ = filename;
    }

private:
    // Whitespace trim helpers
    static inline void trimInPlace(std::string& s) {
        auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    }

    static inline void toLowerInPlace(std::string& s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    }

    UserSettingsParser() = default;
    ~UserSettingsParser() = default;
    UserSettingsParser(const UserSettingsParser&) = delete;
    UserSettingsParser& operator=(const UserSettingsParser&) = delete;

    std::unordered_map<std::string, std::string> settings_;
    std::mutex mutex_;
    std::string currentFilename_;
};

template<>
inline int UserSettingsParser::getValueAs<int>(const std::string& key) {
    return std::stoi(getValue(key));
}

template<>
inline double UserSettingsParser::getValueAs<double>(const std::string& key) {
    return std::stod(getValue(key));
}

template<>
inline bool UserSettingsParser::getValueAs<bool>(const std::string& key) {
    std::string value = getValue(key);
    trimInPlace(value);
    toLowerInPlace(value);
    return value == "true" || value == "1";
}

template<>
inline void UserSettingsParser::setValueAs<int>(const std::string& key, const int& value) {
    setValue(key, std::to_string(value));
}

template<>
inline void UserSettingsParser::setValueAs<double>(const std::string& key, const double& value) {
    setValue(key, std::to_string(value));
}

template<>
inline void UserSettingsParser::setValueAs<bool>(const std::string& key, const bool& value) {
    setValue(key, value ? "true" : "false");
}

#endif // USER_SETTINGS_PARSER_HPP
