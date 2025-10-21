#ifndef __app_main_cpp__
#define __app_main_cpp__

#include "interface_impl.hpp"
#include "tcp_server.hpp"

int main() {

  auto svc = std::make_unique<tcp_server<app>>("127.0.0.1", 8989);
  run_evt_loop start; 
  start(svc->get_event_base()); 
  return 0;
}













#endif
