#ifndef __app_main_cpp__
#define __app_main_cpp__

#include "interface_impl.hpp"
#include "tcp_server.hpp"

int main() {

  auto svc = std::make_unique<tcp_server<app>>("0.0.0.0", 58989);
  std::cerr << "Server started on port:58989" << std::endl;
  run_evt_loop start; 
  start(svc->get_event_base());
  
  std::cerr << "Server stopped on port:58989" << std::endl;
  return 0;
}













#endif
