#ifndef __http2_rsp_builder_cpp__
#define __http2_rsp_builder_cpp__

#include "http2_handler.hpp"

#include "gnmi.pb.h"
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <arpa/inet.h>

#include <sstream>

std::string http2_handler::build_response(const std::string& path, const std::string& req) {
  auto rpc_name = get_rpc_name(path);
  if(!req.empty()) {
    // Extract the length (4 bytes, big-endian/network byte order)
    uint32_t message_length_be;
    std::memcpy(&message_length_be, req.data() + 1, 4);
    uint32_t message_length = ntohl(message_length_be); // Convert to host byte order

    if (req.length() < 5 + message_length) {
        std::cerr << "Error: Incomplete message data after header." << std::endl;
        return std::string();
    }

    std::string protobuf_bytes = req.substr(5, message_length);
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
    return(build_empty_response());
  }

  return(std::string());
}

std::string http2_handler::build_isAlive_response() {
}

std::string http2_handler::build_notification_response() {
}

std::string http2_handler::build_empty_response() {
  gnmi::Poll empty_rsp;
  char is_compressed = 0;
  std::uint32_t length = 0;
  std::stringstream ss;
  std::string serialized_output;

  if(!empty_rsp.SerializeToString(&serialized_output)) {
    std::cout << "Failed serialized gnmi::Poll message." << std::endl;

    // You can now write 'serialized_output' to a file, send it over a non-gRPC connection, etc.
    return std::string();
  }

  std::cout << "Serialized size (bytes): " << serialized_output.length() << std::endl;
  length = serialized_output.length();
  
  ss.write (reinterpret_cast <const char *>(&is_compressed), sizeof(is_compressed));
  if(length > 0) {
    length = htonl(length);;
  }
  ss.write (reinterpret_cast <const char *>(&length), sizeof(length));
  ss << serialized_output;
  return ss.str();    
}








#endif
