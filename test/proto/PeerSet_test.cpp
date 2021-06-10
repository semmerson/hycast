#include "config.h"

#include "P2pNode.h"
#include "PeerSet.h"
#include "logging.h"

#include <condition_variable>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>

namespace {

/// The fixture for testing class `PeerSet`
class PeerSetTest : public ::testing::Test, public hycast::P2pNode
{
protected:
    typedef enum {
        INIT              =    0,
        LISTENING         =  0x1,
        PROD_NOTICE_RCVD  =  0x2,
        SEG_NOTICE_RCVD   =  0x4,
        PROD_REQUEST_RCVD = 0x08,
        SEG_REQUEST_RCVD  = 0x10,
        PROD_INFO_RCVD    = 0x20,
        SEG_RCVD          = 0x40,
        DONE = LISTENING         |
               PROD_NOTICE_RCVD  |
               SEG_NOTICE_RCVD   |
               PROD_REQUEST_RCVD |
               SEG_REQUEST_RCVD  |
               PROD_INFO_RCVD    |
               SEG_RCVD
    } State;
    State                   state;
    hycast::SockAddr        pubAddr;
    std::mutex              mutex;
    std::condition_variable cond;
    hycast::ProdIndex       prodIndex;
    hycast::ProdSize        prodSize;
    hycast::SegSize         segSize;
    hycast::ProdInfo        prodInfo;
    hycast::DataSegId       segId;
    char                    memData[hycast::DataSeg::CANON_DATASEG_SIZE];
    hycast::DataSeg         dataSeg;
    int                     pubPathNoticeCount;
    int                     prodInfoNoticeCount;
    int                     dataSegNoticeCount;
    int                     prodInfoRequestCount;
    int                     dataSegRequestCount;
    int                     prodInfoCount;
    int                     dataSegCount;

    static const int        NUM_SUBSCRIBERS = 10;

    PeerSetTest()
        : state{INIT}
        , pubAddr{"localhost:38800"}
        , mutex{}
        , cond{}
        , prodIndex{1}
        , prodSize{1000000}
        , segSize{sizeof(memData)}
        , prodInfo{prodIndex, "product", prodSize}
        , segId(prodIndex, sizeof(memData)) // Second data-segment
        , memData{}
        , dataSeg{segId, prodSize, memData}
        , pubPathNoticeCount(0)
        , prodInfoNoticeCount(0)
        , dataSegNoticeCount(0)
        , prodInfoRequestCount(0)
        , dataSegRequestCount(0)
        , prodInfoCount(0)
        , dataSegCount(0)
    {
        ::memset(memData, 0xbd, segSize);
    }

public:
    void orState(const State state)
    {
        this->state = static_cast<State>(this->state | state);
        cond.notify_all();
    }

    void waitForState(const State nextState)
    {
        std::unique_lock<decltype(mutex)> lock{mutex};
        while (state != nextState)
            cond.wait(lock);
    }

    // Publisher-side
    bool isPublisher() const override {
        LOG_TRACE;
        return true;
    }

    // Publisher-side
    bool isPathToPub() const override {
        LOG_TRACE;
        return true;
    }

    // Both sides
    void recvNotice(const hycast::PubPath notice, hycast::Peer peer)
            override
    {
        LOG_TRACE;
        std::lock_guard<std::mutex> guard{mutex};
        ++pubPathNoticeCount;
    }

    // Subscriber-side
    void recvNotice(const hycast::ProdIndex notice, hycast::Peer peer)
            override
    {
        LOG_TRACE;
        EXPECT_EQ(notice, prodIndex);
        {
            std::lock_guard<std::mutex> guard{mutex};
            if (++prodInfoNoticeCount == NUM_SUBSCRIBERS)
                orState(PROD_NOTICE_RCVD);
        }
        peer.request(notice);
    }

    // Subscriber-side
    void recvNotice(const hycast::DataSegId& notice, hycast::Peer peer)
            override
    {
        LOG_TRACE;
        EXPECT_EQ(segId, notice);
        {
            std::lock_guard<std::mutex> guard{mutex};
            if (++dataSegNoticeCount == NUM_SUBSCRIBERS)
                orState(SEG_NOTICE_RCVD);
        }
        peer.request(notice);
    }

    // Publisher-side
    void recvRequest(const hycast::ProdIndex request, hycast::Peer peer)
            override
    {
        LOG_TRACE;
        EXPECT_TRUE(prodIndex == request);
        {
            std::lock_guard<std::mutex> guard{mutex};
            if (++prodInfoRequestCount == NUM_SUBSCRIBERS)
                orState(PROD_REQUEST_RCVD);
        }
        peer.send(prodInfo);
    }

    // Publisher-side
    void recvRequest(const hycast::DataSegId& request, hycast::Peer peer)
            override
    {
        LOG_TRACE;
        EXPECT_EQ(segId, request);
        {
            std::lock_guard<std::mutex> guard{mutex};
            if (++dataSegRequestCount == NUM_SUBSCRIBERS)
                orState(SEG_REQUEST_RCVD);
        }
        peer.send(dataSeg);
    }

    // Subscriber-side
    void recvData(const hycast::ProdInfo& data, hycast::Peer peer) override
    {
        LOG_TRACE;
        EXPECT_EQ(prodInfo, data);
        std::lock_guard<std::mutex> guard{mutex};
        if (++prodInfoCount == NUM_SUBSCRIBERS)
            orState(PROD_INFO_RCVD);
    }

    // Subscriber-side
    void recvData(const hycast::DataSeg& actualDataSeg, hycast::Peer peer)
            override
    {
        LOG_TRACE;
        ASSERT_EQ(segSize, actualDataSeg.size());
        EXPECT_EQ(0, ::memcmp(dataSeg.data(), actualDataSeg.data(), segSize));
        std::lock_guard<std::mutex> guard{mutex};
        if (++dataSegCount == NUM_SUBSCRIBERS)
            orState(SEG_RCVD);
    }

    void died(hycast::Peer peer) {
        //LOG_ERROR("Peer %s died", peer.to_string().data());
    }
    void reassigned(const hycast::ProdIndex  notice,
                    hycast::Peer             peer) {}
    void reassigned(const hycast::DataSegId& notice,
                    hycast::Peer             peer) {}

    void startPublisher(hycast::PeerSet& pubPeerSet)
    {
        hycast::TcpSrvrSock srvrSock(pubAddr);
        {
            std::lock_guard<std::mutex> guard{mutex};
            orState(LISTENING);
        }

        for (int i = 0; i < NUM_SUBSCRIBERS; ++i) {
            auto             pubSock = srvrSock.accept();
            auto             rmtAddr = pubSock.getRmtAddr().getInetAddr();
            hycast::InetAddr localhost("127.0.0.1");

            EXPECT_EQ(localhost, rmtAddr);

            hycast::Peer pubPeer{pubSock, *this};
            // Starts receiving and becomes ready to notify
            pubPeerSet.insert(pubPeer, true);
            ASSERT_EQ(i+1, pubPeerSet.size());
        }
    }
};

// Tests default construction
TEST_F(PeerSetTest, DefaultConstruction)
{
    hycast::PeerSet peerSet{*this};
}

// Tests data exchange
TEST_F(PeerSetTest, DataExchange)
{
    try {
        // Create and execute publisher
        hycast::PeerSet pubPeerSet{*this};
        std::thread     srvrThread{&PeerSetTest::startPublisher, this,
                std::ref(pubPeerSet)};

        waitForState(LISTENING);

        // Create and execute reception by subscribing peers on separate threads
        hycast::PeerSet subPeerSet{*this};
        for (int i = 0; i < NUM_SUBSCRIBERS; ++i) {
            hycast::Peer subPeer{pubAddr, *this};
            ASSERT_TRUE(subPeerSet.insert(subPeer)); // Starts reading
            ASSERT_EQ(i+1, subPeerSet.size());
        }

        ASSERT_TRUE(srvrThread.joinable());
        srvrThread.join();

        // Start an exchange
        pubPeerSet.notify(prodIndex);
        pubPeerSet.notify(segId);

        // Wait for the exchange to complete
        waitForState(DONE);
    }
    catch (const std::exception& ex) {
        LOG_ERROR(ex);
    }
}

}  // namespace

static void myTerminate()
{
    if (std::current_exception()) {
        LOG_FATAL("terminate() called with an active exception");
        try {
            std::rethrow_exception(std::current_exception());
        }
        catch (const std::exception& ex) {
            LOG_FATAL(ex);
        }
        catch (...) {
            LOG_FATAL("Exception is unknown");
        }
    }
    else {
        LOG_FATAL("terminate() called without an active exception");
    }
    abort();
}

int main(int argc, char **argv) {
  hycast::log_setName(::basename(argv[0]));
  //hycast::log_setLevel(hycast::LogLevel::TRACE);

  std::set_terminate(&myTerminate);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}