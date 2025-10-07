#ifndef __io_events_hpp__
#define __io_events_hpp__

#include <vector>
#include <memory>

#include "evt_adapter.hpp"
#include "app_interface.hpp"

extern "C" {
#include <event2/buffer.h>
#include <event2/bufferevent.h>
}

class io_evt {
  struct custom_deleter {
    void operator()(struct bufferevent* bevt) {bufferevent_free(bevt);}
  };

  public:
    io_evt(const evt_base& evt_base_ref, evutil_socket_t handle,
           const std::string& peer_host, const std::int32_t& event,
           const std::chrono::seconds& secs, std::unique_ptr<app_interface> app_intf) :
      m_evt_base(evt_base_ref),
      m_buffer_evt_p(bufferevent_socket_new(m_evt_base.get(), handle, BEV_OPT_CLOSE_ON_FREE)),
      m_from_host(peer_host),
      m_app_interface(std::move(app_intf)) {
      get_app_interface()->handle_new_connection(handle, m_from_host,
                                            m_evt_base.get(), m_buffer_evt_p.get());
     
    }
    
    std::int32_t tx(const char* buffer, const size_t& len) {
      bufferevent_write(m_buffer_evt_p.get(), (void *)buffer, len); 
    }

    virtual ~io_evt() = default;
    std::unique_ptr<app_interface>& get_app_interface() {return m_app_interface;}
    // returning managed object of unique_ptr
    struct bufferevent* get_bufferevt() const {return m_buffer_evt_p.get();}

  private:
    const evt_base& m_evt_base;
    std::unique_ptr<struct bufferevent, io_evt::custom_deleter> m_buffer_evt_p;
    std::string m_from_host;
    std::unique_ptr<app_interface> m_app_interface;
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
