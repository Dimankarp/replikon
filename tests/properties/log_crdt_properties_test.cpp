#include "crdt/log.h"
#include "../test_utils.h"
#include <gtest/gtest.h>

using namespace replikon;

class LogCrdtPropertiesTest : public test::ReplikonTest {
protected:
  std::unique_ptr<Instance> createInstance(const std::string &name) {
    return test::createTestInstance(name);
  }
};

TEST_F(LogCrdtPropertiesTest, Idempotence) {
  auto alice = createInstance("alice");
  auto bob = createInstance("bob");

  alice->keysCrdt().addKey(bob->self().author, bob->self().pub_key);
  bob->keysCrdt().addKey(alice->self().author, alice->self().pub_key);

  auto &log_alice = alice->messagesCrdt();
  auto &log_bob = bob->messagesCrdt();

  log_alice.addNewMessage("alice", {"alice", 0, 100, "Message 1"});
  
  auto req = log_bob.getRequest(log_alice.getHeader());
  auto update = log_alice.getUpdate(req);

  log_bob.merge(update);
  auto header1 = log_bob.getHeader();

  log_bob.merge(update);
  auto header2 = log_bob.getHeader();

  EXPECT_EQ(header1, header2);
}

TEST_F(LogCrdtPropertiesTest, Commutativity) {
  auto alice = createInstance("alice");
  auto bob1 = createInstance("bob1");
  auto bob2 = createInstance("bob2");

  alice->keysCrdt().addKey(bob1->self().author, bob1->self().pub_key);
  alice->keysCrdt().addKey(bob2->self().author, bob2->self().pub_key);
  bob1->keysCrdt().addKey(alice->self().author, alice->self().pub_key);
  bob2->keysCrdt().addKey(alice->self().author, alice->self().pub_key);

  auto &log_alice = alice->messagesCrdt();
  auto &log_bob1 = bob1->messagesCrdt();
  auto &log_bob2 = bob2->messagesCrdt();

  log_alice.addNewMessage("alice", {"alice", 0, 100, "Msg A"});
  auto req_a = log_bob1.getRequest(log_alice.getHeader());
  auto update_a = log_alice.getUpdate(req_a);

  log_alice.addNewMessage("alice", {"alice", 0, 101, "Msg B"});
  
  auto req_b = log_bob1.getRequest(log_alice.getHeader());
  req_b["alice"][0].start += 1;
  req_b["alice"][0].len -= 1;
  auto update_b = log_alice.getUpdate(req_b);

  log_bob1.merge(update_a);
  log_bob1.merge(update_b);

  log_bob2.merge(update_b);
  log_bob2.merge(update_a);

  EXPECT_EQ(log_bob1.getHeader(), log_bob2.getHeader());
}

TEST_F(LogCrdtPropertiesTest, Associativity) {
  auto peer_a = createInstance("alice");
  auto peer_b = createInstance("bob");
  auto peer_c = createInstance("charlie");

  peer_a->keysCrdt().addKey(peer_c->self().author, peer_c->self().pub_key);
  peer_b->keysCrdt().addKey(peer_c->self().author, peer_c->self().pub_key);
  peer_c->keysCrdt().addKey(peer_a->self().author, peer_a->self().pub_key);
  peer_c->keysCrdt().addKey(peer_b->self().author, peer_b->self().pub_key);
  peer_b->keysCrdt().addKey(peer_a->self().author, peer_a->self().pub_key);
  peer_a->keysCrdt().addKey(peer_b->self().author, peer_b->self().pub_key);

  auto &log_a = peer_a->messagesCrdt();
  auto &log_b = peer_b->messagesCrdt();
  auto &log_c = peer_c->messagesCrdt();

  log_a.addNewMessage("alice", {"alice", 0, 100, "A"});
  log_b.addNewMessage("bob", {"bob", 0, 100, "B"});

  auto req_ca = log_c.getRequest(log_a.getHeader());
  auto update_a = log_a.getUpdate(req_ca);
  log_c.merge(update_a);

  auto req_cb = log_c.getRequest(log_b.getHeader());
  auto update_b = log_b.getUpdate(req_cb);
  log_c.merge(update_b);

  auto header_c_scenario1 = log_c.getHeader();

  auto peer_c_new = createInstance("charlie2");
  peer_c_new->keysCrdt().addKey(peer_a->self().author, peer_a->self().pub_key);
  peer_c_new->keysCrdt().addKey(peer_b->self().author, peer_b->self().pub_key);
  auto &log_c_new = peer_c_new->messagesCrdt();

  auto req_ba = log_b.getRequest(log_a.getHeader());
  auto update_a_for_b = log_a.getUpdate(req_ba);
  log_b.merge(update_a_for_b);

  auto req_cnew_b = log_c_new.getRequest(log_b.getHeader());
  auto update_ab_for_c = log_b.getUpdate(req_cnew_b);
  log_c_new.merge(update_ab_for_c);

  auto header_c_scenario2 = log_c_new.getHeader();

  EXPECT_EQ(header_c_scenario1, header_c_scenario2);
}