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

        static std::vector<std::string> split(const std::string& input, char delimiter) {
            std::vector<std::string> result;
            std::string token;
            std::istringstream tokenStream(input);

            while (std::getline(tokenStream, token, delimiter)) {
                if (!token.empty()) {
                    result.push_back(token);
                }
            }
            return result;
        }
};