#ifndef __evt_adapter_test_cpp__
#define __evt_adapter_test_cpp__

#include <chrono>
#include "io_events.hpp"
#include "evt_adapter_test.hpp"


EventAdapterTest::EventAdapterTest():
  m_base(std::make_shared<evt_base>()),
  //m_run(std::make_unique<evt_loop>()),
  m_svcs_p(std::make_shared<server_service<rw_operation>>(*m_base, "0.0.0.0", 8080)) {
  //m_svcs_p = std::make_shared<rw_operation>(m_base, "0.0.0.0", 8080);
}

void  EventAdapterTest::SetUp() {
  //m_evts.emplace_back(timer_evt(false, std::chrono::seconds(2)));
  //m_svcs.emplace_back(server_service("0.0.0.0", 8080));
}


void  EventAdapterTest::TearDown() {
}

void  EventAdapterTest::TestBody() {
}

TEST_F(EventAdapterTest, Listener_Event) {
  run();
}













#endif /*__evt_adapter_test_cpp__*/
