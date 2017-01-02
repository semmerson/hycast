/**
 * This file declares a handle class for a socket.
 *
 * Copyright 2016 University Corporation for Atmospheric Research. All rights
 * reserved. See the file COPYIING in the top-level source-directory for
 * licensing conditions.
 *
 *   @file: Socket.h
 * @author: Steven R. Emmerson
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <sys/socket.h>

namespace hycast {

class SocketImpl; // Forward declaration of implementation

class Socket {
protected:
    std::shared_ptr<SocketImpl> pImpl;
    /**
     * Constructs from a socket implementation.
     * @param[in] impl  The implementation
     */
    explicit Socket(SocketImpl* impl);
public:
    /**
     * Constructs from nothing.
     * @throws std::bad_alloc if required memory can't be allocated
     * @exceptionsafety Strong
     */
    explicit Socket();
    /**
     * Constructs from a BSD socket and the number of SCTP streams. Only do this
     * once per socket because the destructor might close the socket.
     * @param[in] sd                  Socket descriptor
     * @param[in] numStreams          Number of SCTP streams
     * @throws std::invalid_argument  `sock < 0 || numStreams > UINT16_MAX`
     * @throws std::system_error      Socket couldn't be configured
     * @throws std::system_error      `getpeername(sd)` failed
     * @see Socket::~Socket()
     * @see Socket::operator=(Socket& socket)
     * @see Socket::operator=(Socket&& socket)
     */
    Socket(
            const int      sd,
            const uint16_t numStreams = 1);
    /**
     * Constructs from a shared pointer to a socket implementation.
     * @param[in] sptr  Shared pointer to implementation
     */
    explicit Socket(std::shared_ptr<SocketImpl> sptr);
    /**
     * Returns the number of SCTP streams.
     * @return the number of SCTP streams
     */
    uint16_t getNumStreams() const;
    /**
     * Returns the Internet socket address of the remote end.
     * @return Internet socket address of the remote end
     */
    const InetSockAddr& getRemoteAddr();
    /**
     * Indicates if this instance equals another.
     * @param[in] that  Other instance
     * @retval `true`   This instance equals the other
     * @retval `false`  This instance doesn't equal the other
     * @exceptionsafety Nothrow
     */
    bool operator==(const Socket& that) const noexcept;
    /**
     * Returns a string representation of this instance's socket.
     * @return String representation of this instance's socket
     * @throws std::bad_alloc if required memory can't be allocated
     * @exceptionsafety Strong
     * @threadsafety    Safe
     */
    std::string to_string() const;
    /**
     * Sends a message.
     * @param[in] streamId  SCTP stream number
     * @param[in] msg       Message to be sent
     * @param[in] len       Size of message in bytes
     * @throws std::system_error if an I/O error occurred
     * @exceptionsafety Basic
     * @threadsafety    Compatible but not safe
     */
    void send(
            const unsigned streamId,
            const void*    msg,
            const size_t   len) const;
    /**
     * Sends a message.
     * @param[in] streamId  SCTP stream number
     * @param[in] iovec     Vector comprising message to send
     * @param[in] iovcnt    Number of elements in `iovec`
     */
    void sendv(
            const unsigned streamId,
            struct iovec*  iovec,
            const int      iovcnt) const;
    /**
     * Returns the SCTP stream number of the current message. Waits for the
     * message if necessary. The message is left in the socket's input buffer.
     * @returns SCTP stream number
     * @throws std::system_error if an I/O error occurs
     * @exceptionsafety Basic
     */
    unsigned getStreamId() const;
    /**
     * Returns the size, in bytes, of the current SCTP message. Waits for the
     * message if necessary. The message is left in the socket's input buffer.
     * @returns Size of message in bytes. Will equal 0 when socket is closed by
     *          remote peer.
     * @throws std::system_error if an I/O error occurs
     * @exceptionsafety Basic
     */
    uint32_t getSize() const;
    /**
     * Receives a message.
     * @param[out] msg     Receive buffer
     * @param[in]  len     Size of receive buffer in bytes
     * @param[in]  flags   Type of message reception. Logical OR of zero or
     *                     more of
     *                       - `MSG_OOB`  Requests out-of-band data
     *                       - `MSG_PEEK` Peeks at the incoming message
     * @throws std::system_error on I/O failure or if number of bytes read
     *     doesn't equal `len`
     */
    void recv(
            void*        msg,
            const size_t len,
            const int    flags = 0) const;
    /**
     * Receives a message.
     * @param[in] iovec     Vector comprising message to receive
     * @param[in] iovcnt    Number of elements in `iovec`
     * @param[in] flags     Type of message reception. Logical OR of zero or
     *                      more of
     *                      - `MSG_OOB`  Requests out-of-band data
     *                      - `MSG_PEEK` Peeks at the incoming message
     * @throws std::system_error if an I/O error occurs
     * @exceptionsafety Basic
     * @threadsafety Safe
     */
    void recvv(
            struct iovec*  iovec,
            const int      iovcnt,
            const int      flags = 0) const;
    /**
     * Indicates if this instance has a current message.
     * @retval true   Yes
     * @retval false  No
     */
    bool hasMessage() const;
    /**
     * Discards the current message.
     * @exceptionsafety Basic guarantee
     * @threadsafety    Thread-compatible but not thread-safe
     */
    void discard() const;
    /**
     * Closes the underlying BSD socket.
     * @exceptionsafety Nothrow
     * @threadsafety    Compatible but not safe
     */
    void close() const;
};

} // namespace

#endif
