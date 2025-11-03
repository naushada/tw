#ifndef __interface_impl_cpp__
#define __interface_impl_cpp__

#include "interface_impl.hpp"
#include "tcp_server.hpp"

int app::handle_event(const short event) {
  std::cerr <<"fn:" <<__func__ << ":"<<__LINE__ << " events:" << event << std::endl;
  get_http2_handler().handle_event(event);
}

int app::handle_read(std::int32_t handle, const std::string& in) {
  // feed to nghttp2 layer to decode request per HTTP2 protocol
  std::string rsp, path;
  get_http2_handler().main(handle, in, rsp, path);

  if(!rsp.empty()) {
     // Extract the length (4 bytes, big-endian/network byte order)
    uint32_t message_length_be;
    std::memcpy(&message_length_be, rsp.data() + 1, 4);
    uint32_t message_length = ntohl(message_length_be); // Convert to host byte order

    if (rsp.length() < 5 + message_length) {
        std::cerr << "Error: Incomplete message data after header." << std::endl;
        return false;
    }

    // Extract the actual protobuf bytes from the string
    std::string protobuf_bytes = rsp.substr(5, message_length);

    auto rpc_name = get_rpc_name(path);
    if(rpc_name.length() > 0 && (!rpc_name.compare("Subscribe"))) {
      gnmi::SubscribeRequest req;
      DebugString(protobuf_bytes, &req);
      // You can now access fields like this:
      if(req.request_case() == gnmi::SubscribeRequest::kSubscribe) {
      }
 
      if(req.request_case() == gnmi::SubscribeRequest::kPoll) {
        std::cerr <<"gnmi::SubscribeRequest::kPoll" << std::endl;
      }

      if(req.has_subscribe()) {
      }
    } else if(rpc_name.length() > 0 && (!rpc_name.compare("Get"))) {
      //
    } else if(rpc_name.length() > 0 && (!rpc_name.compare("PushSubscriptionUpdates"))) {
      gnmi::SubscribeResponse rsp;
      DebugString(protobuf_bytes, &rsp);
      // You can now access fields like this:
      if(rsp.response_case() == gnmi::SubscribeResponse::kUpdate) {
      }
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
