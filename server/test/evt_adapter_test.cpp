#ifndef __evt_adapter_test_cpp__
#define __evt_adapter_test_cpp__

#include <chrono>
#include "io_adapter.hpp"
#include "evt_adapter_test.hpp"


void  EventAdapterTest::SetUp() {
  m_evts.emplace_back(timer_evt(false, std::chrono::seconds(2)));
  m_evts.emplace_back(io_evt(evt::events::Read, std::chrono::seconds(2)));
}


void  EventAdapterTest::TearDown() {
}

void  EventAdapterTest::TestBody() {
}

TEST(EventAdapterTest, Timer_Event) {
}













#endif /*__evt_adapter_test_cpp__*/
