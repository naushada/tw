#ifndef __interface_impl_cpp__
#define __interface_impl_cpp__

#include "interface_impl.hpp"
#include "tcp_server.hpp"
#include "gnmi.pb.h"

int app::handle_event(const short event) {
  std::cout <<"fn:" <<__func__ << ":"<<__LINE__ << " events:" << event << std::endl;
  get_http2_handler().handle_event(event);
}

int app::handle_read(std::int32_t handle, const std::string& in) {
  // feed to nghttp2 layer to decode request per HTTP2 protocol
  std::string rsp, path;
  get_http2_handler().main(handle, in, rsp, path);
  std::cout <<"fn:" << __PRETTY_FUNCTION__ << ":" << __LINE__ << " handle_read is done" << std::endl;
  //std::vector<http2_handler::stream_data>::const_iterator strm_data_it = get_http2_handler().get_stream_data();

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

    std::cout <<"fn:" << __PRETTY_FUNCTION__ << ":" << __LINE__ << "RPC:" << path << std::endl;
    gnmi::SubscribeRequest req;
    req.ParseFromString(protobuf_bytes);
    std::cout << "Successfully parsed gNMI SubscribeRequest:" << std::endl;
    // You can now access fields like this:
    if(req.request_case() == gnmi::SubscribeRequest::kSubscribe) {
      std::cout <<"gnmi::SubscribeRequest::kSubscribe" << std::endl;
    }
 
    
    if(req.request_case() == gnmi::SubscribeRequest::kPoll) {
      std::cout <<"gnmi::SubscribeRequest::kPoll" << std::endl;
    }

    if(req.has_subscribe()) {
      const auto& sub_list = req.subscribe();
      for (const auto& subscription : sub_list.subscription()) {
        // Accessing nested fields, e.g., path elements
        std::cout << "  Path elements: ";
        for (const auto& elem : subscription.path().elem()) {
          std::cout << "/" << elem.name();
          // Handle keys if necessary
        }
        std::cout << std::endl;
        std::cout << "  Mode: " << gnmi::SubscriptionMode_Name(subscription.mode()) << std::endl;
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
