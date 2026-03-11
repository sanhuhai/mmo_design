#pragma once

#include <string>
#include <functional>

#include "register/register_service.h"

#ifdef PROTOBUF_FOUND
#include "register.pb.h"
#endif

namespace register_service {

class MessageHandler {
public:
    MessageHandler();
    ~MessageHandler();
    
    bool initialize();
    void setResponseCallback(std::function<void(const std::string&)> callback);
    void handleMessage(const std::string& message);
    
private:
    void handleSendCodeRequest(const std::string& message);
    void handleRegisterRequest(const std::string& message);
    void handleActivateRequest(const std::string& message);
    
    std::function<void(const std::string&)> response_callback_;
    RegisterService* register_service_;
};

} // namespace register_service