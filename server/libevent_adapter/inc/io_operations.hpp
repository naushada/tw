#ifndef __io_operations_hpp__
#define __io_operations_hpp__

#include <vector>
#include "evt_adapter.hpp"

extern "C" {
#include <event2/bufferevent.h>
}

class io_operation {
  public:
    void set_bufferevent(struct bufferevent* bevt_p) {
      m_bevt_p = bevt_p; 
    }

    virtual int handle_event(evutil_socket_t fd, evt::events what) {return(0);}
    virtual int handle_event(const short event) {return 0;}
    virtual int handle_read(evutil_socket_t handle, const std::vector<std::uint8_t>& in, const size_t& nbytes) { 
      std::cout << "io_operation::handle_read" << std::endl;return(0);
    }

    virtual int handle_read(evutil_socket_t handle, const std::string& in) { 
      std::cout << "io_operation::handle_read:" << in << std::endl;
      return(0);
    }
   
    virtual void handle_connection_new(const int& handle, const std::string& addr,
                                       struct event_base* evbase_p,
                                       struct bufferevent* bevt_p) {
      std::cout << "This must be overriden in subclass" << std::endl;
    }

    virtual void handle_connection_close(int handle) {
    }

  protected:
    struct bufferevent* get_bufferevent() {return m_bevt_p;}

    struct bufferevent* m_bevt_p;
    std::int32_t m_handle;
};

class rw_operation : public io_operation {
  public:
    virtual int handle_event(evutil_socket_t fd, evt::events what) override;
    virtual int handle_event(const short event) override;
    virtual int handle_read(evutil_socket_t handle, const std::vector<std::uint8_t>& in, const size_t& nbytes) override;
    virtual int handle_read(evutil_socket_t handle, const std::string& in) override;
    virtual void handle_connection_new(int handle) override;
    virtual void handle_connection_close(int handle) override;
  private:
    std::int32_t m_handle;
};






#endif
