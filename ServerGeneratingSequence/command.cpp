#include <regex>
#include <iostream>
#include "sequencecommand.hpp"
#include "exportseqcommand.hpp"

namespace sgs {

    Command::Command(Server& server):
                 m_server(server){}

    std::unique_ptr<Command> Command::createCommand(Server& server, const std::string& cmd) {

        std::regex reg(R"(seq[1-3] [0-9]?[0-9]?[0-9]?[0-9] [0-9]?[0-9]?[0-9]?[0-9])");
        if(std::regex_match(cmd, reg)) {
            return std::make_unique<SequenceCommand>(server, cmd);
        }
        reg.assign(R"(export seq)");
        if(std::regex_match(cmd, reg)) {
            return std::make_unique<ExportSeqCommand>(server);
        }

        return nullptr;
    }
} /* sgs */
