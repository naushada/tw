#ifndef __service_adapter_hpp__
#define __service_adapter_hpp__

#include <vector>
#include <unordered_map>
#include <memory>

#include "io_adapter.hpp"
#include "evt_adapter.hpp"
#include "io_operations.hpp"

extern "C" {
#include <event2/listener.h>
#include <arpa/inet.h>
}

template <typename T = rw_operation>
class server_service {
  public:
    server_service(std::shared_ptr<evt_base> evt_base_p, const std::string& host, const std::uint32_t& port) : 
      m_evt_base_p(evt_base_p),
      m_listener_p(nullptr, evconnlistener_free),
      m_io_operation_p(std::make_shared<T>()) {
      struct addrinfo *result;
      auto s = getaddrinfo(host.data(), std::to_string(port).c_str(), nullptr, &result);

      if(!s) {
        m_listener_addr = *((struct sockaddr_in*)(result->ai_addr));
        freeaddrinfo(result);
      }

      m_listener_p.reset(evconnlistener_new_bind(m_evt_base_p->get(), accept_new_conn_cb, this, 
                         (LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE), -1,
                         (struct sockaddr*)&m_listener_addr, sizeof(m_listener_addr)));
    }

    auto& connected_client() { return m_connected_client;}
    
    static void accept_new_conn_cb(struct evconnlistener *listener, evutil_socket_t handle,
                               struct sockaddr *sa, int socklen, void *ctx) {
      auto instance = static_cast<server_service*>(ctx);
      instance->connected_client().insert(std::pair<std::int32_t, io_evt>(handle, io_evt(instance->evt_base_p(), handle,
                                inet_ntoa(((struct sockaddr_in*)sa)->sin_addr),
                                EV_READ|EV_WRITE, std::chrono::seconds(2), instance->io_operation())));
    }

    std::shared_ptr<evt_base> evt_base_p() const {return m_evt_base_p;}
    std::shared_ptr<rw_operation> io_operation() const {return m_io_operation_p;}
  private:

    std::shared_ptr<evt_base> m_evt_base_p;
    std::unique_ptr<struct evconnlistener, decltype(&evconnlistener_free)> m_listener_p;
    struct sockaddr_in m_listener_addr;
    std::unordered_map<std::int32_t, io_evt> m_connected_client;
    std::shared_ptr<rw_operation> m_io_operation_p;
};







#endif
