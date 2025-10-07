#ifndef __app_interface_hpp__
#define __app_interface_hpp__

#include <vector>
#include "evt_adapter.hpp"

extern "C" {
#include <event2/bufferevent.h>
}

class app_interface {
  public:
    virtual int handle_event(const short event) {
      std::cout << "app_interface::handle_event must be overridden in sub-class event:"<< event << std::endl;
      return 0;
    }
    virtual int handle_read(evutil_socket_t handle, const std::string& in) { 
      std::cout << "handle:" << handle << " app_interface::handle_read:" << in << std::endl;
      return(0);
    }
   
    virtual void handle_new_connection(const int& handle, const std::string& addr,
                   struct event_base* evbase_p,
                   struct bufferevent* bevt_p) {
      std::cout << "app_interface handle_new_connection This must be overriden in subclass" << std::endl;
      (void) handle;
      (void) addr;
      (void) evbase_p;
      (void) bevt_p;
    }

    virtual void handle_connection_close(int handle) {
      std::cout << "app_interface handle_connection_close This must be overriden in subclass" << std::endl;
      (void) handle;
    }

  protected:
};

class rw_operation : public app_interface {
  public:
    virtual int handle_event(const short event) override;
    virtual int handle_read(evutil_socket_t handle, const std::string& in) override;
    virtual void handle_new_connection(const int& handle, const std::string& addr,
                   struct event_base* evbase_p,
                   struct bufferevent* bevt_p) override;
    virtual void handle_connection_close(int handle) override;
  private:
    std::int32_t m_handle;
};






#endif
