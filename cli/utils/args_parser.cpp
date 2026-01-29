#include "args_parser.h"
#include <algorithm>

ArgsParser::ArgsParser(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {
        args_.emplace_back(argv[i]);
    }

    // Parse key-value pairs
    for (size_t i = 0; i < args_.size(); ++i) {
        const auto& arg = args_[i];
        
        // Check if this is a key (starts with - or --)
        if (!arg.empty() && arg[0] == '-') {
            // Check if next argument is a value (doesn't start with -)
            if (i + 1 < args_.size() && !args_[i + 1].empty() && args_[i + 1][0] != '-') {
                arg_map_[arg] = args_[i + 1];
                ++i; // Skip the value in next iteration
            } else {
                // It's a flag without value
                arg_map_[arg] = "";
            }
        }
    }
}

bool ArgsParser::has_flag(const std::string& flag) const {
    return arg_map_.find(flag) != arg_map_.end();
}

std::string ArgsParser::get_value(const std::string& key, const std::string& default_value) const {
    auto it = arg_map_.find(key);
    if (it != arg_map_.end()) {
        return it->second;
    }
    return default_value;
}

int ArgsParser::get_int(const std::string& key, int default_value) const {
    auto value_str = get_value(key, "");
    if (value_str.empty()) {
        return default_value;
    }
    
    try {
        return std::stoi(value_str);
    } catch (...) {
        return default_value;
    }
}

double ArgsParser::get_double(const std::string& key, double default_value) const {
    auto value_str = get_value(key, "");
    if (value_str.empty()) {
        return default_value;
    }
    
    try {
        return std::stod(value_str);
    } catch (...) {
        return default_value;
    }
}
