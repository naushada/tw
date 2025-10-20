#ifndef __app_interface_hpp__
#define __app_interface_hpp__

#include "http2_handler.hpp"
#include "interface.hpp"

class app : public app_interface {
  public:
    app(): app_interface(),
      m_http2_handler(std::make_unique<http2_handler>()) {
    
    }

    virtual int handle_event(const short event) override;
    virtual int handle_read(std::int32_t handle, const std::string& in) override;
    virtual void handle_new_connection(const int& handle, const std::string& addr) override;
    virtual void handle_connection_close(int handle) override;
  private:
    std::unique_ptr<http2_handler> m_http2_handler;
};

#endif
