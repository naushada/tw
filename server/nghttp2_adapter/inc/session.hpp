#ifndef __session_hpp__
#define __session_hpp__

#include "io_operations.hpp"

extern "C" {
#include <nghttp2/nghttp2.h>
}

// per http2 connection data
class http2_conn_data : public io_operation {
  struct custom_deleter {
    void operator()(nghttp2_session_callbacks *callbacks) {
      nghttp2_session_callbacks_del(callbacks);
    }

    void operator()(nghttp2_session *sess_p) {
      nghttp2_session_del(sess_p);
    }
  }; 
  public:
    http2_conn_data() : m_callbacks_p(nullptr),
      m_session_p(nullptr) {
      nghttp2_session_callbacks* callbacks_p = nullptr;
      int rv = nghttp2_session_callbacks_new(&callbacks_p);
      if(!rv) {
        m_callbacks_p.reset(callbacks_p);
      }
    }
  
    static nghttp2_ssize send_callback(nghttp2_session *session,
                                       const uint8_t *data, size_t length,
                                       int flags, void *user_data);
    static int on_frame_recv_callback(nghttp2_session *session,
                                      const nghttp2_frame *frame, void *user_data);
    static int on_stream_close_callback(nghttp2_session *session, int32_t stream_id,
                                        uint32_t error_code, void *user_data);
    static int on_header_callback(nghttp2_session *session,
                                  const nghttp2_frame *frame, const uint8_t *name,
                                  size_t namelen, const uint8_t *value,
                                  size_t valuelen, uint8_t flags,
                                  void *user_data);
    static int on_begin_headers_callback(nghttp2_session *session,
                                         const nghttp2_frame *frame,
                                         void *user_data);
    
    std::int32_t init() {

      nghttp2_session_callbacks_set_send_callback2(m_callbacks_p.get(), send_callback);
      nghttp2_session_callbacks_set_on_frame_recv_callback(m_callbacks_p.get(), on_frame_recv_callback);
      nghttp2_session_callbacks_set_on_stream_close_callback(m_callbacks_p.get(), on_stream_close_callback);
      nghttp2_session_callbacks_set_on_header_callback(m_callbacks_p.get(), on_header_callback);
      nghttp2_session_callbacks_set_on_begin_headers_callback(m_callbacks_p.get(), on_begin_headers_callback);

      nghttp2_session* sess_p = nullptr;
      int rv = nghttp2_session_server_new(&sess_p, m_callbacks_p.get(), this);
      if(rv) {
        return(rv);
      }

      m_session_p.reset(sess_p);
      // reclaim memory now
      m_callbacks_p.reset(nullptr);
    }

    // libevent_adapter is interfacing with below function to nghttp2_adapter    
    virtual int handle_event(evutil_socket_t fd, evt::events what) override;
    virtual int handle_event(const short event) override;
    virtual int handle_read(evutil_socket_t handle, const std::vector<std::uint8_t>& in, const size_t& nbytes) override;
    virtual int handle_read(evutil_socket_t handle, const std::string& in) override;
    virtual void handle_connection_new(const int& handle, const std::string& addr,
                                       struct event_base* evbase_p,
                                       struct bufferevent* bevt_p) override;
    virtual void handle_connection_close(int handle) override;

  private:
    std::unique_ptr<nghttp2_session_callbacks, custom_deleter> m_callbacks_p;
    std::unique_ptr<nghttp2_session , custom_deleter> m_sesion_p;
    const struct event_base *m_evt_base_p;
    const struct bufferevent *m_buffer_evt_p;
    //struct http2_stream_data m_stream_data;
    //SSL_CTX *m_ssl_ctx_p;
    std::string m_client_addr;
    std::int32_t m_handle;

};







#endif
