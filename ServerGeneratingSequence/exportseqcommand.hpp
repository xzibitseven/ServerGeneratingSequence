#ifndef EXPORTSEQCOMMAND_HPP
#define EXPORTSEQCOMMAND_HPP

#include "command.hpp"

namespace sgs {

    class ExportSeqCommand : public Command {
        public:
            explicit ExportSeqCommand(Server& server);
            ~ExportSeqCommand() = default;

            virtual void execute(const std::int32_t fd);
    };

} /* namespace sgs */

#endif // EXPORTSEQCOMMAND_HPP
