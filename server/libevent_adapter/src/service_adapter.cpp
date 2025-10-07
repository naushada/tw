#ifndef __service_adapter_cpp__
#define __service_adapter_cpp__

#include "service_adapter.hpp"

int main(std::int32_t argc, char* argv[]) {

  evt_base base;
  server_service<session_data> svc(base, "0.0.0.0", 8989);
  return(0);

}

#endif
