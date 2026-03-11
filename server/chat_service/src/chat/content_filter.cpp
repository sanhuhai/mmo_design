#include "chat/content_filter.h"
#include <iostream>

namespace chat_service {

ContentFilter::ContentFilter() {
    loadSensitiveWords();
}

ContentFilter::~ContentFilter() {
}

bool ContentFilter::filterContent(std::string& content) {
    bool filtered = false;
    for (const auto& word : sensitive_words_) {
        size_t pos = content.find(word);
        while (pos != std::string::npos) {
            content.replace(pos, word.length(), std::string(word.length(), '*'));
            filtered = true;
            pos = content.find(word, pos + word.length());
        }
    }
    return filtered;
}

bool ContentFilter::isSensitive(const std::string& content) {
    for (const auto& word : sensitive_words_) {
        if (content.find(word) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void ContentFilter::addSensitiveWord(const std::string& word) {
    sensitive_words_.insert(word);
}

void ContentFilter::removeSensitiveWord(const std::string& word) {
    sensitive_words_.erase(word);
}

void ContentFilter::loadSensitiveWords() {
    // 这里应该从配置文件或数据库加载敏感词
    // 暂时硬编码一些示例敏感词
    sensitive_words_.insert("敏感词1");
    sensitive_words_.insert("敏感词2");
    sensitive_words_.insert("敏感词3");
}

} // namespace chat_service
