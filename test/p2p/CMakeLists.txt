include_directories(
        ${CMAKE_SOURCE_DIR}/main
        ${CMAKE_SOURCE_DIR}/main/p2p
        ${CMAKE_SOURCE_DIR}/main/misc
        ${CMAKE_SOURCE_DIR}/main/inet
)

add_executable(Peer_test Peer_test.cpp)
target_link_libraries(Peer_test hycast gtest)
add_test(Peer_test Peer_test)

#add_executable(NoticeQueue_test NoticeQueue_test.cpp)
#target_link_libraries(NoticeQueue_test hycast gtest)
#add_test(NoticeQueue_test NoticeQueue_test)

add_executable(PeerSet_test PeerSet_test.cpp)
target_link_libraries(PeerSet_test hycast gtest)
add_test(PeerSet_test PeerSet_test)

add_executable(Bookkeeper_test Bookkeeper_test.cpp)
target_link_libraries(Bookkeeper_test hycast gtest)
add_test(Bookkeeper_test Bookkeeper_test)