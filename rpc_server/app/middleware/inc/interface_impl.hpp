#ifndef __interface_impl_hpp__
#define __interface_impl_hpp__

#include "interface.hpp"
#include "http2_handler.hpp"

class app : public app_interface {
  public:

    app(): app_interface(),
      m_http2_handler(std::make_unique<http2_handler>()),
      m_handle(-1),
      m_addr() {
    
    }

    virtual int handle_event(const short event) override;
    virtual int handle_read(std::int32_t handle, const std::string& in) override;
    virtual void handle_new_connection(const int& handle, const std::string& addr) override;
    virtual void handle_connection_close(int handle) override;

    // helper function
    http2_handler& get_http2_handler() { return *m_http2_handler;}
    std::int32_t handle() const { return m_handle; }
    std::string addr() const { return m_addr; }

  private:
    std::unique_ptr<http2_handler> m_http2_handler;
    std::int32_t m_handle;
    std::string m_addr;
};

#endif
