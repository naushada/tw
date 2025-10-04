#ifndef __service_adapter_hpp__
#define __service_adapter_hpp__

#include <vector>
#include <unordered_map>
#include <memory>

#include "io_events.hpp"
#include "evt_adapter.hpp"
#include "io_operations.hpp"

extern "C" {
#include <event2/listener.h>
#include <arpa/inet.h>
}

template <typename T>
class server_service {
  struct evconn_deleter {
    void operator()(struct evconnlistener* listener) {evconnlistener_free(listener);}
  };

  public:
    server_service(const evt_base& evt_base_ref, const std::string& host, const std::uint32_t& port) : 
      m_evt_base(evt_base_ref),
      m_evconn_listener_p(nullptr),
      m_io_operation(std::make_unique<T>()),
      m_listener_addr(),
      m_connected_client() {
      struct addrinfo *result;
      auto s = getaddrinfo(host.data(), std::to_string(port).c_str(), nullptr, &result);

      if(!s) {
        m_listener_addr = *((struct sockaddr_in*)(result->ai_addr));
        freeaddrinfo(result);
      }

      m_evconn_listener_p.reset(evconnlistener_new_bind(get_event_base().get(), accept_new_conn_cb, this, 
                         (LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE), 16/*backlog*/,
                         (struct sockaddr*)&m_listener_addr, sizeof(m_listener_addr)));
    }

    auto& connected_client() { return m_connected_client;}
    
    static void accept_new_conn_cb(struct evconnlistener *listener, evutil_socket_t handle,
                                   struct sockaddr *sa, int socklen, void *ctx) {
      auto instance = static_cast<server_service*>(ctx);
      auto events = EV_READ|EV_WRITE;
      auto conn_handler_p = std::make_unique<io_evt>(instance->get_event_base(), handle,
                                                   inet_ntoa(((struct sockaddr_in*)sa)->sin_addr),
                                                   events, std::chrono::seconds(2),
                                                   instance->io_operation());

      bufferevent_setcb(conn_handler_p->get_bufferevt(), read_cb, write_cb, event_cb, ctx);
      bufferevent_enable(conn_handler_p->get_bufferevt(), events);

      auto res = instance->connected_client().insert(
        std::pair<std::int32_t, std::unique_ptr<io_evt>>(
        handle, std::move(conn_handler_p)));

      if(!res.second) {
        std::cout << "Addition of new client is failed for handle:" << handle << std::endl;
      }
    }

    static void read_cb(struct bufferevent *bev, void *ctx) {
      auto instance = static_cast<server_service*>(ctx);
      struct evbuffer *input = bufferevent_get_input(bev);
      evutil_socket_t handle = bufferevent_getfd(bev);
      size_t nbytes = evbuffer_get_length(input);
      std::vector<std::uint8_t> buffer(nbytes);
      evbuffer_remove(input, buffer.data(), nbytes);
      std::cout << "handle:" << handle << " nbytes:"<<nbytes << "\n" << std::string((char *)buffer.data(), nbytes) << std::endl;
      auto conn_handler_it = instance->connected_client().find(handle);
      if(conn_handler_it != instance->connected_client().end()) {
        auto& io_evt = *conn_handler_it->second;
        io_evt.get_io_operation()->handle_read(handle, buffer, nbytes);
      }
    }
    
    std::int32_t tx(evutil_socket_t handle, const char* buffer, const size_t& nbytes) {
      auto conn_handler_it = connected_client().find(handle);
      if(conn_handler_it != connected_client().end()) {
        auto& io_evt = *conn_handler_it->second;
        io_evt.get_io_operation()->tx(buffer, nbytes);
        return nbytes;
      }
      
      return 0;
    }

    static void write_cb(struct bufferevent *bev, void *ctx) {
      // Handle write completion if needed
    }

    static void event_cb(struct bufferevent *bev, short events, void *ctx) {
      auto instance = static_cast<server_service*>(ctx);
      evutil_socket_t handle = bufferevent_getfd(bev);
      auto conn_handler_it = instance->connected_client().find(handle);
      if(conn_handler_it != instance->connected_client().end()) {
        auto& io_evt = *conn_handler_it->second;
        io_evt.get_io_operation()->handle_event(events);
      }

      if(events & BEV_EVENT_ERROR) {
        perror("Error from bufferevent");
      }
      if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        instance->connected_client().erase(handle); 
      }
    }

    const evt_base& get_event_base() const {return m_evt_base;}
    auto io_operation() const {return std::make_unique<T>();}

  private:

    const evt_base& m_evt_base;
    std::unique_ptr<struct evconnlistener, evconn_deleter> m_evconn_listener_p;
    std::unique_ptr<T> m_io_operation;
    struct sockaddr_in m_listener_addr;
    std::unordered_map<std::int32_t, std::unique_ptr<io_evt>> m_connected_client;
};







#endif
