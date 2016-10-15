/**
 * This file declares a connection between peers.
 *
 * Copyright 2016 University Corporation for Atmospheric Research. All rights
 * reserved. See the file COPYING in the top-level source-directory for
 * licensing conditions.
 *
 *   @file: PeerConnection.h
 * @author: Steven R. Emmerson
 */

#ifndef PEERCONNECTION_H_
#define PEERCONNECTION_H_

#include "ProdInfo.h"
#include "Socket.h"

#include <memory>

namespace hycast {

class PeerConnectionImpl; // Forward declaration

class PeerConnection final {
    std::shared_ptr<PeerConnectionImpl> pImpl;
public:
    /**
     * Constructs from a socket.
     * @param[in] sock  Socket
     */
    PeerConnection(Socket& sock);
    /**
     * Sends information about a product to the remote peer.
     * @param[in] prodInfo  Product information
     */
    void sendProdInfo(const ProdInfo& prodInfo);
};

} // namespace

#endif /* PEERCONNECTION_H_ */
