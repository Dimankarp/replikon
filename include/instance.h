#ifndef REPLIKON_INSTANCE_H
#define REPLIKON_INSTANCE_H

#include "crdt/keys.h"
#include "crdt/log.h"
#include "dao/key_value.h"
#include "dao/message.h"
#include "dao/security.h"
#include "security/provider.h"
#include "sqlite.h"
#include "types.h"
#include "utils.h"
#include <memory>
namespace replikon {

class Instance {
public:
  Instance(const std::string &db_path, SecurityUserInfo self)
      : _self{std::move(self)} {
    _db = std::make_shared<replikon::db::Sqlite>();
    auto res = _db->connect(db_path);
    REPLIKON_ASSERT(res == db::SqliteResult::OK);
    auto exp = _db->prepareStatement(replikon::INIT_MESSAGES);
    auto statement = std::move(exp.value());
    res = statement.step();
    REPLIKON_ASSERT(res == db::SqliteResult::OK);

    exp = _db->prepareStatement(replikon::INDEX_MESSAGES);
    statement = std::move(exp.value());
    res = statement.step();
    REPLIKON_ASSERT(res == db::SqliteResult::OK);

    exp = _db->prepareStatement(replikon::TEMP_SEARCH_INTERVALS);
    statement = std::move(exp.value());
    res = statement.step();
    REPLIKON_ASSERT(res == db::SqliteResult::OK);

    exp = _db->prepareStatement(replikon::INIT_SECURITY);
    statement = std::move(exp.value());
    res = statement.step();
    REPLIKON_ASSERT(res == db::SqliteResult::OK);

    exp = _db->prepareStatement(replikon::INIT_KEY_VALUE);
    statement = std::move(exp.value());
    res = statement.step();
    REPLIKON_ASSERT(res == db::SqliteResult::OK);

    _messages_dao = std::make_shared<dao::MessagesDao>(_db);
    _security_dao = std::make_shared<dao::SecurityDao>(_db);
    _kv_dao = std::make_shared<dao::KeyValueDao>(_db);
    _security_provider = std::make_shared<sec::ED25519SecurityProvider>(
        _security_dao, _self.author, _self.pub_key, _self.priv_key.value());
    _messages_log =
        std::make_unique<crdt::Log<ChatMessage, sec::ED25519SecurityProvider>>(
            _messages_dao, _security_provider);
    _keys = std::make_unique<crdt::KeysCrdt<sec::ED25519SecurityProvider>>(
        _kv_dao, _security_dao, _security_provider, _self.author);
  }

public:
  crdt::Log<ChatMessage, sec::ED25519SecurityProvider> &messagesCrdt() & {
    return *_messages_log;
  }
  crdt::KeysCrdt<sec::ED25519SecurityProvider> &keysCrdt() & { return *_keys; }

  const SecurityUserInfo &self() const & { return _self; }

  void setAdmin(const Author &admin, PubKey pubk) {
    auto res = _security_dao->insertAdminPublicKey(admin, pubk);
    REPLIKON_ASSERT(res == db::SqliteResult::OK);
  }

  std::shared_ptr<dao::KeyValueDao> kvDao() const { return _kv_dao; }
  std::shared_ptr<dao::MessagesDao> messagesDao() const {
    return _messages_dao;
  }
  std::shared_ptr<dao::SecurityDao> securityDao() const {
    return _security_dao;
  }
  std::shared_ptr<replikon::db::Sqlite> db() const { return _db; }

private:
  std::shared_ptr<dao::MessagesDao> _messages_dao;
  std::shared_ptr<dao::SecurityDao> _security_dao;
  std::shared_ptr<dao::KeyValueDao> _kv_dao;
  std::shared_ptr<replikon::db::Sqlite> _db;
  std::shared_ptr<sec::ED25519SecurityProvider> _security_provider;
  std::unique_ptr<crdt::Log<ChatMessage, sec::ED25519SecurityProvider>>
      _messages_log;
  std::unique_ptr<crdt::KeysCrdt<sec::ED25519SecurityProvider>> _keys;
  SecurityUserInfo _self;
};

} // namespace replikon

#endif // REPLIKON_INSTANCE_H