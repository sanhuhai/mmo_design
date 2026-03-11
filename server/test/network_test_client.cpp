#include "network/network.h"
#include "network/connection.h"
#include "test.pb.h"
#include <uv.h>
#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <chrono>

using namespace network;
using namespace test;

int main() {
    uv_loop_t* loop = uv_default_loop();
    uv_tcp_t client;
    uv_tcp_init(loop, &client);
    
    struct sockaddr_in dest;
    uv_ip4_addr("127.0.0.1", 8080, &dest);
    
    // 连接到服务器
    int r = uv_tcp_connect(
        new uv_connect_t,
        &client,
        (const struct sockaddr*)&dest,
        [](uv_connect_t* req, int status) {
            if (status < 0) {
                std::cerr << "Connection error: " << uv_strerror(status) << std::endl;
                return;
            }
            
            std::cout << "Connected to server" << std::endl;
            
            // 构建 protobuf 消息
            TestMessage test_message;
            test_message.set_id(1);
            test_message.set_content("开始连上服务器了你的手机酷狗hi输入的数据干护额时任韩国i哦二十万股");
            test_message.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count() / 1000000);
            
            // 序列化消息
            std::string message_str;
            test_message.SerializeToString(&message_str);
            
            // 添加长度前缀
            uint32_t length = message_str.size();
            std::string encoded_message;
            encoded_message.append(reinterpret_cast<const char*>(&length), sizeof(length));
            encoded_message.append(message_str);
            
            // 发送消息
            uv_buf_t buf = uv_buf_init(const_cast<char*>(encoded_message.data()), encoded_message.size());
            uv_write_t* write_req = new uv_write_t;
            uv_write(write_req, (uv_stream_t*)req->handle, &buf, 1, [](uv_write_t* req, int status) {
                if (status < 0) {
                    std::cerr << "Write error: " << uv_strerror(status) << std::endl;
                } else {
                    std::cout << "Protobuf message sent" << std::endl;
                }
                delete req;
            });
            
            // 设置读取回调
            uv_read_start((uv_stream_t*)req->handle, [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
                buf->base = new char[suggested_size];
                buf->len = suggested_size;
            }, [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
                if (nread < 0) {
                    if (nread != UV_EOF) {
                        std::cerr << "Read error: " << uv_strerror(nread) << std::endl;
                    }
                    uv_close((uv_handle_t*)stream, nullptr);
                } else if (nread > 0) {
                    // 解析长度前缀
                    if (nread < sizeof(uint32_t)) {
                        std::cout << "Received incomplete message" << std::endl;
                    } else {
                        uint32_t length = *reinterpret_cast<const uint32_t*>(buf->base);
                        if (nread < sizeof(uint32_t) + length) {
                            std::cout << "Received incomplete message" << std::endl;
                        } else {
                            // 提取消息内容
                            std::string message(buf->base + sizeof(uint32_t), length);
                            
                            // 解析 protobuf 响应
                            TestResponse response;
                            if (response.ParseFromString(message)) {
                                std::cout << "Received protobuf response: " << std::endl;
                                std::cout << "  Code: " << response.code() << std::endl;
                                std::cout << "  Message: " << response.message() << std::endl;
                                std::cout << "  Data ID: " << response.data().id() << std::endl;
                                std::cout << "  Data Content: " << response.data().content() << std::endl;
                            } else {
                                std::cout << "Failed to parse protobuf response" << std::endl;
                            }
                        }
                    }
                }
                delete[] buf->base;
            });
            
            delete req;
        }
    );
    
    if (r < 0) {
        std::cerr << "Connect error: " << uv_strerror(r) << std::endl;
        return 1;
    }
    
    // 运行事件循环
    uv_run(loop, UV_RUN_DEFAULT);
    
    return 0;
}
