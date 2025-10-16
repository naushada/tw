#ifndef __evt_adapter_test_cpp__
#define __evt_adapter_test_cpp__

#include <chrono>
#include "evt_adapter.hpp"
#include "evt_adapter_test.hpp"


ServerAppTest::ServerAppTest():
  //m_base(std::make_unique<evt_base>()),
  //m_run(std::make_unique<evt_loop>()),
  m_svcs_p(std::make_unique<tcp_server<io_operation>>("127.0.0.1", 8989)) {
  //m_svcs_p = std::make_shared<rw_operation>(m_base, "0.0.0.0", 8080);
}

void  ServerAppTest::SetUp() {
  //m_evts.emplace_back(timer_evt(false, std::chrono::seconds(2)));
  //m_svcs.emplace_back(server_service("0.0.0.0", 8080));
}


void  ServerAppTest::TearDown() {
}

void  ServerAppTest::TestBody() {
}

TEST_F(ServerAppTest, Listener_Event) {
  run();
}













#endif /*__evt_adapter_test_cpp__*/
