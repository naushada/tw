#ifndef __server_app_cpp__
#define __server_app_cpp__

#include "server_app.hpp"
template <typename T>
server_app<T>::server_app(const std::string& host, const std::uint32_t& port) : 
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
void server_app<T>::accept_new_conn_cb(struct evconnlistener *listener, evutil_socket_t handle,
                                    struct sockaddr *sa, int socklen, void *ctx) {
  auto instance = static_cast<server_app*>(ctx);
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
void server_app<T>::read_cb(struct bufferevent *bev, void *ctx) {
  auto instance = static_cast<server_app*>(ctx);
  struct evbuffer *input = bufferevent_get_input(bev);
  evutil_socket_t handle = bufferevent_getfd(bev);
  size_t nbytes = evbuffer_get_length(input);
  // get the contiguous block of data in oneshot
  std::string data_str(reinterpret_cast<char *>(evbuffer_pullup(input, -1)), nbytes);
  std::cout << "handle:" << handle << " nbytes:"<<nbytes << "\n" << data_str << std::endl;
  auto conn_handler_it = instance->connected_client().find(handle);
  if(conn_handler_it != instance->connected_client().end()) {
    // dispatch receive data to app_interface to process it.
    conn_handler_it->second->get_app_interface()->handle_read(handle, data_str);
  }
}
    
template <typename T>
std::int32_t server_app<T>::tx(evutil_socket_t handle, const char* buffer, const size_t& nbytes) {
  auto conn_handler_it = connected_client().find(handle);
  if(conn_handler_it != connected_client().end()) {
    conn_handler_it->second->get_io_operation()->tx(buffer, nbytes);
    return nbytes;
  }
      
  return -1;
}

template <typename T>
void server_app<T>::event_cb(struct bufferevent *bev, short events, void *ctx) {
  auto instance = static_cast<server_app*>(ctx);
  evutil_socket_t handle = bufferevent_getfd(bev);
  auto conn_handler_it = instance->connected_client().find(handle);
  if(conn_handler_it != instance->connected_client().end()) {
    conn_handler_it->second->get_app_interface()->handle_event(events);
  }

  if(events & BEV_EVENT_ERROR) {
    std::cout <<"fn:" <<__func__ << " Error for handle:" << handle << " from bufferevent" << std::endl;
  }

  if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    instance->connected_client().erase(handle); 
  }
}

template <typename T>
void server_app<T>::write_cb(struct bufferevent *bev, void *ctx) {
  // Handle write completion if needed
}

#endif
