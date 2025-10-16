#ifndef __service_app_hpp__
#define __service_app_hpp__

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "evt_adapter.hpp"
#include "app_interface.hpp"

extern "C" {
  #include <event2/listener.h>
  #include <arpa/inet.h>
}

template <typename T>
class server_app {
  struct custom_deleter {
    void operator()(struct evconnlistener* listener) {evconnlistener_free(listener);}
  };

  public:
    server_app(const std::string& host, const std::uint32_t& port);
    auto& connected_client() { return m_connected_client;}
   
    // This is a callback invoked by libevent when a client is connected 
    static void accept_new_conn_cb(struct evconnlistener *listener, evutil_socket_t handle,
                                   struct sockaddr *sa, int socklen, void *ctx);
    static void read_cb(struct bufferevent *bev, void *ctx);
    std::int32_t tx(evutil_socket_t handle, const char* buffer, const size_t& nbytes);
    static void write_cb(struct bufferevent *bev, void *ctx);
    static void event_cb(struct bufferevent *bev, short events, void *ctx);
    const evt_base& get_event_base() const {return *m_evt_base_p;}
    auto create_app_interface() const {return std::make_unique<T>();}

  private:
    std::unique_ptr<evt_base> m_evt_base_p;
    std::unique_ptr<struct evconnlistener, custom_deleter> m_evconn_listener_p;
    std::unique_ptr<T> m_app_interface;
    struct sockaddr_in m_listener_addr;
    std::unordered_map<std::int32_t, std::unique_ptr<evt_io>> m_connected_client;
};







#endif
