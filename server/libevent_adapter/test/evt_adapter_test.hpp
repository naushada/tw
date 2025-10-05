#ifndef __evt_adapter_test_hpp__
#define __evt_adapter_test_hpp__

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

#include "service_adapter.hpp"

class EventAdapterTest : public ::testing::Test {
  public:
    EventAdapterTest();
    virtual ~EventAdapterTest() = default;

    virtual void SetUp() override;
    virtual void TearDown() override;
    virtual void TestBody() override;
    //auto svc() {return m_svcs_p;}
    auto& base() {return *m_base;}
    void run() { m_run(base());}

  private:
    std::unique_ptr<evt_base> m_base;
    //std::unique_ptr<evt_loop> m_run;
    evt_loop m_run;
    std::unique_ptr<server_service<rw_operation>> m_svcs_p;
};

#endif /*__evt_adapter_test_hpp__*/
