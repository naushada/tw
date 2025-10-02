#ifndef __io_operations_hpp__
#define __io_operations_hpp__

#include <vector>
#include "evt_adapter.hpp"

extern "C" {
#include <event2/bufferevent.h>
}

class io_operation {
  public:  
    virtual int handle_event(evutil_socket_t fd, evt::events what) {return(0);}
    virtual int handle_read(evutil_socket_t handle, const std::vector<std::uint8_t>& in, const size_t& nbytes) {return(0);}
};

class rw_operation : public io_operation {
  public:
    virtual int handle_event(evutil_socket_t fd, evt::events what) override;
    virtual int handle_read(evutil_socket_t handle, const std::vector<std::uint8_t>& in, const size_t& nbytes) override;
};






#endif
