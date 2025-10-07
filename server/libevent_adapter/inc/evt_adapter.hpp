#ifndef __evt_adapter_hpp__
#define __evt_adapter_hpp__

#include <iostream>
#include <memory>
#include <chrono>
#include <memory>

extern "C" {
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <event2/event.h>
  #include <sys/time.h>
}

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

class evt {
  public:
    enum class events {
      Timeout = EV_TIMEOUT,
      Read = EV_READ,
      Write = EV_WRITE,
      Signal = EV_SIGNAL,
      Persist = EV_PERSIST,
      ET = EV_ET,
      All = (Timeout|Read|Write|Signal|Persist|ET)
    };

    evt(std::shared_ptr<evt_base> base, evt::events evt_type, evutil_socket_t handle) : 
      m_evt_base_p(base),
      m_events_p(event_new(m_evt_base_p->get(), handle, to_int(evt_type), evt_callback, this), event_free),
      m_fd(handle) {
//        m_events_p = std::make_shared<struct event>(event_new(m_evt_base_p->get(), m_fd, to_int(evt_type), evt_callback, this), event_free);
        add_event();
      }
 
    evt(std::shared_ptr<evt_base> base, evt::events evt_type, std::chrono::seconds to_secs) : 
      m_evt_base_p(base),
      m_events_p(event_new(m_evt_base_p->get(), -1, to_int(evt_type), evt_callback, this), event_free),
      m_fd(-1),
      m_to{.tv_sec=0, .tv_usec=0} {
        m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        m_to.tv_sec= to_secs.count();
        m_to.tv_usec = 0;
        add_event(&m_to);
      }

    evt(std::shared_ptr<evt_base> base, const bool& is_periodic, const std::chrono::seconds& to_secs) : 
      m_evt_base_p(base),
      m_events_p(event_new(m_evt_base_p->get(), -1, to_int(evt::events::Timeout), evt_callback, this), event_free),
      m_fd(-1),
      m_is_periodic(is_periodic) {

        if(is_periodic) {
          //m_events_p = std::make_shared<struct event>(event_new(m_evt_base_p->get(), -1, (to_int(evt::events::Timeout) | to_int(evt::events::Persist)), evt_callback, this), event_free);
        } else {
          //m_events_p = std::make_shared<struct event>(event_new(m_evt_base_p->get(), -1, to_int(evt::events::Timeout), evt_callback, this), event_free);
        }

        m_to.tv_sec= to_secs.count();
        m_to.tv_usec = 0;
        add_event(&m_to);
     }

    explicit operator bool() const {return m_fd > 0;}
    

    static void evt_callback(evutil_socket_t fd, short what, void* arg) {
      events event_type{what};
      
      evt* instance = static_cast<evt*>(arg);

      switch(event_type) {
        case events::Timeout:
        {
          std::cerr << "fd:" << instance->fd() << " event:" << instance->to_int(event_type) << " msg:events::Timeout" << std::endl;
        }
        break;
        case events::Read:
        {
        }
        break;
        case events::Write:
        {
        }
        break;
        case events::Signal:
        {
        }
        break;
        case events::Persist:
        {
        }
        break;
        case events::ET:
        {
        }
        break;

        default:
        {
        }
      }
 
      instance->handle_event(fd, event_type);
    }
    
    int add_event(const struct timeval* tv) {
      if (m_events_p) {
        return event_add(m_events_p.get(), tv);
      }
      return -1;
    }

    int add_event() {
      if (m_events_p) {
        return event_add(m_events_p.get(), NULL);
      }
      return -1;
    }

    int del_event() {
      if (m_events_p) {
        return event_del(m_events_p.get());
      }
      return -1;
    }

    bool is_periodic() const { return m_is_periodic;}

    virtual int handle_event(evutil_socket_t fd, evt::events what) {
      return(-1);
    }

    std::int32_t stop_timer() { 
      if(fd() < 0) {
        return(-1);
      }
      del_event();
    }
    
    std::int32_t start_timer(const std::chrono::seconds& secs) {
      if(fd() < 0) {
        return(-1);
      }
      m_to.tv_sec= secs.count();
      m_to.tv_usec = 0;
      add_event(&m_to);
    }

    std::int32_t fd() const { return m_fd;}
  protected:
    std::int32_t to_int(events _e) {
      return static_cast<std::int32_t>(_e);
    }

  private:
    std::shared_ptr<evt_base> m_evt_base_p;
    std::shared_ptr<struct event> m_events_p;
    std::int32_t m_fd;
    struct timeval m_to;
    bool m_is_periodic;
};

struct evt_loop {
  int operator()(const evt_base& base) {event_base_dispatch(base.get());}
};

#endif
