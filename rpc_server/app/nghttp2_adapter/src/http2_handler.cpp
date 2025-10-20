#ifndef __http2_handler_cpp__
#define __http2_handler_cpp__

#include "http2_handler.hpp"

#if 0
// virtual methods for io_operations
int http2_handler::handle_event(const short events) {
  std::cout << "fn:" << __func__ << " the event:" << events << " is" << std::endl;
  if(events & BEV_EVENT_CONNECTED) {
    std::cout << "fn:" << __func__ << " peer is connected" << std::endl;
    #if 0
    //const unsigned char *alpn = NULL;
    //unsigned int alpnlen = 0;
    //SSL *ssl;

    fprintf(stderr, "%s connected\n", http2_handler->client_addr);

    ssl = bufferevent_openssl_get_ssl(http2_handler->bev);

    SSL_get0_alpn_selected(ssl, &alpn, &alpnlen);

    if (alpn == NULL || alpnlen != 2 || memcmp("h2", alpn, 2) != 0) {
      fprintf(stderr, "%s h2 is not negotiated\n", http2_handler->client_addr);
      delete_http2_http2_handler(http2_handler);
      return;
    }

    initialize_nghttp2_session(http2_handler);

    if(send_server_connection_header(http2_handler) != 0 ||
        session_send(http2_handler) != 0) {
      delete_http2_http2_handler(http2_handler);
      return;
    }

    return;
  #endif
  }

  if(events & BEV_EVENT_EOF) {
    std::cout << "fn:" << __func__<<" peer:"<< client_addr() << 
                 " for handle:" <<  handle() << " is closed"<<std::endl;
  } else if(events & BEV_EVENT_ERROR) {
    std::cout << "fn:" << __func__<<" peer:"<< client_addr() << 
                 " for handle:"<< handle() << " event Error"<<std::endl;
  } else if(events & BEV_EVENT_TIMEOUT) {
    std::cout << "fn:" << __func__<<" peer:"<< client_addr() << 
                 " for handle:" << handle() << " event timed out"<<std::endl;
  }
  //delete_http2_http2_handler(http2_handler);
  return 0;
}
#endif

int http2_handler::process_request_from_app(const std::int32_t& handle, const std::string& in) {
  std::int64_t readlen;

  // m_ctx_p holds all the registered callback (done in init). This will
  // decode the in.data into http2 frames and calls respective callback to deliver to
  // http2_handler.
  readlen = nghttp2_session_mem_recv(m_ctx_p.get(), reinterpret_cast<const std::uint8_t *>(in.data()), in.length());
  if(readlen < 0) {
    std::cout << "fn:" << __func__ <<" line:" << __LINE__ << 
                 " Fatal error:" << nghttp2_strerror((std::int32_t)readlen) << std::endl;
    return -1;
  }
  std::cout <<"fn:" << __func__ << " readlen:"<< readlen << std::endl;
/*
  if(evbuffer_drain(input, (size_t)readlen) != 0) {
    warnx("Fatal error: evbuffer_drain failed");
    return -1;
  }
*/

#if 0
  if(session_send(m_session_p.get()) != 0) {
    std::cout <<"line:"<< __LINE__ << " session_send failed" << std::endl;
    return -1;
  }
#endif

  return 0;
}

// new client is connected to this server
void http2_handler::handle_new_connection(const int& handle, const std::string& addr) {
  m_handle = handle;
  m_client_addr = addr;
}

void http2_handler::handle_connection_close(std::int32_t handle) {
}

ssize_t http2_handler::send_callback2(nghttp2_session *ng_session,
                                 const uint8_t *data, size_t length,
                                 int flags, void *user_data) {
  http2_handler *ctx_p = static_cast<http2_handler*>(user_data);
#if 0
  struct bufferevent *bev = sess_data->get_bufferevent();
  /* Avoid excessive buffering in server side. */
  if(evbuffer_get_length(bufferevent_get_output(bev)) >= OUTPUT_WOULDBLOCK_THRESHOLD) {
    return NGHTTP2_ERR_WOULDBLOCK;
  }

  bufferevent_write(bev, data, length);
#endif
  //tx(handle(), data, length);
  return(length);
}

std::int32_t http2_handler::on_frame_recv_callback(nghttp2_session *ng_session,
                       const nghttp2_frame *frame, 
                       void *user_data) {
  http2_handler *ctx_p = static_cast<http2_handler*>(user_data);
  switch(frame->hd.type) {
    case NGHTTP2_DATA:
    case NGHTTP2_HEADERS:
      /* Check that the client request has finished */
      if(frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
        if(!ctx_p->is_stream_data_found(frame->hd.stream_id)) {
          return 0;
        }

        /* For DATA and HEADERS frame, this callback may be called after
           on_stream_close_callback. Check that stream still alive. */
        auto& stream_data = ctx_p->get_stream_data(frame->hd.stream_id);
        return ctx_p->on_request_recv(frame->hd.stream_id);
      }
      break;
    default:
      break;
  }
  return 0;
}

std::int32_t http2_handler::on_request_recv(std::int32_t stream_id) {
  //int fd;
  std::vector<nghttp2_nv> hdrs = {
    {MAKE_NV(":status", "200")}
  };

  //char *rel_path;
  if(!is_stream_data_found(stream_id)) {
    std::cout << "stream data not present for stream_id:" << stream_id << std::endl;
    return 0;
  }

  auto &strm_data = get_stream_data(stream_id);
  if(strm_data.request_path().empty()) {
    #if 0
    if(error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    #endif
    std::cout <<"fn:" << __func__ <<" request_path is empty" << std::endl;
    return 0;
  }

  std::cout <<"peer:"<< m_client_addr <<" GET " << strm_data.request_path() << std::endl;
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
  if(/*send_response(m_session_p.get(), strm_data.stream_id(), hdrs, ARRLEN(hdrs), fd) != 0*/1) {
    std::cout << "fn:" << __func__ <<" line:" << __LINE__ << " unable to send response" <<
                 " for stream-id:" << strm_data.stream_id() << std::endl;
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }
  return 0;
}

int http2_handler::on_stream_close_callback(nghttp2_session *ng_session,
                       std::int32_t stream_id,
                       std::uint32_t error_code,
                       void *user_data) {
  http2_handler *ctx_p = static_cast<http2_handler*>(user_data);
  ctx_p->delete_stream_data(stream_id);
  std::cout <<"fn:" << __func__ << " dtream_id:"<< stream_id << " closed" << std::endl;
  (void)ng_session;
  (void)error_code;
  return 0;
}

int http2_handler::on_header_callback(nghttp2_session *ng_session,
                       const nghttp2_frame *frame, const std::uint8_t *name,
                       size_t namelen, const std::uint8_t *value,
                       size_t valuelen, std::uint8_t flags,
                       void *user_data) {

  http2_handler *ctx_p = static_cast<http2_handler*>(user_data);
  std::string PATH(":path");
  std::string name_str(reinterpret_cast<const char*>(name), namelen);
  std::string value_str(reinterpret_cast<const char *>(value), valuelen);

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

      auto& strm_data = ctx_p->get_stream_data(frame->hd.stream_id);
      if(name_str.length() == PATH.length() && PATH == name_str) {
        auto end_pos = value_str.find('?');
        if(end_pos != std::string::npos) {
          auto path = value_str.substr(0, end_pos);
          strm_data.request_path(ctx_p->percent_decode(path));
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

int http2_handler::on_begin_headers_callback(nghttp2_session *ng_session,
                       const nghttp2_frame *frame,
                       void *user_data) {
  http2_handler *ctx_p = static_cast<http2_handler *>(user_data);

  if(frame->hd.type != NGHTTP2_HEADERS ||
    frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
    return 0;
  }

  ctx_p->create_stream_data(frame->hd.stream_id);
  std::cout << "fn:" << __func__ <<" stream_id:" << frame->hd.stream_id 
            << " created successfully"<< std::endl;
  /*
  nghttp2_session_set_stream_user_data(session, frame->hd.stream_id,
    http2_handler);*/

  return 0;
}

#endif
