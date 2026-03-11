#include "network/network.h"
#include "network/server.h"
#include "network/message.h"
#include <iostream>

using namespace network;

void onMessageReceived(const std::string& message) {
    std::cout << "Received message: " << message << std::endl;
}

void onConnection(uv_tcp_t* conn) {
    std::cout << "New connection" << std::endl;
}

void onError(ErrorCode error) {
    std::cerr << "Error: " << static_cast<int>(error) << std::endl;
}

int main() {
    // Initialize network library
    if (!init()) {
        std::cerr << "Failed to initialize network library" << std::endl;
        return 1;
    }
    
    // Create server
    Server server;
    
    // Set callbacks
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessageReceived);
    server.setErrorCallback(onError);
    
    // Start server
    if (!server.start("0.0.0.0", 8080)) {
        std::cerr << "Failed to start server" << std::endl;
        cleanup();
        return 1;
    }
    
    std::cout << "Server started on port 8080. Press Enter to exit." << std::endl;
    
    // Wait for user input
    std::string input;
    std::getline(std::cin, input);
    
    // Stop server
    server.stop();
    
    // Cleanup
    cleanup();
    return 0;
}
