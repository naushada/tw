#ifndef __session_hpp__
#define __session_hpp__

#include <cctype>
#include <string>
#include <cstring>
#include <algorithm>

#include "app_interface.hpp"

extern "C" {
#include <nghttp2/nghttp2.h>
}

// per http2 connection data
class session_data : public app_interface {
  struct custom_deleter {
    void operator()(nghttp2_session_callbacks *callbacks) {
      nghttp2_session_callbacks_del(callbacks);
    }

    void operator()(nghttp2_session *sess_p) {
      nghttp2_session_del(sess_p);
    }
  };

  // streams per connection
  public:
    struct stream_data {
      std::string m_request_path;
      std::int32_t m_stream_id;
      std::int32_t m_fd;
      stream_data(std::string request_path, std::int32_t stream_id, std::int32_t fd) :
        m_request_path(request_path),
        m_stream_id(stream_id),
        m_fd(fd) {}
      std::string request_path() const { return m_request_path;}
      std::int32_t stream_id() const {return m_stream_id;}
      std::int32_t fd() const {return m_fd;}

      void request_path(const std::string& path) {m_request_path = path;}
      void stream_id(std::int32_t& id) {m_stream_id = id;}
      void fd(std::int32_t& handle) {m_fd = handle;}
    };

    // Member Function
    session_data() : m_callbacks_p(nullptr),
      m_session_p(nullptr) {
      nghttp2_session_callbacks* callbacks_p = nullptr;
      int rv = nghttp2_session_callbacks_new(&callbacks_p);
      if(!rv) {
        m_callbacks_p.reset(callbacks_p);
      }
    }

    void create_stream_data(std::int32_t stream_id) {
      m_stream_data.emplace_back("", stream_id, m_handle);
    }
    
    bool is_stream_data_found(std::int32_t stream_id) {
      auto it = std::find_if(m_stream_data.begin(), m_stream_data.end(), [&](const auto& ent) {
        return(ent.stream_id() == stream_id);
      });
      
      return(it != m_stream_data.end());
    }

    void delete_stream_data(std::int32_t stream_id) {
      auto new_end = std::remove_if(m_stream_data.begin(), m_stream_data.end(), [&](const auto& ent) {
        return stream_id == ent.stream_id();
      });
      m_stream_data.erase(new_end, m_stream_data.end());
    }

    stream_data& get_stream_data(std::int32_t stream_id) {
      auto it = std::find_if(m_stream_data.begin(), m_stream_data.end(), [&](const auto& ent) {
          return(ent.stream_id() == stream_id);
        });

      if(it != m_stream_data.end()) {
        return *it;
      }
    }

    /* Returns int value of hex string character |c| */
    std::uint8_t hex_to_uint(std::uint8_t c) {
      if('0' <= c && c <= '9') {
        return (std::uint8_t)(c - '0');
      }
      if('A' <= c && c <= 'F') {
        return (std::uint8_t)(c - 'A' + 10);
      }
      if('a' <= c && c <= 'f') {
        return (std::uint8_t)(c - 'a' + 10);
      }
      return 0;
    }

    /* Decodes percent-encoded byte string |value| with length |valuelen|
       and returns the decoded byte string in allocated buffer. The return
       value is NULL terminated. The caller must free the returned
       string. */
    std::string percent_decode(const std::string& value) {
      std::string res;
      size_t valuelen = value.length();
      res.reserve(valuelen);

      if(valuelen > 3) {
        size_t i, j;
        for(i = 0, j = 0; i < valuelen - 2;) {
          if(value[i] != '%' || !std::isxdigit(value[i + 1]) || !std::isxdigit(value[i + 2])) {
            res[j++] = value[i++];
            continue;
          }
          res[j++] = ((hex_to_uint(value[i + 1]) << 4) + hex_to_uint(value[i + 2]));
          i += 3;
        }
        memcpy(&res[j], &value[i], 2);
      } else {
        res = value;
      }
      return res;
    }

    std::int32_t init() {
      nghttp2_session_callbacks_set_send_callback2(m_callbacks_p.get(), send_callback);
      nghttp2_session_callbacks_set_on_frame_recv_callback(m_callbacks_p.get(), on_frame_recv_callback);
      nghttp2_session_callbacks_set_on_stream_close_callback(m_callbacks_p.get(), on_stream_close_callback);
      nghttp2_session_callbacks_set_on_header_callback(m_callbacks_p.get(), on_header_callback);
      nghttp2_session_callbacks_set_on_begin_headers_callback(m_callbacks_p.get(), on_begin_headers_callback);

      nghttp2_session* sess_p = nullptr;
      // creates a session object and copied all provided callbacks into newly created 
      // session.
      std::int32_t rv = nghttp2_session_server_new(&sess_p, m_callbacks_p.get(), this);

      if(rv) {
        return(rv);
      }
      m_session_p.reset(sess_p);
      // we are done with m_callbacks_p now,
      m_callbacks_p.reset(nullptr);
    }
 
    struct event_base* get_evtbase() const {return m_evt_base_p;}
    struct bufferevent* get_bufferevent() const {return m_buffer_evt_p;}
    nghttp2_session* get_nghttp2_session() const {return m_session_p.get();}
    const std::string& client_addr() const {return m_client_addr;}
    const std::int32_t& handle() const {return m_handle;}

    std::int32_t on_request_recv(std::int32_t stream_id);

    // interface callbacks to nghttp2 library  
    static nghttp2_ssize send_callback(nghttp2_session *session,
                           const uint8_t *data, size_t length,
                           int flags, void *user_data);

    // on_frame_recv_callback: Invoked once after the entire DATA frame has been completely received
    static int on_frame_recv_callback(nghttp2_session *session,
                 const nghttp2_frame *frame, void *user_data);

    static int on_stream_close_callback(nghttp2_session *session, int32_t stream_id,
                 uint32_t error_code, void *user_data);

    // This callback is invoked for each filed in HEADER frame
    static int on_header_callback(nghttp2_session *session,
                 const nghttp2_frame *frame, const uint8_t *name,
                 size_t namelen, const uint8_t *value,
                 size_t valuelen, uint8_t flags,
                 void *user_data);

    static int on_begin_headers_callback(nghttp2_session *session,
                 const nghttp2_frame *frame,
                 void *user_data);
    
    // Hook method for libevent_adapter is interfacing with below function to nghttp2_adapter    
    virtual int handle_event(const short event) override;
    virtual int handle_read(evutil_socket_t handle, const std::string& in) override;
    virtual void handle_new_connection(const int& handle, const std::string& addr,
                   struct event_base* evbase_p,
                   struct bufferevent* bevt_p) override;
    virtual void handle_connection_close(int handle) override;

  private:
    std::unique_ptr<nghttp2_session_callbacks, custom_deleter> m_callbacks_p;
    std::unique_ptr<nghttp2_session , custom_deleter> m_session_p;
    struct event_base *m_evt_base_p;
    struct bufferevent *m_buffer_evt_p;
    std::vector<stream_data> m_stream_data;
    std::string m_client_addr;
    std::int32_t m_handle;
};







#endif
