#ifndef __io_operations_cpp__
#define __io_operations_cpp__


#include <iostream>
#include "io_operations.hpp"

int rw_operation::handle_event(const short event) {
  std::cout <<"handle:" << m_handle << " event:" << event << std::endl;
  return(0); 
}

int rw_operation::handle_read(evutil_socket_t handle, const std::string& in) {
  std::cout << "rw_operation::handle_read:" << in << std::endl;
  return(0);
}


void rw_operation::handle_connection_new(const int& handle, const std::string& addr,
                     struct event_base* evbase_p,
                     struct bufferevent* bevt_p) {
}

void rw_operation::handle_connection_close(int handle) {

}



#endif
