/**
 * Copyright 2016 University Corporation for Atmospheric Research. All rights
 * reserved. See the file COPYING in the top-level source-directory for
 * licensing conditions.
 *
 *   @file: InetAddr_test.cpp
 * @author: Steven R. Emmerson
 *
 * This file tests class `InetAddr`.
 */

#include "InetAddr.h"

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <string.h>
#include <sys/socket.h>

namespace {

// The fixture for testing class InetAddr.
class InetAddrTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  InetAddrTest()
  {
  }

  virtual ~InetAddrTest() {
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

  // Objects declared here can be used by all tests in the test case for
  // SockAddrInet.
};

static const char* IPV4_ADDR1 = "128.117.140.56";
static const char* IPV4_ADDR2 = "128.117.140.57";

static const char* IPV6_ADDR1 = "2001:db8::ff00:42:8329";
static const char* IPV6_ADDR2 = "2001:db8::ff00:42:8330";

static const char* HOSTNAME = "zero.unidata.ucar.edu";

// Tests default construction
TEST_F(InetAddrTest, DefaultConstruction) {
    hycast::InetAddr addr1;
    hycast::InetAddr addr2;
}

// Tests construction from an IPv4 address string
TEST_F(InetAddrTest, ConstructionFromIPv4String) {
    hycast::InetAddr addr1{IPV4_ADDR1};
    EXPECT_STREQ(IPV4_ADDR1, addr1.to_string().data());
}

// Tests construction from an IPv6 address string
TEST_F(InetAddrTest, ConstructionFromIPv6String) {
    hycast::InetAddr addr2{IPV6_ADDR1};
    EXPECT_STREQ(IPV6_ADDR1, addr2.to_string().data());
}

// Tests construction from a hostname
TEST_F(InetAddrTest, ConstructionFromHostname) {
    hycast::InetAddr addr1{HOSTNAME};
    EXPECT_STREQ(HOSTNAME, addr1.to_string().data());
}

// Tests copy construction
TEST_F(InetAddrTest, CopyConstruction) {
    hycast::InetAddr addr1{IPV4_ADDR1};
    hycast::InetAddr addr2{addr1};
    EXPECT_STREQ(IPV4_ADDR1, addr2.to_string().data());

    hycast::InetAddr addr3{IPV6_ADDR1};
    hycast::InetAddr addr4{addr3};
    EXPECT_STREQ(IPV6_ADDR1, addr4.to_string().data());
}

// Tests copy assignment
TEST_F(InetAddrTest, CopyAssignment) {
    hycast::InetAddr addr1{IPV4_ADDR1};
    hycast::InetAddr addr2{IPV4_ADDR2};
    addr2 = addr1;
    EXPECT_STREQ(IPV4_ADDR1, addr2.to_string().data());

    hycast::InetAddr addr3{IPV6_ADDR1};
    hycast::InetAddr addr4{IPV6_ADDR2};
    addr4 = addr3;
    EXPECT_STREQ(IPV6_ADDR1, addr4.to_string().data());
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}