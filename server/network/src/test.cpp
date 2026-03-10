#include <uv.h>
#include <iostream>

int main() {
    uv_loop_t* loop = uv_default_loop();
    std::cout << "Hello, LibUV!" << std::endl;
    return uv_run(loop, UV_RUN_DEFAULT);
}