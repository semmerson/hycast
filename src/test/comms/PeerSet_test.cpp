/**
 * This file tests class `PeerSet`.
 *
 * Copyright 2017 University Corporation for Atmospheric Research. All rights
 * reserved. See the file COPYING in the top-level source-directory for
 * licensing conditions.
 *
 *   @file: PeerSet_test.cpp
 * @author: Steven R. Emmerson
 */

#include "PeerMsgRcvr.h"
#include "PeerSet.h"
#include "ClntSctpSock.h"
#include "SctpSock.h"
#include "HycastTypes.h"
#include "logging.h"
#include "ProdInfo.h"
#include "SrvrSctpSock.h"

#include <gtest/gtest.h>
#include <pthread.h>
#include <thread>
#include <unistd.h>

namespace {

// The fixture for testing class PeerSet.
class PeerSetTest : public ::testing::Test {
protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    PeerSetTest() {
        // You can do set-up work for each test here.
    }

    virtual ~PeerSetTest() {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    virtual void SetUp() {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    virtual void TearDown() {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    /**
     * Thread-safe peer-manager that discards everything.
     */
    class ClientMsgRcvr final : public hycast::PeerMsgRcvr {
        hycast::ProdInfo prodInfo;
        hycast::ChunkInfo chunkInfo;
    public:
        ClientMsgRcvr(
                const hycast::ProdInfo prodInfo,
                const hycast::ChunkInfo chunkInfo)
            : prodInfo{prodInfo}
            , chunkInfo{chunkInfo}
        {}
        void recvNotice(const hycast::ProdInfo& info) {}
        void recvNotice(const hycast::ProdInfo& info, hycast::Peer& peer) {
            EXPECT_EQ(prodInfo, info);
        }
        void recvNotice(const hycast::ChunkInfo& info, hycast::Peer& peer) {
            EXPECT_EQ(chunkInfo, info);
        }
        void recvRequest(const hycast::ProdIndex& index, hycast::Peer& peer) {
        }
        void recvRequest(const hycast::ChunkInfo& info, hycast::Peer& peer) {
        }
        void recvData(hycast::LatentChunk chunk) {}
        void recvData(hycast::LatentChunk chunk, hycast::Peer& peer) {
            chunk.discard();
        }
    };

    /**
     * Server that echos everything back to the client.
     */
    class Server {
        /**
         * Thread-safe peer-manager that echos everything back to the remote
         * peer.
         */
        class ServerMsgRcvr final : public hycast::PeerMsgRcvr {
            hycast::ProdInfo prodInfo;
        public:
            void recvNotice(const hycast::ProdInfo& info, hycast::Peer& peer) {
                prodInfo = info;
                peer.sendNotice(info);
            }
            void recvNotice(const hycast::ChunkInfo& info, hycast::Peer& peer) {
                peer.sendNotice(info);
            }
            void recvRequest(const hycast::ProdIndex& index, hycast::Peer& peer) {
                peer.sendRequest(index);
            }
            void recvRequest(const hycast::ChunkInfo& info, hycast::Peer& peer) {
                peer.sendRequest(info);
            }
            void recvData(hycast::LatentChunk latentChunk, hycast::Peer& peer) {
                const hycast::ChunkSize expectedSize =
                        prodInfo.getChunkSize(latentChunk.getIndex());
                char                    data[expectedSize];
                const size_t            actualSize =
                        latentChunk.drainData(data, expectedSize);
                EXPECT_EQ(expectedSize, actualSize);
                hycast::ActualChunk chunk{latentChunk.getInfo(), data};
                peer.sendData(chunk);
            }
        };
        std::thread thread;
        void runServer(hycast::SrvrSctpSock serverSock) {
            ServerMsgRcvr srvrMsgRcvr{};
            hycast::PeerSet peerSet{[](hycast::Peer&){}};
            for (;;) {
                try {
                    hycast::SctpSock sock{serverSock.accept()};
                    hycast::Peer   peer{srvrMsgRcvr, sock};
                    peerSet.tryInsert(peer);
                }
                catch (const std::exception& e) {
                    hycast::log_what(e);
                }
            }
        }
    public:
        Server(const hycast::InetSockAddr& serverSockAddr)
            : thread{}
        {
            hycast::SrvrSctpSock serverSock{serverSockAddr,
                hycast::Peer::getNumStreams()};
            thread = std::thread([=]{ runServer(serverSock); });
        }
        ~Server() {
            ::pthread_cancel(thread.native_handle());
            thread.join();
        }
    };

    hycast::Peer getClientPeer() {
        hycast::ClntSctpSock sock{serverSockAddr, hycast::Peer::getNumStreams()};
        return hycast::Peer(clntMsgRcvr, sock);
    }

    // Objects declared here can be used in all TEST_F tests
    hycast::InetSockAddr serverSockAddr{"127.0.0.1", 38800};
    hycast::ProdInfo     prodInfo{"product", 1, 100000};
    hycast::ChunkInfo    chunkInfo{prodInfo, 2};
    ClientMsgRcvr        clntMsgRcvr{prodInfo, chunkInfo};
};

// Tests default construction
TEST_F(PeerSetTest, DefaultConstruction) {
    hycast::PeerSet peerSet{[](hycast::Peer&){}};
}

// Tests construction with invalid argument
TEST_F(PeerSetTest, InvalidConstruction) {
    EXPECT_THROW(hycast::PeerSet peerSet([](hycast::Peer&){}, 0), std::invalid_argument);
}

// Tests inserting a peer and incrementing its value
TEST_F(PeerSetTest, IncrementPeerValue) {
    Server server{serverSockAddr};
    hycast::Peer     peer{getClientPeer()};
    hycast::PeerSet  peerSet{[](hycast::Peer&){}};
    EXPECT_EQ(hycast::PeerSet::InsertStatus::SUCCESS, peerSet.tryInsert(peer));
    peerSet.incValue(peer);
}

// Tests removing the worst peer from a 1-peer set
TEST_F(PeerSetTest, RemoveWorst) {
    Server server{serverSockAddr};
    hycast::Peer     peer1{getClientPeer()};
    hycast::PeerSet  peerSet{[](hycast::Peer&){}, 1, 0};
    EXPECT_EQ(hycast::PeerSet::InsertStatus::SUCCESS, peerSet.tryInsert(peer1));
    hycast::Peer worstPeer{};
    hycast::Peer peer2{getClientPeer()};
    EXPECT_EQ(hycast::PeerSet::InsertStatus::REPLACED,
            peerSet.tryInsert(peer2, &worstPeer));
    EXPECT_EQ(peer1, worstPeer);
}

// Tests inserting a peer and sending notices
TEST_F(PeerSetTest, PeerInsertionAndNotices) {
    Server server{serverSockAddr};
    hycast::Peer     peer{getClientPeer()};
    hycast::PeerSet  peerSet{[](hycast::Peer&){}};
    EXPECT_EQ(hycast::PeerSet::InsertStatus::SUCCESS, peerSet.tryInsert(peer));
    peerSet.sendNotice(prodInfo);
    peerSet.sendNotice(chunkInfo);
    ::sleep(1);
}

// Tests inserting the same peer twice
TEST_F(PeerSetTest, DuplicatePeerInsertion) {
    Server server{serverSockAddr};
    hycast::Peer     peer{getClientPeer()};
    hycast::PeerSet  peerSet{[](hycast::Peer&){}};
    EXPECT_EQ(hycast::PeerSet::InsertStatus::SUCCESS, peerSet.tryInsert(peer));
    EXPECT_EQ(hycast::PeerSet::InsertStatus::EXISTS, peerSet.tryInsert(peer));
}

// Tests inserting the same peer twice but by Internet socket address
TEST_F(PeerSetTest, DuplicatePeerInsertionByAddress) {
    Server server{serverSockAddr};
    hycast::Peer     peer{getClientPeer()};
    hycast::PeerSet  peerSet{[](hycast::Peer&){}};
    EXPECT_EQ(hycast::PeerSet::InsertStatus::SUCCESS, peerSet.tryInsert(peer));
    EXPECT_EQ(hycast::PeerSet::InsertStatus::EXISTS,
            peerSet.tryInsert(peer.getRemoteAddr(), clntMsgRcvr, nullptr));
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
