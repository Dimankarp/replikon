#ifndef REPLIKON_CONSTANTS_H
#define REPLIKON_CONSTANTS_H

#include <sodium.h>
#include <string>
namespace replikon {

const static std::string INIT_MESSAGES = "CREATE TABLE IF NOT EXISTS messages ("
                                         "id INTEGER PRIMARY KEY,"
                                         "author TEXT NOT NULL,"
                                         "origin_ts INTEGER,"
                                         "lamport INTEGER NOT NULL,"
                                         "body TEXT,"
                                         "signature BLOB)";

const static std::string INDEX_MESSAGES =
    "CREATE UNIQUE INDEX IF NOT EXISTS idx_messages_lamport "
    "ON messages (id, lamport)";

const static std::string TEMP_SEARCH_INTERVALS =
    "CREATE TEMP TABLE search_intervals "
    "(start INTEGER, end INTEGER)";

const static std::string INIT_SECURITY =
    "CREATE TABLE IF NOT EXISTS security ("
    "id INTEGER PRIMARY KEY,"
    "author TEXT UNIQUE NOT NULL,"
    "public_key BLOB NOT NULL,"
    "private_key BLOB,"
    "is_admin INTEGER NOT NULL DEFAULT FALSE )";

const static std::string INIT_KEY_VALUE =
    "CREATE TABLE IF NOT EXISTS key_value ("
    "id INTEGER PRIMARY KEY,"
    "key TEXT UNIQUE NOT NULL,"
    "value BLOB)";

const size_t HASH_LEN = crypto_generichash_blake2b_BYTES;

const std::string KEYS_CRDT_VERSION_KEY = "keys_crdt_version";
const std::string KEYS_CRDT_SIGN_KEY = "keys_crdt_sign";
const std::string ADMIN_AUTHOR_KEY = "admin_author";

} // namespace replikon

#endif // REPLIKON_CONSTANTS_H