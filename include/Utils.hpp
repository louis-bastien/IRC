#pragma once
#include <sstream>
#include <string>

class Utils {
    public:
        template<typename T>
        static std::string toString(T value) {
            std::ostringstream oss;
            oss << value;
            std::string newString(oss.str());
            return newString;
        }
};