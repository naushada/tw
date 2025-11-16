#ifndef __http2_rsp_builder_cpp__
#define __http2_rsp_builder_cpp__

#include "http2_handler.hpp"

#include "gnmi.pb.h"
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <arpa/inet.h>

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
  }
}

std::string http2_handler::build_isAlive_response() {
}

std::string http2_handler::build_notification_response() {
}










#endif
