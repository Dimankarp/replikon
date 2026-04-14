#ifndef REPLIKON_CONSTANTS_H
#define REPLIKON_CONSTANTS_H

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

    
} // namespace replikon

#endif // REPLIKON_CONSTANTS_H