#ifndef __io_adapter_hpp__
#define __io_adapter_hpp__

#include <vector>
#include <memory>

extern "C" {
#include <event2/buffer.h>
#include <event2/bufferevent.h>
}

#include "evt_adapter.hpp"
#include "io_adapter.hpp"
#include "io_operations.hpp"

class io_evt {
  public:
    io_evt(std::shared_ptr<evt_base> evt_base_p, evutil_socket_t handle, const std::string& peer_host, const std::int32_t& event, const std::chrono::seconds& secs, std::shared_ptr<io_operation> io_operation_p = std::make_shared<rw_operation>()) :
      m_buffer_evt_p(bufferevent_socket_new(evt_base_p->get(), handle, BEV_OPT_CLOSE_ON_FREE), bufferevent_free),
      m_evt_base_p(evt_base_p),
      m_from_host(peer_host),
      m_io_operation_p(io_operation_p) {
      //bufferevent_setcb(m_buffer_evt_p.get(), read_cb, write_cb, event_cb, this);
      //bufferevent_enable(m_buffer_evt_p.get(), event);
    }
/*
    static void read_cb(struct bufferevent *bev, void *ctx) {
      auto instance = static_cast<io_evt*>(ctx);
      struct evbuffer *input = bufferevent_get_input(bev);
      evutil_socket_t handle = bufferevent_getfd(bev);
      size_t nbytes = evbuffer_get_length(input);
      std::vector<std::uint8_t> buffer(nbytes);
      evbuffer_remove(input, buffer.data(), nbytes);
      //std::cout << "handle:" << handle << " nbytes:"<<nbytes << "\n" << std::string((char *)buffer.data(), nbytes) << std::endl;
      if(instance->get_io_operation()) {
        instance->get_io_operation()->handle_read(handle, buffer, nbytes);
      }
    }
*/    
    std::int32_t tx(const char* buffer, const size_t& len) {
      bufferevent_write(m_buffer_evt_p.get(), (void *)buffer, len); 
    }

#if 0
    static void write_cb(struct bufferevent *bev, void *ctx) {
      // Handle write completion if needed
    }

    static void event_cb(struct bufferevent *bev, short events, void *ctx) {
      auto instance = static_cast<io_evt*>(ctx);
      if(instance->get_io_operation()) {
        instance->get_io_operation()->handle_event(events);
      }
      if(events & BEV_EVENT_ERROR) {
        perror("Error from bufferevent");
      }
      if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_free(bev);
      }
    }
#endif

    virtual ~io_evt() = default;
    std::shared_ptr<io_operation> get_io_operation() const {return m_io_operation_p;}
    std::shared_ptr<struct bufferevent> get_buffer_evt() {return m_buffer_evt_p;}

  private:
    std::shared_ptr<struct bufferevent> m_buffer_evt_p;
    std::shared_ptr<evt_base> m_evt_base_p;
    std::string m_from_host;
    std::shared_ptr<io_operation> m_io_operation_p;
};

/*
class timer_evt : public evt {
  public:
    timer_evt(const bool& is_periodic, const std::chrono::seconds& secs): evt(is_periodic, secs) {}
    virtual ~timer_evt() = default;
    virtual int handle_event(evutil_socket_t fd, evt::events what) override;
  private:
};
*/

#endif
