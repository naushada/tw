#ifndef __http2_handler_cpp__
#define __http2_handler_cpp__

#include <unistd.h>
#include "http2_handler.hpp"

int http2_handler::handle_event(const short events) {
  std::cout <<"fn:" << __func__ << ":" <<__LINE__ << " events:" << std::to_string(events) << std::endl;
  return 0;
}

std::int32_t http2_handler::tx(const std::uint8_t* data, ssize_t len) {

  ssize_t offset = 0;
  while(offset != len) {
    auto ret = write(handle(), data + offset, len - offset);
    if(ret < 0) {
      std::cout <<"fn:" << __func__ << ":" << __LINE__ << " could sent only bytes:" << offset << std::endl;
      return -1;
    }
    offset += ret;
  }

  return offset;
}

std::int32_t http2_handler::send_pending_data_to_peer() {
  const std::uint8_t *data_p = nullptr;
  ssize_t len = -1;
  while((len = nghttp2_session_mem_send(get_nghttp2_session(), &data_p)) > 0) {
    auto sent_bytes = tx(data_p, len);
    std::cout <<"fn:" << __func__ << ":" << __LINE__ << " sent nbytes:" << sent_bytes << " to peer" << std::endl;
  }

  return len;
}

/**
 * @brief This is the entry function for nghttp2 library.Feed received data into
 *        nghttp2 library for heavy lifting and decoding HEADER or DATA Frame of
 *        HTTP2 protocol.
 */
int http2_handler::process_request_from_peer(const std::int32_t& handle, const std::string& in) {
  std::int64_t readlen;

  // m_ctx_p holds all the registered callback (done in init). This will
  // decode the in.data into http2 frames and calls respective callback to deliver to
  // http2_handler.
  ssize_t offset = 0;
  ssize_t len = in.length();
  while(offset != len) {
    readlen = nghttp2_session_mem_recv(m_ctx_p.get(), reinterpret_cast<const std::uint8_t *>(in.data() + offset), len-offset);
    if(readlen < 0) {
      std::cout << "fn:" << __func__ <<" line:" << __LINE__ << 
                   " Fatal error:" << nghttp2_strerror((std::int32_t)readlen) << std::endl;
      return -1;
    }
    offset += readlen;
  }

  std::cout <<"fn:" << __PRETTY_FUNCTION__ << " feed nbytes:"<< offset <<" to nghttp2 library for processing HTTP2" << std::endl;
  
  int rv;
  rv = nghttp2_session_send(m_ctx_p.get());
  if (rv != 0) {
    std::cout <<"Fatal error:" << nghttp2_strerror(rv) << std::endl;
    return -1;
  }

  return readlen;
}

// new client is connected to this server
void http2_handler::handle_new_connection(const int& handle, const std::string& addr) {
  m_handle = handle;
  m_client_addr = addr;
  std::cout <<"fn:" << __func__ << ":"<< __LINE__ <<" handle:" << m_handle << std::endl;


  nghttp2_settings_entry iv[1] = {
      {NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 100}};
  int rv;

  rv = nghttp2_submit_settings(m_ctx_p.get(), NGHTTP2_FLAG_NONE, iv,
                               ARRLEN(iv));
  if (rv != 0) {
    std::cout <<"fn:" << __func__ <<":"<< __LINE__ << "Fatal error:" << nghttp2_strerror(rv);
    //return -1;
  }
}

void http2_handler::handle_connection_close(std::int32_t handle) {
  std::cout <<"fn:" << __func__ <<":"<<__LINE__ <<" connection is closed" << std::endl;
}

ssize_t http2_handler::send_callback2(nghttp2_session *ng_session,
                                 const uint8_t *data, size_t length,
                                 int flags, void *user_data) {
  http2_handler *ctx_p = static_cast<http2_handler*>(user_data);

  //tx(handle(), data, length);
  auto ret = ctx_p->tx(data, length);
  std::cout << "fn:"<< __PRETTY_FUNCTION__ << ":" << __LINE__ << " flags:"<< std::to_string(flags) <<" sent a packet of ret:" 
            << ret << " length:" << length <<" on-handle:"<< ctx_p->handle()<< std::endl;
  return(ret);
}

// The callback function for receiving data chunks
std::int32_t http2_handler::on_data_chunk_recv_callback(nghttp2_session *session, uint8_t flags, int32_t stream_id,
                            const uint8_t *data, size_t len, void *user_data) {
  http2_handler *ctx_p = static_cast<http2_handler*>(user_data);
  if(ctx_p->is_stream_data_found(stream_id)) {
    auto it = ctx_p->get_stream_data(stream_id);
    if(it != ctx_p->get_stream_data().end()) {
      it->app_data(data, len);
      std::cout <<"fn:"<<__PRETTY_FUNCTION__ <<":" << __LINE__ << " path:"<< it->request_path() << " data:" << it->app_data() << std::endl;
      
    }
  }
  
  return 0;
}

// This function is invoked by nghttp2 once either HEADER or DATA FRAME is received completly.
std::int32_t http2_handler::on_frame_recv_callback(nghttp2_session *ng_session,
                       const nghttp2_frame *frame, 
                       void *user_data) {
  http2_handler *ctx_p = static_cast<http2_handler*>(user_data);
  switch(frame->hd.type) {
    case NGHTTP2_DATA:
    {
      std::cout << "fn:" << __PRETTY_FUNCTION__ <<":" << __LINE__ << "complete data-frame:" << std::to_string(frame->hd.type) << std::endl;
      nghttp2_nv hdrs[] = {MAKE_NV(":status", "200")}; 
      auto rv = nghttp2_submit_response(ng_session, frame->hd.stream_id, hdrs, ARRLEN(hdrs), NULL);
      if (rv != 0) {
      }
    }
    break;
    case NGHTTP2_HEADERS:
      std::cout << "fn:" << __PRETTY_FUNCTION__ <<":" << __LINE__ << "complete header-frame:" << std::to_string(frame->hd.type) << std::endl;
      /* Check that the client request has finished */
      if(frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
        if(!ctx_p->is_stream_data_found(frame->hd.stream_id)) {
          std::cout <<"fn:"<<__func__ <<":" <<__LINE__ <<" stream-id:" << std::to_string(frame->hd.stream_id) << " not found" << std::endl;
          return 0;
        }

        /* For DATA and HEADERS frame, this callback may be called after
           on_stream_close_callback. Check that stream still alive. */
        auto it = ctx_p->get_stream_data(frame->hd.stream_id);
        if(it != ctx_p->get_stream_data().end()) { 
          return ctx_p->on_request_recv(frame->hd.stream_id);
        }
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

  auto it = get_stream_data(stream_id);
  if(it == get_stream_data().end() || it->request_path().empty()) {
    #if 0
    if(error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    #endif
    std::cout <<"fn:" << __func__ <<" request_path is empty" << std::endl;
    return 0;
  }

  std::cout <<"peer:"<< m_client_addr <<" GET " << it->request_path() << std::endl;
  std::cout <<"Response yet to be sent" << std::endl;
  if(/*send_response(m_session_p.get(), strm_data.stream_id(), hdrs, ARRLEN(hdrs), fd) != 0*/1) {
    std::cout << "fn:" << __func__ <<" line:" << __LINE__ << " unable to send response" <<
                 " for stream-id:" << it->stream_id() << std::endl;
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
  std::cout <<"fn:" << __func__ << " stream_id:"<< stream_id << " is closed" << std::endl;
  (void)ng_session;
  (void)error_code;
  return 0;
}

/**
 * @brief This function is called by nghttp2 library to
 * deliver the header's fields to application
 */
std::int32_t http2_handler::on_header_callback(nghttp2_session *ng_session,
                       const nghttp2_frame *frame, const std::uint8_t *name,
                       size_t namelen, const std::uint8_t *value,
                       size_t valuelen, std::uint8_t flags,
                       void *user_data) {

  http2_handler *ctx_p = static_cast<http2_handler*>(user_data);
  std::string PATH(":path");
  std::string name_str(reinterpret_cast<const char*>(name), namelen);
  std::string value_str(reinterpret_cast<const char *>(value), valuelen);

  std::cout <<"fn:"<< __PRETTY_FUNCTION__ << ":" << __LINE__ << " flags:" << std::to_string(flags) 
            << "frame-type:" <<std::to_string(frame->hd.type) 
            << std::endl;

  switch(frame->hd.type) {
    case NGHTTP2_HEADERS:
      if(frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
        break;
      }

      auto it = ctx_p->get_stream_data(frame->hd.stream_id);
      if(name_str.length() == PATH.length() && PATH == name_str) {
        std::cout <<"matched :path" << std::endl;
        auto end_pos = value_str.find('?');
        if(end_pos != std::string::npos) {
          auto path = value_str.substr(0, end_pos);
          it->request_path(ctx_p->percent_decode(path));
        } else if(it != ctx_p->get_stream_data().end()) {
          it->request_path(value_str);
          std::cout << "This is the path:" << it->request_path() << " value_str:" << value_str << std::endl;
        }
      }
      std::cout <<"fn:" << __func__ << ":" << __LINE__ << " name-str:"<< name_str <<" value-str:"<< value_str << " path:" 
                << it->request_path() << std::endl;
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
  std::cout << "fn:" << __func__ <<" frame-type:" <<std::to_string(frame->hd.type) << " stream-id:" << std::to_string(frame->hd.stream_id) 
            << " created successfully"<< std::endl;
  return 0;
}

#endif
