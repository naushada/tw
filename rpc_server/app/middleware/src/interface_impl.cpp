#ifndef __interface_impl_cpp__
#define __interface_impl_cpp__

#include "interface_impl.hpp"
#include "tcp_server.hpp"


int app::handle_event(const short event) {
  std::cout <<"fn:" <<__func__ << ":"<<__LINE__ << " events:" << event << std::endl;
  get_http2_handler().handle_event(event);
}

int app::handle_read(std::int32_t handle, const std::string& in) {
  // feed to nghttp2 layer to decode request per HTTP2 protocol
  get_http2_handler().process_request_from_peer(handle, in);
  std::cout <<"fn:" << __PRETTY_FUNCTION__ << ":" << __LINE__ << " handle_read is done" << std::endl;
}

void app::handle_new_connection(const int& handle, const std::string& addr) {
  m_handle = handle;
  m_addr = addr;
  get_http2_handler().handle_new_connection(handle, addr);
}

void app::handle_connection_close(int handle) {
  get_http2_handler().handle_connection_close(handle);
}










#endif
