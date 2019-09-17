#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <memory>

namespace sgs {

   class Server;

    class Command {
        public:
            explicit Command(Server& server);
            virtual ~Command() = default;
            virtual void execute(const std::int32_t fd) = 0;
            static std::unique_ptr<Command> createCommand(Server& server, const std::string& cmd);

        protected:
            Server& m_server;
    };

} /* namespace sgs */

#endif // COMMAND_HPP
