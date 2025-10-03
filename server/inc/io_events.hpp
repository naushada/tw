#ifndef __io_adapter_hpp__
#define __io_adapter_hpp__

#include <vector>
#include <memory>

extern "C" {
#include <event2/buffer.h>
#include <event2/bufferevent.h>
}

#include "evt_adapter.hpp"
#include "io_operations.hpp"

class io_evt {
  public:
    io_evt(std::shared_ptr<evt_base> evt_base_p, evutil_socket_t handle, const std::string& peer_host, const std::int32_t& event, const std::chrono::seconds& secs, std::shared_ptr<io_operation> io_operation_p = std::make_shared<rw_operation>()) :
      m_buffer_evt_p(bufferevent_socket_new(evt_base_p->get(), handle, BEV_OPT_CLOSE_ON_FREE), bufferevent_free),
      m_evt_base_p(evt_base_p),
      m_from_host(peer_host),
      m_io_operation_p(io_operation_p) {
    }
    
    std::int32_t tx(const char* buffer, const size_t& len) {
      bufferevent_write(m_buffer_evt_p.get(), (void *)buffer, len); 
    }

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
