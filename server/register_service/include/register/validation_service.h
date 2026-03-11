#pragma once

#include <string>

namespace register_service {

class ValidationService {
public:
    bool validateUsername(const std::string& username, std::string& error_msg);
    bool validatePassword(const std::string& password, std::string& error_msg);
    bool validatePhone(const std::string& phone, std::string& error_msg);
    bool validateEmail(const std::string& email, std::string& error_msg);
    
private:
    bool isValidUsernameFormat(const std::string& username);
    bool isValidPasswordFormat(const std::string& password);
    bool isValidPhoneFormat(const std::string& phone);
    bool isValidEmailFormat(const std::string& email);
    bool isSensitiveWord(const std::string& text);
    
    static const int USERNAME_MIN_LENGTH = 4;
    static const int USERNAME_MAX_LENGTH = 20;
    static const int PASSWORD_MIN_LENGTH = 8;
    static const int PASSWORD_MAX_LENGTH = 32;
};

} // namespace register_service