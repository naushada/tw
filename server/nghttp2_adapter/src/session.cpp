#ifndef __session_cpp__
#define __session_cpp__

#include "session.hpp"

// virtual methods for io_operations
int session_data::handle_event(const short what) {
  std::cout << "fn:" << __pretty_function__ << " the event:" << what << " is" << std::endl;
  if(events & BEV_EVENT_CONNECTED) {
    std::cout << "fn:" << __pretty_function__ << " peer is connected" << std::endl;
    #if 0
    //const unsigned char *alpn = NULL;
    //unsigned int alpnlen = 0;
    //SSL *ssl;

    fprintf(stderr, "%s connected\n", session_data->client_addr);

    ssl = bufferevent_openssl_get_ssl(session_data->bev);

    SSL_get0_alpn_selected(ssl, &alpn, &alpnlen);

    if (alpn == NULL || alpnlen != 2 || memcmp("h2", alpn, 2) != 0) {
      fprintf(stderr, "%s h2 is not negotiated\n", session_data->client_addr);
      delete_http2_session_data(session_data);
      return;
    }

    initialize_nghttp2_session(session_data);

    if(send_server_connection_header(session_data) != 0 ||
        session_send(session_data) != 0) {
      delete_http2_session_data(session_data);
      return;
    }

    return;
  #endif
  }

  if(events & BEV_EVENT_EOF) {
    std::cout << "fn:" << __pretty_function__<<" peer:"<< client_addr() << 
                 " for handle:", handle() << " is closed"<<std::endl;
  } else if(events & BEV_EVENT_ERROR) {
    std::cout << "fn:" << __pretty_function__<<" peer:"<< client_addr() << 
                 " for handle:", handle() << " event Error"<<std::endl;
  } else if(events & BEV_EVENT_TIMEOUT) {
    std::cout << "fn:" << __pretty_function__<<" peer:"<< client_addr() << 
                 " for handle:", handle() << " event timed out"<<std::endl;
  }
  //delete_http2_session_data(session_data);
  return 0;
}

int session_data::handle_read(evutil_socket_t handle, const std::string& in) {
  nghttp2_ssize readlen;

  // m_session_p holds all the registered callback (done in init). This will
  // decode the in.data into http2 frames and calls respective callback to deliver to
  // session_data.
  readlen = nghttp2_session_mem_recv2(m_session_p.get(), in.data(), in.length());
  if(readlen < 0) {
    std::cout << "fn:" << __pretty_function__ <<" line:" << __LINE__ << 
                 " Fatal error:" << nghttp2_strerror((int)readlen) << std::endl;
    return -1;
  }
  std::cout <<"fn:" << __pretty_function__ << " readlen:"<< readlen << std::endl;
/*
  if(evbuffer_drain(input, (size_t)readlen) != 0) {
    warnx("Fatal error: evbuffer_drain failed");
    return -1;
  }
*/
  if(session_send(m_session_p.get()) != 0) {
    std::cout <<"line:"<< __LINE__ << " session_send failed" << std::endl;
    return -1;
  }
  return 0;
}

// new client is connected to this server
void session_data::handle_connection_new(const int& handle, const std::string& addr,
                        struct event_base* evbase_p,
                        struct bufferevent* bevt_p) {
  m_handle = handle;
  m_client_addr = addr;
  m_evt_base_p = evbase_p;
  m_buffer_evt_p = bevt_p;
}

void session_data::handle_connection_close(int handle) {
}

nghttp2_ssize session_data::send_callback(nghttp2_session *ng_session,
                                 const uint8_t *data, size_t length,
                                 int flags, void *user_data) {
  session_data *sess_data = static_cast<session_data*>(user_data);
  struct bufferevent *bev = session_data->get_bufferevent();
  /* Avoid excessive buffering in server side. */
  if(evbuffer_get_length(bufferevent_get_output(bev)) >= OUTPUT_WOULDBLOCK_THRESHOLD) {
    return NGHTTP2_ERR_WOULDBLOCK;
  }

  bufferevent_write(bev, data, length);
  return (nghttp2_ssize)length;
}

int session_data::on_frame_recv_callback(nghttp2_session *ng_session,
                       const nghttp2_frame *frame, 
                       void *user_data) {
  session_data *sess_data = static_cast<session_data*>(user_data);
  switch(frame->hd.type) {
    case NGHTTP2_DATA:
    case NGHTTP2_HEADERS:
      /* Check that the client request has finished */
      if(frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
        if(!sess_data->is_stream_data_found(frame->hd.stream_id)) {
          return 0;
        }

        /* For DATA and HEADERS frame, this callback may be called after
           on_stream_close_callback. Check that stream still alive. */
        auto& stream_data = sess_data->get_stream_data(frame->hd.stream_id);
        return sess_data->on_request_recv(frame->hd.stream_id);
      }
      break;
    default:
      break;
  }
  return 0;
}

std::int32_t session_data::on_request_recv(std::int32_t stream_id) {
  int fd;
  std::vector<nghttp2_nv> hdrs = {
    {MAKE_NV(":status", "200")}
  };

  char *rel_path;
  if(!is_stream_data_found(stream_id)) {
    std::cout << "stream data not present for stream_id:" << stream_id << std::endl;
    return 0;
  }

  auto &stream_data = get_stream_data(stream_id);
  if(stream_data.request_path().empty()) {
    #if 0
    if(error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    #endif
    std::cout <<"fn:" << __pretty_function__ <<" request_path is empty" << std::endl;
    return 0;
  }

  std::cout <<"peer:"<< m_client_addr <<" GET " << stream_data.request_path() << std::endl;
  std::cout <<"Response yet to be sent" << std::endl;
#if 0
  if(!check_path(stream_data->request_path)) {
    if(error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    return 0;
  }

  for(rel_path = stream_data->request_path; *rel_path == '/'; ++rel_path)
    ;
  fd = open(rel_path, O_RDONLY);

  if(fd == -1) {
    if(error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    return 0;
  }
  stream_data->fd = fd;

  if(send_response(session, stream_data->stream_id, hdrs, ARRLEN(hdrs), fd) != 0) {
    close(fd);
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }
#endif
  if(send_response(m_session_p.get(), stream_data.stream_id, hdrs, ARRLEN(hdrs), fd) != 0) {
    std::cout << "fn:" << __pretty_function__ <<" line:" << __LINE__ << " unable to send response" <<
                 " for stream-id:" << stream_data.stream_id << std::endl; 
  }
  return 0;
}

int session_data::on_stream_close_callback(nghttp2_session *ng_session,
                       int32_t stream_id,
                       uint32_t error_code,
                       void *user_data) {
  session_data *sess_data = static_cast<session_data*>(user_data);
  sess_data->delete_stream_data(stream_id);
  std::cout <<"fn:" << __pretty_function__ << " dtream_id:"<< stream_id << " closed" << std::endl;
  (void)ng_session;
  (void)error_code;
  return 0;
}

int session_data::on_header_callback(nghttp2_session *ng_session,
                       const nghttp2_frame *frame, const uint8_t *name,
                       size_t namelen, const uint8_t *value,
                       size_t valuelen, uint8_t flags,
                       void *user_data) {

  session_data *sess_data = static_cast<session_data*>(user_data);
  std::string PATH(":path");
  std::string name_str(reinterpret_cast<char*>(name), namelen);
  std::string value_str(reinterpret_cast<value>, valuelen);

  (void)flags;

  switch(frame->hd.type) {
    case NGHTTP2_HEADERS:
      if(frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
        break;
      }
      /*
      http2_stream_data *stream_data;
      stream_data =
        nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
      if(!stream_data || stream_data->request_path) {
        break;
      }*/

      stream_data& strm_data = sess_data->get_stream_data(frame->hd.stream_id);
      if(name_str.length() == PATH.length() && PATH == name_str) {
        auto end = value_str.find('?');
        if(end != std::string::npos) {
          auto path = value_str(0, end);
          stream_data.request_path(percent_decode());
        }
      }
      break;
    }
  return 0;
}

/**
 * @brief HEADERS frame is exchanged to create a stream for a given
 * TCP connection.
 * */

int session_data::on_begin_headers_callback(nghttp2_session *ng_session,
                       const nghttp2_frame *frame,
                       void *user_data) {
  session_data *sess_data = (session_data *)user_data;

  if(frame->hd.type != NGHTTP2_HEADERS ||
    frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
    return 0;
  }

  sess_data->create_stream_data(frame->hd.stream_id);
  std::cout <<"fn:" << __pretty_function__ <<" stream_id:" << frame->hd.stream_id << " created successfully"
  <<std::endl;
  /*
  nghttp2_session_set_stream_user_data(session, frame->hd.stream_id,
    session_data);*/

  return 0;
}

#endif
