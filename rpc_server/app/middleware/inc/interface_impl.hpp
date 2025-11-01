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
    virtual ~app() { std::cout <<"fn:"<< __PRETTY_FUNCTION__ << ":" << __LINE__ << " app interface is reclaimed" << std::endl;}

    virtual int handle_event(const short event) override;
    virtual int handle_read(std::int32_t handle, const std::string& in) override;
    virtual void handle_new_connection(const int& handle, const std::string& addr) override;
    virtual void handle_connection_close(int handle) override;

    // helper function
    // Returning managed object of unique_ptr to caller.
    http2_handler& get_http2_handler() { return *m_http2_handler;}
    std::int32_t handle() const { return m_handle; }
    std::string addr() const { return m_addr; }
    
    std::unordered_map<std::string, std::string>& get_requested_rpc() {return m_requested_rpc;}
    void insert_requested_rpc(const std::string& rpc_path, const std::string& rpc_request) { m_requested_rpc.emplace(rpc_path, rpc_request);}

  private:
    std::unique_ptr<http2_handler> m_http2_handler;
    std::int32_t m_handle;
    std::string m_addr;
    std::unordered_map<std::string, std::string> m_requested_rpc;
};

#endif
