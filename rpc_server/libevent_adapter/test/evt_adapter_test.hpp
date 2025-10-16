#ifndef __evt_adapter_test_hpp__
#define __evt_adapter_test_hpp__

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

#include "server_app.hpp"

class io_operation: public app_interface {
public:
    virtual int handle_event(const short event) override {
      std::cout << "app_interface::handle_event must be overridden in sub-class event:"<< event << std::endl;
      return 0;
    }

    virtual int handle_read(evutil_socket_t handle, const std::string& in) override { 
      std::cout << "handle:" << handle << " app_interface::handle_read:" << in << std::endl;
      return(0);
    }
   
    virtual void handle_new_connection(const int& handle, const std::string& addr,
                   struct event_base* evbase_p,
                   struct bufferevent* bevt_p) override {
      std::cout << "app_interface handle_new_connection This must be overriden in subclass" << std::endl;
      (void) handle;
      (void) addr;
      (void) evbase_p;
      (void) bevt_p;
    }

    virtual void handle_connection_close(int handle) override {
      std::cout << "app_interface handle_connection_close This must be overriden in subclass" << std::endl;
      (void) handle;
    }

};

class ServerAppTest : public ::testing::Test {
  public:
    ServerAppTest();
    virtual ~ServerAppTest() = default;

    virtual void SetUp() override;
    virtual void TearDown() override;
    virtual void TestBody() override;
    //auto svc() {return m_svcs_p;}
    //auto& base() {return *m_base;}
    void run() { m_run(m_svcs_p->get_event_base());}

  private:
    //std::unique_ptr<evt_base> m_base;
    //std::unique_ptr<evt_loop> m_run;
    evt_loop m_run;
    std::unique_ptr<server_app<io_operation>> m_svcs_p;
};

#endif /*__evt_adapter_test_hpp__*/
