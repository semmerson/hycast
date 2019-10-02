/**
 * This file tests class `Codec`.
 *
 * Copyright 2017 University Corporation for Atmospheric Research. All rights
 * reserved. See file "COPYING" in the top-level source-directory for usage
 * restrictions.
 *
 *       File: Codec_test.cpp
 * Created On: May 21, 2019
 *     Author: Steven R. Emmerson
 */
#include "config.h"

#include "Codec.h"

#include <condition_variable>
#include <gtest/gtest.h>
#include <cstdio>
#include <mutex>
#include <thread>

namespace {

/// The fixture for testing class `Codec`
class CodecTest : public ::testing::Test
{
protected:
    std::mutex              mutex;
    std::condition_variable cond;
    bool                    srvrReady;

    // You can remove any or all of the following functions if its body
    // is empty.

    CodecTest()
        : mutex{}
        , cond{}
        , srvrReady{false}
    {
        // You can do set-up work for each test here.
    }

    virtual ~CodecTest()
    {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    virtual void SetUp()
    {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    virtual void TearDown()
    {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Objects declared here can be used by all tests in the test case for Error.

public:
    void runServer(hycast::SrvrSock& srvrSock)
    {
        {
            std::lock_guard<decltype(mutex)> lock{mutex};
            srvrReady = true;
            cond.notify_one();
        }

        hycast::Socket      sock{srvrSock.accept()};
        hycast::StreamCodec codec(sock);

        for (;;) {
            uint8_t bytes[1500];

            size_t nbytes = codec.decode(bytes, sizeof(bytes));
            codec.encode(bytes, nbytes);
        }
    }
};

// Tests default construction
TEST_F(CodecTest, DefaultConstruction)
{
    hycast::StreamCodec codec();
}

// Tests scalar serialization
TEST_F(CodecTest, ScalarSerialization)
{
    hycast::SockAddr srvrAddr("localhost:38800");
    hycast::SrvrSock srvrSock(srvrAddr);
    std::thread      srvrThread([this,&srvrSock](){runServer(srvrSock);});

    {
        /*
         * The following is necessary because `ClntSock` constructor throws if
         * `::connect()` is called before `::listen()`
         */
        std::unique_lock<decltype(mutex)> lock{mutex};
        while (!srvrReady)
            cond.wait(lock);
    }

    hycast::ClntSock    clntSock(srvrAddr);
    hycast::StreamCodec codec(clntSock);

    ::fprintf(stderr, "ScalarSerialization(): uint16_t\n");
    uint16_t          writeint16 = 0x0102;
    codec.encode(writeint16);
    uint16_t readint16 = 0;
    codec.decode(readint16);
    EXPECT_EQ(writeint16, readint16);

    ::fprintf(stderr, "ScalarSerialization(): ChunkId\n");
    hycast::ChunkId writeChunkId{0x0102030405060708};
    codec.encode(writeChunkId);
    hycast::ChunkId readChunkId;
    codec.decode(readChunkId);
    EXPECT_EQ(writeChunkId.id, readChunkId.id);

    ::pthread_cancel(srvrThread.native_handle());
    srvrThread.join();
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
