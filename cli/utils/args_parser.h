#pragma once

#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief Simple command-line argument parser
 */
class ArgsParser {
public:
    ArgsParser(int argc, char* argv[]);

    /**
     * @brief Check if a flag exists
     * @param flag Flag name (e.g., "--help")
     * @return true if flag is present
     */
    bool has_flag(const std::string& flag) const;

    /**
     * @brief Get value for a key
     * @param key Key name (e.g., "--input")
     * @param default_value Default value if key not found
     * @return Value string
     */
    std::string get_value(const std::string& key, const std::string& default_value = "") const;

    /**
     * @brief Get integer value for a key
     * @param key Key name
     * @param default_value Default value if key not found
     * @return Integer value
     */
    int get_int(const std::string& key, int default_value = 0) const;

    /**
     * @brief Get double value for a key
     * @param key Key name
     * @param default_value Default value if key not found
     * @return Double value
     */
    double get_double(const std::string& key, double default_value = 0.0) const;

private:
    std::vector<std::string> args_;
    std::unordered_map<std::string, std::string> arg_map_;
};
