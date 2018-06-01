////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Andreas Streichardt
////////////////////////////////////////////////////////////////////////////////

#include "Scheduler/SocketTcp.h"

using namespace arangodb;

size_t SocketTcp::write(basics::StringBuffer* buffer,
                        boost::system::error_code& ec) {
  MUTEX_LOCKER(guard, _lock);
  if (_encrypted) {
    return _sslSocket.write_some(boost::asio::buffer(buffer->begin(), buffer->length()), ec);
  } else {
    return _socket.write_some(boost::asio::buffer(buffer->begin(), buffer->length()), ec);
  }
}

void SocketTcp::asyncWrite(boost::asio::mutable_buffers_1 const& buffer,
                           AsyncHandler const& handler) {
  MUTEX_LOCKER(guard, _lock);
  if (_encrypted) {
    return boost::asio::async_write(_sslSocket, buffer, _sslSocketStrand.wrap(handler));
  } else {
    return boost::asio::async_write(_socket, buffer, handler);
  }
}

size_t SocketTcp::read(boost::asio::mutable_buffers_1 const& buffer,
                       boost::system::error_code& ec) {
  MUTEX_LOCKER(guard, _lock);
  if (_encrypted) {
    return _sslSocket.read_some(buffer, ec);
  } else {
    return _socket.read_some(buffer, ec);
  }
}

void SocketTcp::shutdown(boost::system::error_code& ec, bool closeSend, bool closeReceive) {
  MUTEX_LOCKER(guard, _lock);
  Socket::shutdown(ec, closeSend, closeReceive);
}

void SocketTcp::close(boost::system::error_code& ec) {
  MUTEX_LOCKER(guard, _lock);
  if (_socket.is_open()) {
    _socket.close(ec);
    if (ec && ec != boost::asio::error::not_connected) {
      LOG_TOPIC(DEBUG, Logger::COMMUNICATION)
          << "closing socket failed with: " << ec.message();
    }
  }
}

std::size_t SocketTcp::available(boost::system::error_code& ec) {
  MUTEX_LOCKER(guard, _lock);
  return static_cast<size_t>(_socket.available(ec));
}

void SocketTcp::asyncRead(boost::asio::mutable_buffers_1 const& buffer,
                          AsyncHandler const& handler) {
  MUTEX_LOCKER(guard, _lock);
  if (_encrypted) {
    return _sslSocket.async_read_some(buffer, _sslSocketStrand.wrap(handler));
  } else {
    return _socket.async_read_some(buffer, handler);
  }
}

void SocketTcp::shutdownReceive(boost::system::error_code& ec) {
  _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec);
}

void SocketTcp::shutdownSend(boost::system::error_code& ec) {
  _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
}
