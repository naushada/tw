#ifndef __interface_hpp__
#define __interface_hpp__

#include <string>
#include <iostream>
#include <ostream>

extern "C" {
  #include <event2/buffer.h>
  #include <event2/bufferevent.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <event2/event.h>
  #include <sys/time.h>
}

class app_interface {
  public:

    struct event_base* get_evtbase() const {return m_evt_base_p;}
    struct bufferevent* get_bufferevent() const {return m_buffer_evt_p;}

    void handle_new_connection(const int& handle, const std::string& addr,
                   struct event_base* evbase_p,
                   struct bufferevent* bevt_p) {
      std::cout << "app_interface handle_new_connection " << std::endl;
      m_handle = handle;
      m_evt_base_p = evbase_p;
      m_buffer_evt_p = bevt_p;
      handle_new_connection(handle, addr);
    }

    virtual int handle_event(const short event) {
      std::cout << "app_interface::handle_event must be overridden in sub-class event:"<< event << std::endl;
      return 0;
    }

    virtual int handle_read(std::int32_t handle, const std::string& in) { 
      std::cout << "handle:" << handle << " app_interface::handle_read:" << in << std::endl;
      return(0);
    }
   
    virtual void handle_new_connection(const int& handle, const std::string& addr) {
      std::cout << "app_interface handle_new_connection This must be overriden in subclass" << std::endl;
      (void) handle;
      (void) addr;
    }

    virtual void handle_connection_close(int handle) {
      std::cout << "app_interface handle_connection_close This must be overriden in subclass" << std::endl;
      (void) handle;
    }

  protected:
    struct event_base *m_evt_base_p;
    struct bufferevent *m_buffer_evt_p;
    std::int32_t m_handle;
};




#endif
