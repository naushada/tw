#ifndef __session_cpp__
#define __session_cpp__

#include "session.hpp"

// virtual methods for io_operations
int hhttp2_conn_data::handle_event(evutil_socket_t fd, evt::events what) {
}

int http2_conn_data::handle_event(const short event) {
}
int http2_conn_data::handle_read(evutil_socket_t handle, const std::vector<std::uint8_t>& in, const size_t& nbytes) {
}
int http2_conn_data::handle_read(evutil_socket_t handle, const std::string& in) {
}

void handle_connection_new(const int& handle, const std::string& addr,
                           struct event_base* evbase_p,
                           struct bufferevent* bevt_p) {
  m_handle = handle;
  m_client_addr = addr;
  m_evt_base_p = evbase_p;
  m_buffer_evt_p = bevt_p;
}

void http2_conn_data::handle_connection_close(int handle) {
}

nghttp2_ssize http2_conn_data::send_callback(nghttp2_session *session,
                                   const uint8_t *data, size_t length,
                                   int flags, void *user_data) {
  http2_conn_data *instance = static_cast<http2_conn_data*>(user_data);
  struct bufferevent *bev = session_data->bev;
  /* Avoid excessive buffering in server side. */
  if(evbuffer_get_length(bufferevent_get_output(session_data->bev)) >= OUTPUT_WOULDBLOCK_THRESHOLD) {
    return NGHTTP2_ERR_WOULDBLOCK;
  }

  bufferevent_write(bev, data, length);
  return (nghttp2_ssize)length;
}


int http2_conn_data::on_frame_recv_callback(nghttp2_session *session,
                                            const nghttp2_frame *frame, 
                                            void *user_data) {
  http2_conn_data *instance = static_cast<http2_conn_data*>(user_data);
  http2_stream_data *stream_data;
  switch(frame->hd.type) {
    case NGHTTP2_DATA:
    case NGHTTP2_HEADERS:
      /* Check that the client request has finished */
      if(frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
        stream_data =
          nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
        /* For DATA and HEADERS frame, this callback may be called after
           on_stream_close_callback. Check that stream still alive. */
        if(!stream_data) {
          return 0;
        }
        return on_request_recv(session, session_data, stream_data);
      }
      break;
    default:
      break;
  }
  return 0;
}

int http2_conn_data::on_stream_close_callback(nghttp2_session *session, int32_t stream_id,
                                        uint32_t error_code _U_, void *user_data) {
}

int http2_conn_data::on_header_callback(nghttp2_session *session,
                                        const nghttp2_frame *frame, const uint8_t *name,
                                        size_t namelen, const uint8_t *value,
                                        size_t valuelen, uint8_t flags,
                                        void *user_data) {
  http2_stream_data *stream_data;
  const char PATH[] = ":path";
  (void)flags;
  (void)user_data;

  switch(frame->hd.type) {
    case NGHTTP2_HEADERS:
      if(frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
        break;
      }
      stream_data =
        nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
      if(!stream_data || stream_data->request_path) {
        break;
      }

      if(namelen == sizeof(PATH) - 1 && memcmp(PATH, name, namelen) == 0) {
        size_t j;
        for(j = 0; j < valuelen && value[j] != '?'; ++j)
          ;
        stream_data->request_path = percent_decode(value, j);
      }
      break;
    }
  return 0;
}

int http2_conn_data::on_begin_headers_callback(nghttp2_session *session,
                                         const nghttp2_frame *frame,
                                         void *user_data) {
  http2_session_data *session_data = (http2_session_data *)user_data;
  http2_stream_data *stream_data;

  if (frame->hd.type != NGHTTP2_HEADERS ||
      frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
    return 0;
  }
  stream_data = create_http2_stream_data(session_data, frame->hd.stream_id);
  nghttp2_session_set_stream_user_data(session, frame->hd.stream_id,
                                       stream_data);
  return 0;
}

#endif
