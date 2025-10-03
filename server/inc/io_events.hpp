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
  struct deleter {
    void operator()(struct bufferevent* bevt) {bufferevent_free(bevt);}
  };

  public:
    io_evt(const evt_base& evt_base_ref, evutil_socket_t handle, const std::string& peer_host, const std::int32_t& event, const std::chrono::seconds& secs, const io_operation& io_operation_ref) :
      m_evt_base(evt_base_ref),
      m_buffer_evt_p(bufferevent_socket_new(m_evt_base.get(), handle, BEV_OPT_CLOSE_ON_FREE)),
      m_from_host(peer_host),
      m_io_operation(io_operation_ref) {
    }
    
    std::int32_t tx(const char* buffer, const size_t& len) {
      bufferevent_write(m_buffer_evt_p.get(), (void *)buffer, len); 
    }

    virtual ~io_evt() = default;
    io_operation get_io_operation() const {return m_io_operation;}
    // returning managed object of unique_ptr
    struct bufferevent* get_bufferevt() const {return m_buffer_evt_p.get();}

  private:
    const evt_base& m_evt_base;
    std::unique_ptr<struct bufferevent, io_evt::deleter> m_buffer_evt_p;
    std::string m_from_host;
    const io_operation& m_io_operation;
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
