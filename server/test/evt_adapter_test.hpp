#ifndef __evt_adapter_test_hpp__
#define __evt_adapter_test_hpp__

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

#include "evt_adapter.hpp"

class EventAdapterTest : public ::testing::Test {
  public:
    EventAdapterTest();
    virtual ~EventAdapterTest() = default;

    virtual void SetUp() override;
    virtual void TearDown() override;
    virtual void TestBody() override;
  private:
    std::unique_ptr<evt_loop> m_run;
    std::vector<evt> m_evts;
};

#endif /*__evt_adapter_test_hpp__*/
