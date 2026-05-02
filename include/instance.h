#ifndef REPLIKON_INSTANCE_H
#define REPLIKON_INSTANCE_H

#include "crdt/log.h"
#include "dao/message.h"
#include "dao/security.h"
#include "security/provider.h"
#include "types.h"
#include <memory>
namespace replikon{

    class Instance{

        public:
        



        private:
        std::shared_ptr<dao::MessagesDao> _messages_dao;
        std::shared_ptr<dao::SecurityDao> _security_dao;
        std::shared_ptr<sec::ED25519SecurityProvider> _security_provider;
        Log<ChatMessage, sec::ED25519SecurityProvider> _messages_log;
    }


}

#endif // REPLIKON_INSTANCE_H