#include <gtest/gtest.h>
#include "test_utils.h"
#include "dao/message.h"

using namespace replikon;

using test::ReplikonTest;
TEST_F(ReplikonTest, MessageRetrievalEmpty) {
    auto instance = test::createTestInstance("alice");
    
    dao::MessagesDao msg_dao(instance->db());
    
    std::vector<Interval> intervals = {{0, 10}};
    auto msgs = msg_dao.getAllMessages("bob", intervals);
    ASSERT_TRUE(msgs.hasValue());
    ASSERT_TRUE(msgs.value().empty());
}

TEST_F(ReplikonTest, MessageInsertGetLamportAndGaps) {
    auto instance = test::createTestInstance("alice");
    dao::MessagesDao msg_dao(instance->db());
    
    std::string dummy_body = "test params";
    serde::Buffer dummy_sign;

    // Lamport initially starts at empty (should map to 1 based on COALESCE(MAX, 0) + 1 logic or similar)
    auto lamport_pre = msg_dao.getNextLamport("alice");
    ASSERT_TRUE(lamport_pre.hasValue());
    ASSERT_EQ(lamport_pre.value(), 1);

    // Insert 0, 1, 2,  and 4 (island at 4, gap at 3)
    msg_dao.insertMessage(SignedChatMeesage{{"alice", 0, 1000, dummy_body}, dummy_sign});
    msg_dao.insertMessage(SignedChatMeesage{{"alice", 1, 1001, dummy_body}, dummy_sign});
    msg_dao.insertMessage(SignedChatMeesage{{"alice", 2, 1002, dummy_body}, dummy_sign});
    msg_dao.insertMessage(SignedChatMeesage{{"alice", 4, 1004, dummy_body}, dummy_sign});

    // Check getting next lamport
    auto lamport_post = msg_dao.getNextLamport("alice");
    ASSERT_EQ(lamport_post.value(), 5);

    // Check headers returned correctly identifies gaps
    auto headers_res = msg_dao.getHeaders();
    ASSERT_TRUE(headers_res.hasValue());
    auto headers = headers_res.value();
    
    auto alice_headers = headers["alice"];
    ASSERT_EQ(alice_headers.size(), 2);
    
    // Island 1: 0, 1, 2 (start 0, len 3)
    ASSERT_EQ(alice_headers[0].start, 0);
    ASSERT_EQ(alice_headers[0].len, 3);
    
    // Island 2: 4, len 1
    ASSERT_EQ(alice_headers[1].start, 4);
    ASSERT_EQ(alice_headers[1].len, 1);

    // Check retrieving specific intervals (0 to 1, and 4 to 5)
    std::vector<Interval> req_intervals = {{0, 2}, {4, 2}};
    auto get_msgs = msg_dao.getAllMessages("alice", req_intervals);
    ASSERT_TRUE(get_msgs.hasValue());
    
    auto msgs_list = get_msgs.value();
    ASSERT_EQ(msgs_list.size(), 3); // Should retrieve 0, 1, and 4
    ASSERT_EQ(msgs_list[0].msg.lamport, 0);
    ASSERT_EQ(msgs_list[1].msg.lamport, 1);
    ASSERT_EQ(msgs_list[2].msg.lamport, 4);

    // Inserting ignored overlapping message
    auto res_ignore = msg_dao.insertMessage(SignedChatMeesage{{"alice", 0, 1000, dummy_body}, dummy_sign});
    ASSERT_EQ(res_ignore, db::SqliteResult::OK); // SQLite IGNORE doesn't error
}
