#ifndef SEQUENCECOMMAND_HPP
#define SEQUENCECOMMAND_HPP

#include "command.hpp"

namespace sgs {

    class SequenceCommand : public Command {
        public:
            explicit SequenceCommand(Server& server,const std::string& cmd);
            ~SequenceCommand() = default;

            virtual void execute(const std::int32_t fd);

        private:
            const std::string m_cmd;
            std::uint64_t m_index;
    };

} /* namespace sgs */

#endif // SEQUENCECOMMAND_HPP
