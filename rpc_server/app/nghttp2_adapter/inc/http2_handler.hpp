#ifndef __http2_handler_hpp__
#define __http2_handler_hpp__

#include <cctype>
#include <string>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <memory>

extern "C" {
#include <nghttp2/nghttp2.h>
#include <nghttp2/nghttp2ver.h>
}

#define ARRLEN(x) (sizeof(x) / sizeof(x[0]))

#define MAKE_NV(NAME, VALUE)                                                   \
  {                                                                            \
    (uint8_t *)NAME,   (uint8_t *)VALUE,     sizeof(NAME) - 1,                 \
    sizeof(VALUE) - 1, NGHTTP2_NV_FLAG_NONE,                                   \
  }

// per http2 connection data
class http2_handler {
  struct custom_deleter {
    void operator()(nghttp2_session_callbacks *callbacks) {
      nghttp2_session_callbacks_del(callbacks);
    }

    void operator()(nghttp2_session *ctx_p) {
      nghttp2_session_del(ctx_p);
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

    // ctor
    http2_handler() : m_callbacks_p(nullptr),
      m_ctx_p(nullptr) {
      nghttp2_session_callbacks* callbacks_p = nullptr;
      int rv = nghttp2_session_callbacks_new(&callbacks_p);
      if(!rv) {
        m_callbacks_p.reset(callbacks_p);
        this->init();
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

    void handle_new_connection(const int& handle, const std::string& addr);
    void handle_connection_close(std::int32_t handle);

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
      // nghttp2 library will invoke send_callback2 to send http2 DATA or HEADERS frame
      nghttp2_session_callbacks_set_send_callback(m_callbacks_p.get(), send_callback2);
      /**
       * @functypedef
       *
       * Callback function invoked by `nghttp2_session_recv()` and
       * `nghttp2_session_mem_recv2()` when a frame is received.  The
       * |user_data| pointer is the third argument passed in to the call to
       * `nghttp2_session_client_new()` or `nghttp2_session_server_new()`.
       *
       * If frame is HEADERS or PUSH_PROMISE, the ``nva`` and ``nvlen``
       * member of their data structure are always ``NULL`` and 0
       * respectively.  The header name/value pairs are emitted via
       * :type:`nghttp2_on_header_callback`.
       *
       * Only HEADERS and DATA frame can signal the end of incoming data.
       * If ``frame->hd.flags & NGHTTP2_FLAG_END_STREAM`` is nonzero, the
       * |frame| is the last frame from the remote peer in this stream.
       *
       * This callback won't be called for CONTINUATION frames.
       * HEADERS/PUSH_PROMISE + CONTINUATIONs are treated as single frame.
       *
       * The implementation of this function must return 0 if it succeeds.
       * If nonzero value is returned, it is treated as fatal error and
       * `nghttp2_session_recv()` and `nghttp2_session_mem_recv2()`
       * functions immediately return
       * :enum:`nghttp2_error.NGHTTP2_ERR_CALLBACK_FAILURE`.
       *
       * To set this callback to :type:`nghttp2_session_callbacks`, use
       * `nghttp2_session_callbacks_set_on_frame_recv_callback()`.
      */ 
      nghttp2_session_callbacks_set_on_frame_recv_callback(m_callbacks_p.get(), on_frame_recv_callback);
      /**
       * @functypedef
       *
       * Callback function invoked when the stream |stream_id| is closed.
       * The reason of closure is indicated by the |error_code|.  The
       * |error_code| is usually one of :enum:`nghttp2_error_code`, but that
       * is not guaranteed.  The stream_user_data, which was specified in
       * `nghttp2_submit_request2()` or `nghttp2_submit_headers()`, is still
       * available in this function.  The |user_data| pointer is the third
       * argument passed in to the call to `nghttp2_session_client_new()` or
       * `nghttp2_session_server_new()`.
       *
       * This function is also called for a stream in reserved state.
       *
       * The implementation of this function must return 0 if it succeeds.
       * If nonzero is returned, it is treated as fatal error and
       * `nghttp2_session_recv()`, `nghttp2_session_mem_recv2()`,
       * `nghttp2_session_send()`, and `nghttp2_session_mem_send2()`
       * functions immediately return
       * :enum:`nghttp2_error.NGHTTP2_ERR_CALLBACK_FAILURE`.
       *
       * To set this callback to :type:`nghttp2_session_callbacks`, use
       * `nghttp2_session_callbacks_set_on_stream_close_callback()`.
      */ 
      nghttp2_session_callbacks_set_on_stream_close_callback(m_callbacks_p.get(), on_stream_close_callback);
      /**
       * @functypedef
       *
       * Callback function invoked when a header name/value pair is received
       * for the |frame|.  The |name| of length |namelen| is header name.
       * The |value| of length |valuelen| is header value.  The |flags| is
       * bitwise OR of one or more of :type:`nghttp2_nv_flag`.
       *
       * If :enum:`nghttp2_nv_flag.NGHTTP2_NV_FLAG_NO_INDEX` is set in
       * |flags|, the receiver must not index this name/value pair when
       * forwarding it to the next hop.  More specifically, "Literal Header
       * Field never Indexed" representation must be used in HPACK encoding.
       *
       * When this callback is invoked, ``frame->hd.type`` is either
       * :enum:`nghttp2_frame_type.NGHTTP2_HEADERS` or
       * :enum:`nghttp2_frame_type.NGHTTP2_PUSH_PROMISE`.  After all header
       * name/value pairs are processed with this callback, and no error has
       * been detected, :type:`nghttp2_on_frame_recv_callback` will be
       * invoked.  If there is an error in decompression,
       * :type:`nghttp2_on_frame_recv_callback` for the |frame| will not be
       * invoked.
       *
       * Both |name| and |value| are guaranteed to be NULL-terminated.  The
       * |namelen| and |valuelen| do not include terminal NULL.  If
       * `nghttp2_option_set_no_http_messaging()` is used with nonzero
       * value, NULL character may be included in |name| or |value| before
       * terminating NULL.
       *
       * Please note that unless `nghttp2_option_set_no_http_messaging()` is
       * used, nghttp2 library does perform validation against the |name|
       * and the |value| using `nghttp2_check_header_name()` and
       * `nghttp2_check_header_value()`.  In addition to this, nghttp2
       * performs validation based on HTTP Messaging rule, which is briefly
       * explained in :ref:`http-messaging` section.
       *
       * If the application uses `nghttp2_session_mem_recv2()`, it can
       * return :enum:`nghttp2_error.NGHTTP2_ERR_PAUSE` to make
       * `nghttp2_session_mem_recv2()` return without processing further
       * input bytes.  The memory pointed by |frame|, |name| and |value|
       * parameters are retained until `nghttp2_session_mem_recv2()` or
       * `nghttp2_session_recv()` is called.  The application must retain
       * the input bytes which was used to produce these parameters, because
       * it may refer to the memory region included in the input bytes.
       *
       * Returning
       * :enum:`nghttp2_error.NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE` will
       * close the stream (promised stream if frame is PUSH_PROMISE) by
       * issuing RST_STREAM with
       * :enum:`nghttp2_error_code.NGHTTP2_INTERNAL_ERROR`.  In this case,
       * :type:`nghttp2_on_header_callback` and
       * :type:`nghttp2_on_frame_recv_callback` will not be invoked.  If a
       * different error code is desirable, use
       * `nghttp2_submit_rst_stream()` with a desired error code and then
       * return :enum:`nghttp2_error.NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE`.
       * Again, use ``frame->push_promise.promised_stream_id`` as stream_id
       * parameter in `nghttp2_submit_rst_stream()` if frame is
       * PUSH_PROMISE.
       *
       * The implementation of this function must return 0 if it succeeds.
       * It may return :enum:`nghttp2_error.NGHTTP2_ERR_PAUSE` or
       * :enum:`nghttp2_error.NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE`.  For
       * other critical failures, it must return
       * :enum:`nghttp2_error.NGHTTP2_ERR_CALLBACK_FAILURE`.  If the other
       * nonzero value is returned, it is treated as
       * :enum:`nghttp2_error.NGHTTP2_ERR_CALLBACK_FAILURE`.  If
       * :enum:`nghttp2_error.NGHTTP2_ERR_CALLBACK_FAILURE` is returned,
       * `nghttp2_session_recv()` and `nghttp2_session_mem_recv2()`
       * functions immediately return
       * :enum:`nghttp2_error.NGHTTP2_ERR_CALLBACK_FAILURE`.
       *
       * To set this callback to :type:`nghttp2_session_callbacks`, use
       * `nghttp2_session_callbacks_set_on_header_callback()`.
       *
       * .. warning::
       *
       *   Application should properly limit the total buffer size to store
       *   incoming header fields.  Without it, peer may send large number
       *   of header fields or large header fields to cause out of memory in
       *   local endpoint.  Due to how HPACK works, peer can do this
       *   effectively without using much memory on their own.
      */ 
      nghttp2_session_callbacks_set_on_header_callback(m_callbacks_p.get(), on_header_callback);
      /**
       * @functypedef
       *
       * Callback function invoked when the reception of header block in
       * HEADERS or PUSH_PROMISE is started.  Each header name/value pair
       * will be emitted by :type:`nghttp2_on_header_callback`.
       *
       * The ``frame->hd.flags`` may not have
       * :enum:`nghttp2_flag.NGHTTP2_FLAG_END_HEADERS` flag set, which
       * indicates that one or more CONTINUATION frames are involved.  But
       * the application does not need to care about that because the header
       * name/value pairs are emitted transparently regardless of
       * CONTINUATION frames.
       *
       * The server applications probably create an object to store
       * information about new stream if ``frame->hd.type ==
       * NGHTTP2_HEADERS`` and ``frame->headers.cat ==
       * NGHTTP2_HCAT_REQUEST``.  If |session| is configured as server side,
       * ``frame->headers.cat`` is either ``NGHTTP2_HCAT_REQUEST``
       * containing request headers or ``NGHTTP2_HCAT_HEADERS`` containing
       * trailer fields and never get PUSH_PROMISE in this callback.
       *
       * For the client applications, ``frame->hd.type`` is either
       * ``NGHTTP2_HEADERS`` or ``NGHTTP2_PUSH_PROMISE``.  In case of
       * ``NGHTTP2_HEADERS``, ``frame->headers.cat ==
       * NGHTTP2_HCAT_RESPONSE`` means that it is the first response
       * headers, but it may be non-final response which is indicated by 1xx
       * status code.  In this case, there may be zero or more HEADERS frame
       * with ``frame->headers.cat == NGHTTP2_HCAT_HEADERS`` which has
       * non-final response code and finally client gets exactly one HEADERS
       * frame with ``frame->headers.cat == NGHTTP2_HCAT_HEADERS``
       * containing final response headers (non-1xx status code).  The
       * trailer fields also has ``frame->headers.cat ==
       * NGHTTP2_HCAT_HEADERS`` which does not contain any status code.
       *
       * Returning
       * :enum:`nghttp2_error.NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE` will
       * close the stream (promised stream if frame is PUSH_PROMISE) by
       * issuing RST_STREAM with
       * :enum:`nghttp2_error_code.NGHTTP2_INTERNAL_ERROR`.  In this case,
       * :type:`nghttp2_on_header_callback` and
       * :type:`nghttp2_on_frame_recv_callback` will not be invoked.  If a
       * different error code is desirable, use
       * `nghttp2_submit_rst_stream()` with a desired error code and then
       * return :enum:`nghttp2_error.NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE`.
       * Again, use ``frame->push_promise.promised_stream_id`` as stream_id
       * parameter in `nghttp2_submit_rst_stream()` if frame is
       * PUSH_PROMISE.
       *
       * The implementation of this function must return 0 if it succeeds.
       * It can return
       * :enum:`nghttp2_error.NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE` to
       * reset the stream (promised stream if frame is PUSH_PROMISE).  For
       * critical errors, it must return
       * :enum:`nghttp2_error.NGHTTP2_ERR_CALLBACK_FAILURE`.  If the other
       * value is returned, it is treated as if
       * :enum:`nghttp2_error.NGHTTP2_ERR_CALLBACK_FAILURE` is returned.  If
       * :enum:`nghttp2_error.NGHTTP2_ERR_CALLBACK_FAILURE` is returned,
       * `nghttp2_session_mem_recv2()` function will immediately return
       * :enum:`nghttp2_error.NGHTTP2_ERR_CALLBACK_FAILURE`.
       *
       * To set this callback to :type:`nghttp2_session_callbacks`, use
       * `nghttp2_session_callbacks_set_on_begin_headers_callback()`.
      */ 
      nghttp2_session_callbacks_set_on_begin_headers_callback(m_callbacks_p.get(), on_begin_headers_callback);

      nghttp2_session* sess_p = nullptr;
      // creates a session object and copied all provided callbacks into newly created 
      // session. this pointer is returned to every callback invoked by nghttp2 library.
      std::int32_t rv = nghttp2_session_server_new(&sess_p, m_callbacks_p.get(), this);

      if(rv) {
        return(rv);
      }
      m_ctx_p.reset(sess_p);
      // we are done with m_callbacks_p now,
      m_callbacks_p.reset(nullptr);
    }
 
    nghttp2_session* get_nghttp2_session() const {return m_ctx_p.get();}
    const std::string& client_addr() const {return m_client_addr;}
    const std::int32_t& handle() const {return m_handle;}

    std::int32_t on_request_recv(std::int32_t stream_id);
   
    std::int32_t process_request_from_app(const std::int32_t& handle, const std::string& in_data);

    // interface callbacks to nghttp2 library  
    static ssize_t send_callback2(nghttp2_session *session,
                           const uint8_t *data, size_t length,
                           int flags, void *user_data);

    // on_frame_recv_callback: Invoked once after the entire DATA frame has been completely received
    static std::int32_t on_frame_recv_callback(nghttp2_session *session,
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
  private:
    std::unique_ptr<nghttp2_session_callbacks, custom_deleter> m_callbacks_p;
    std::unique_ptr<nghttp2_session , custom_deleter> m_ctx_p;
    std::vector<stream_data> m_stream_data;
    std::string m_client_addr;
    std::int32_t m_handle;
};







#endif
