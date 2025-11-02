#ifndef __tcp_server_hpp__
#define __tcp_server_hpp__

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "evt_adapter.hpp"
#include "interface.hpp"

extern "C" {
  #include <event2/listener.h>
  #include <arpa/inet.h>
}

template <typename T>
class tcp_server {
  struct custom_deleter {
    void operator()(struct evconnlistener* listener) {evconnlistener_free(listener);}
  };

  public:
    tcp_server(const std::string& host, const std::uint32_t& port);
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


template <typename T>
tcp_server<T>::tcp_server(const std::string& host, const std::uint32_t& port) : 
  m_evt_base_p(std::make_unique<evt_base>()),
  m_evconn_listener_p(nullptr),
  m_app_interface(nullptr),
  m_listener_addr(),
  m_connected_client() {
    struct addrinfo *result;
    auto s = getaddrinfo(host.data(), std::to_string(port).c_str(), nullptr, &result);
    if(!s) {
      m_listener_addr = *((struct sockaddr_in*)(result->ai_addr));
      freeaddrinfo(result);
    }

    // TCP server listener
    m_evconn_listener_p.reset(evconnlistener_new_bind(get_event_base().get(),
      accept_new_conn_cb,
      this/*This is for *ctx*/,
      (LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE),
      16/*backlog*/,
      (struct sockaddr*)&m_listener_addr,
      sizeof(m_listener_addr)));
}


template <typename T>
void tcp_server<T>::accept_new_conn_cb(struct evconnlistener *listener, evutil_socket_t handle,
                                    struct sockaddr *sa, int socklen, void *ctx) {
  auto instance = static_cast<tcp_server*>(ctx);
  auto events = EV_READ|EV_WRITE;
                                       
  auto conn_handler_p = std::make_unique<evt_io>(instance->get_event_base(),
                                                 handle,
                                                 inet_ntoa(((struct sockaddr_in*)sa)->sin_addr),
                                                 events,
                                                 std::chrono::seconds(2),
                                                 instance->create_app_interface()/*per connection*/);

  bufferevent_setcb(conn_handler_p->get_bufferevt(), read_cb, write_cb, event_cb, ctx);
  bufferevent_enable(conn_handler_p->get_bufferevt(), events);

  auto res = instance->connected_client().insert(std::pair<std::int32_t, std::unique_ptr<evt_io>>(
                                                 handle, std::move(conn_handler_p)));
  if(!res.second) {
    std::cout << "Addition of new client is failed for handle:" << handle << std::endl;
  }
}

template <typename T>
void tcp_server<T>::read_cb(struct bufferevent *bev, void *ctx) {
  auto instance = static_cast<tcp_server*>(ctx);
  struct evbuffer *input = bufferevent_get_input(bev);
  evutil_socket_t handle = bufferevent_getfd(bev);
  size_t nbytes = evbuffer_get_length(input);
  // get the contiguous block of data in oneshot
  std::string data_str(reinterpret_cast<char *>(evbuffer_pullup(input, nbytes)), nbytes);
  //std::cout << "fn:" << __PRETTY_FUNCTION__  << " handle:" << handle << " nbytes:" << nbytes << "\ndata:" << data_str << std::endl;
  auto conn_handler_it = instance->connected_client().find(handle);
  if(conn_handler_it != instance->connected_client().end()) {
    // dispatch receive data to app_interface to process it.
    //std::cout<<"fn:" << __PRETTY_FUNCTION__  << ":" << __LINE__ << " invoking handle_read" << std::endl;
    conn_handler_it->second->get_app_interface()->handle_read(handle, data_str);
  }

  evbuffer_drain(input, nbytes);
}
    
template <typename T>
std::int32_t tcp_server<T>::tx(evutil_socket_t handle, const char* buffer, const size_t& nbytes) {
  auto conn_handler_it = connected_client().find(handle);
  if(conn_handler_it != connected_client().end()) {
    conn_handler_it->second->get_io_operation()->tx(buffer, nbytes);
    return nbytes;
  }
      
  return -1;
}

template <typename T>
void tcp_server<T>::event_cb(struct bufferevent *bev, short events, void *ctx) {
  auto instance = static_cast<tcp_server*>(ctx);
  evutil_socket_t handle = bufferevent_getfd(bev);
  auto conn_handler_it = instance->connected_client().find(handle);
  if(conn_handler_it != instance->connected_client().end()) {
    conn_handler_it->second->get_app_interface()->handle_event(events);
  }

  if(events & BEV_EVENT_ERROR) {
    std::cout <<"fn:" <<__func__ << " Error for handle:" << handle << " from bufferevent" << std::endl;
  }

  if (events & BEV_EVENT_EOF) {
    // closing the connection
    conn_handler_it->second->get_app_interface()->handle_connection_close(handle);
    //std::cout << "fn:" << __func__ << ":"<<__LINE__ << " closing the connection handle:" << handle << std::endl; 
    instance->connected_client().erase(handle); 
  }
}

template <typename T>
void tcp_server<T>::write_cb(struct bufferevent *bev, void *ctx) {
  // Handle write completion if needed
  std::cout <<"fn:" << __func__ << ":" << __LINE__ << " write_cb not implemented" << std::endl;
}




#endif
