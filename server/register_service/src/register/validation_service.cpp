#include "register/validation_service.h"
#include <regex>

namespace register_service {

bool ValidationService::validateUsername(const std::string& username, std::string& error_msg) {
    if (!isValidUsernameFormat(username)) {
        error_msg = "Username must be 4-20 characters, only letters, numbers and underscores";
        return false;
    }
    
    if (isSensitiveWord(username)) {
        error_msg = "Username contains sensitive words";
        return false;
    }
    
    return true;
}

bool ValidationService::validatePassword(const std::string& password, std::string& error_msg) {
    if (!isValidPasswordFormat(password)) {
        error_msg = "Password must be 8-32 characters, contain letters and numbers";
        return false;
    }
    
    return true;
}

bool ValidationService::validatePhone(const std::string& phone, std::string& error_msg) {
    if (!isValidPhoneFormat(phone)) {
        error_msg = "Invalid phone number format";
        return false;
    }
    
    return true;
}

bool ValidationService::validateEmail(const std::string& email, std::string& error_msg) {
    if (!isValidEmailFormat(email)) {
        error_msg = "Invalid email format";
        return false;
    }
    
    return true;
}

bool ValidationService::isValidUsernameFormat(const std::string& username) {
    if (username.length() < USERNAME_MIN_LENGTH || username.length() > USERNAME_MAX_LENGTH) {
        return false;
    }
    
    std::regex pattern("^[a-zA-Z0-9_]+$");
    return std::regex_match(username, pattern);
}

bool ValidationService::isValidPasswordFormat(const std::string& password) {
    if (password.length() < PASSWORD_MIN_LENGTH || password.length() > PASSWORD_MAX_LENGTH) {
        return false;
    }
    
    bool has_letter = false;
    bool has_digit = false;
    
    for (char c : password) {
        if (isalpha(c)) {
            has_letter = true;
        } else if (isdigit(c)) {
            has_digit = true;
        }
    }
    
    return has_letter && has_digit;
}

bool ValidationService::isValidPhoneFormat(const std::string& phone) {
    std::regex pattern("^1[3-9]\\d{9}$");
    return std::regex_match(phone, pattern);
}

bool ValidationService::isValidEmailFormat(const std::string& email) {
    std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return std::regex_match(email, pattern);
}

bool ValidationService::isSensitiveWord(const std::string& text) {
    // 简化实现，实际项目中需要使用敏感词库
    std::vector<std::string> sensitive_words = {"admin", "root", "test"};
    for (const auto& word : sensitive_words) {
        if (text.find(word) != std::string::npos) {
            return true;
        }
    }
    return false;
}

} // namespace register_service