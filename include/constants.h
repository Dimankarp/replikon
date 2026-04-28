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
                                         "body TEXT)";

const static std::string INDEX_MESSAGES =
    "CREATE UNIQUE INDEX IF NOT EXISTS idx_messages_lamport "
    "ON messages (id, lamport)";

const static std::string TEMP_SEARCH_INTERVALS =
    "CREATE TEMP TABLE search_intervals "
    "(start INTEGER, end INTEGER)";

const static std::string INIT_SECURITY = "CREATE TABLE IF NOT EXISTS security ("
                                         "id INTEGER PRIMARY KEY,"
                                         "author TEXT UNIQUE NOT NULL,"
                                         "public_key BLOB NOT NULL,"
                                         "private_key BLOB)";

const size_t HASH_LEN = crypto_generichash_blake2b_BYTES;

} // namespace replikon

#endif // REPLIKON_CONSTANTS_H