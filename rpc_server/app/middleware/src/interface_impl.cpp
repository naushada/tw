#ifndef __interface_impl_cpp__
#define __interface_impl_cpp__

#include "interface_impl.hpp"
#include "tcp_server.hpp"
#include "/home/mnahmed/tw/rpc_server/build/app/idl/gnmi.pb.h"

int app::handle_event(const short event) {
  std::cout <<"fn:" <<__func__ << ":"<<__LINE__ << " events:" << event << std::endl;
  get_http2_handler().handle_event(event);
}

int app::handle_read(std::int32_t handle, const std::string& in) {
  // feed to nghttp2 layer to decode request per HTTP2 protocol
  get_http2_handler().process_request_from_peer(handle, in);
  std::cout <<"fn:" << __PRETTY_FUNCTION__ << ":" << __LINE__ << " handle_read is done" << std::endl;
  //std::vector<http2_handler::stream_data>::const_iterator strm_data_it = get_http2_handler().get_stream_data();
  const auto& strm_vec = http2_handler().get_stream_data();

  if(!get_requested_rpc().empty()) {
    for(auto const&[path, received_data]: get_requested_rpc()) {
      std::cout <<"fn:" << __PRETTY_FUNCTION__ << ":" << __LINE__ << "RPC:" << path << std::endl;
      gnmi::SubscribeRequest req;
      req.ParseFromArray(received_data.data(), received_data.length());
      std::cout << "Successfully parsed gNMI SubscribeRequest:" << std::endl;
      // You can now access fields like this:
      std::cout << req.subscribe().encoding() << std::endl;
    }
  }
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
