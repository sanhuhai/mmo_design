#pragma once

#include <string>
#include <unordered_set>

namespace chat_service {

class ContentFilter {
public:
    ContentFilter();
    ~ContentFilter();
    
    bool filterContent(std::string& content);
    bool isSensitive(const std::string& content);
    void addSensitiveWord(const std::string& word);
    void removeSensitiveWord(const std::string& word);
    
private:
    std::unordered_set<std::string> sensitive_words_;
    
    void loadSensitiveWords();
};

} // namespace chat_service
