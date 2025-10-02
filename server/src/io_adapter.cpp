#ifndef __io_adapter_cpp__
#define __io_adapter_cpp__

#include <iostream>
#include "io_operations.hpp"

int rw_operation::handle_read(evutil_socket_t handle, const std::vector<std::uint8_t>& in, const size_t& nbytes) {
  std::cout << "Request of size:"<< nbytes <<std::endl;
  return(0);
}

int rw_operation::handle_event(evutil_socket_t fd, evt::events what) {
  std::cout << "handle event" << std::endl;
  return(0);
}

/*
int timer_evt::handle_event(evutil_socket_t handle, evt::events what) {
  std::cout <<"event on handle:"<< handle<<std::endl;
  return(0);
}
*/










#endif
