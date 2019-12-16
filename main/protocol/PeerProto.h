/**
 * Peer-to-peer protocol.
 *
 * Copyright 2019 University Corporation for Atmospheric Research. All Rights
 * reserved. See file "COPYING" in the top-level source-directory for usage
 * restrictions.
 *
 *        File: PeerProto.h
 *  Created on: Nov 4, 2019
 *      Author: Steven R. Emmerson
 */

#ifndef MAIN_PROTOCOL_PEERPROTO_H_
#define MAIN_PROTOCOL_PEERPROTO_H_

#include "hycast.h"
#include "PortPool.h"
#include "Socket.h"

#include <memory>

namespace hycast {

/**
 * Interface for an observer of a PeerProto.
 */
class PeerProtoObs
{
public:
    virtual ~PeerProtoObs() noexcept
    {}

    /**
     * Handles a notice of an available chunk.
     *
     * @param[in] chunkId  Chunk identifier
     */
    virtual void acceptNotice(ChunkId chunkId) =0;

    /**
     * Handles a request for a chunk.
     *
     * @param[in] chunkId  Chunk identifier
     */
    virtual void acceptRequest(ChunkId chunkId) =0;

    /**
     * Accepts product-information from the remote peer.
     *
     * @param[in] prodInfo   Product-information
     * @retval    `true`     Accepted
     * @retval    `false`    Previously accepted
     */
    virtual void accept(const ProdInfo& prodInfo) =0;

    /**
     * Accepts a data-segment from the remote peer.
     *
     * @param[in] seg        Data-segment
     * @retval    `true`     Segment was accepted
     * @retval    `false`    Segment was previously accepted
     */
    virtual void accept(TcpSeg& seg) =0;
};

class PeerProto
{
    class                 Impl;
    std::shared_ptr<Impl> pImpl;

    PeerProto(Impl* impl);

public:
    /**
     * Server-side construction.
     *
     * @param[in]     sock      Server's listening TCP socket
     * @param[in,out] portPool  Pool of port numbers for transient servers
     * @cancellationpoint       Yes
     */
    PeerProto(
            TcpSock&  sock,
            PortPool& portPool);

    /**
     * Client-side construction.
     *
     * @param[in] rmtSrvrAddr  Socket address of remote peer-server
     * @cancellationpoint      Yes
     */
    PeerProto(const SockAddr& rmtSrvrAddr);

    /**
     * Sets the corresponding local peer.
     *
     * @param[in] observer  Observes this instance
     * @return              Reference to this instance
     * @cancellationpoint   No
     */
    const PeerProto& set(PeerProtoObs* observer) const;

    /**
     * Returns the socket address of the remote peer.
     *
     * @return             Socket address of remote peer
     * @cancellationpoint  No
     */
    SockAddr getRmtAddr() const;

    /**
     * Returns the socket address of the local peer.
     *
     * @return             Socket address of local peer
     * @cancellationpoint  No
     */
    SockAddr getLclAddr() const;

    /**
     * Returns a string representation of this instance.
     *
     * @return             String representation of this instance
     * @cancellationpoint  No
     */
    std::string to_string() const;

    /**
     * Executes this instance.
     *
     * @throws SystemError   System error
     * @throws RuntimeError  Remote peer closed the connection
     * @throws LogicError    This method has already been called
     */
    void operator()() const;

    /**
     * Halts execution of this instance by shutting-down the connection with the
     * remote peer.
     */
    void halt() const;

    /**
     * Notifies the remote peer of available product information.
     *
     * @param[in] prodIndex  Product index
     * @cancellationpoint    Yes
     */
    void notify(ProdId prodIndex) const;

    /**
     * Notifies the remote peer of an available data-segment.
     *
     * @param[in] id       Data-segment ID
     * @cancellationpoint  Yes
     */
    void notify(const SegId& id) const;

    /**
     * Requests information on a product from the remote peer.
     *
     * @param[in] prodId     Product identifier
     * @cancellationpoint    Yes
     */
    void request(ProdId prodId) const;

    /**
     * Requests a data-segment from the remote peer.
     *
     * @param[in] segId      Segment identifier
     * @cancellationpoint    Yes
     */
    void request(SegId segId) const;

    /**
     * Sends product-information to the remote peer.
     *
     * @param[in] info     Product-information
     * @cancellationpoint  Yes
     */
    void send(const ProdInfo& info) const;

    /**
     * Sends a data-segment to the remote peer.
     *
     * @param[in] seg      Data-segment
     * @cancellationpoint  Yes
     */
    void send(const MemSeg& seg) const;
};

} // namespace

#endif /* MAIN_PROTOCOL_PEERPROTO_H_ */
