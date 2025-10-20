#ifndef __app_interface_cpp__
#define __app_interface_cpp__

#include "app_interface.hpp"
#include "tcp_server.hpp"
//#include "http2_handler.hpp"


int app::handle_event(const short event) {
}

int app::handle_read(std::int32_t handle, const std::string& in) {
}

void app::handle_new_connection(const int& handle, const std::string& addr) {
}

void app::handle_connection_close(int handle) {
}










#endif
