#ifndef __evt_adapter_hpp__
#define __evt_adapter_hpp__

#include <iostream>
#include <memory>
#include <chrono>
#include <memory>
#include <vector>

#include "interface.hpp"

class evt_base {
  struct custom_deleter {
    void operator()(struct event_base* evt) {event_base_free(evt);}
  };

  public:
    evt_base(std::int32_t priority = 0): m_event_base_p(event_base_new()) {
      if(priority > 0 && priority <= 255) {
        event_base_priority_init(m_event_base_p.get(), priority);
      }
    }

    struct event_base* operator->() const {
      return m_event_base_p.get();
    }

    struct event_base& operator*() const {
      return *m_event_base_p;
    }
    
    struct event_base* get() const {
      return m_event_base_p.get();
    }

    struct event_base* get() {
      return m_event_base_p.get();
    }

    explicit operator bool() const {return (m_event_base_p != nullptr);}

  private:
    std::unique_ptr<struct event_base, evt_base::custom_deleter> m_event_base_p; 
};

class evt_io {
  struct custom_deleter {
    void operator()(struct bufferevent* bevt) {bufferevent_free(bevt);}
  };

  public:
    evt_io(const evt_base& evt_base_ref, evutil_socket_t handle,
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

    ~evt_io() { std::cout <<"fn:" << __PRETTY_FUNCTION__ <<":" << __LINE__ <<" dtor"<<std::endl;}
    std::unique_ptr<app_interface>& get_app_interface() {return m_app_interface;}
    // returning managed object of unique_ptr
    struct bufferevent* get_bufferevt() const {return m_buffer_evt_p.get();}

  private:
    const evt_base& m_evt_base;
    std::unique_ptr<struct bufferevent, evt_io::custom_deleter> m_buffer_evt_p;
    std::string m_from_host;
    std::unique_ptr<app_interface> m_app_interface;
};

struct run_evt_loop {
  int operator()(const evt_base& base) {event_base_dispatch(base.get()); return 0;}
};

#endif
