#include <regex>
#include "common.hpp"
#include "server.hpp"
#include "sequencecommand.hpp"

namespace sgs {

    SequenceCommand::SequenceCommand(Server& server,const std::string& cmd):
              Command(server),
              m_cmd(cmd),
              m_index(std::stoul( cmd.substr(0,cmd.find(' ')).substr(3)) - 1 ) {} /* if index  with 1 (example "seq1")
              else if index with 0 then no need "-1" */

    /*virtual*/ void SequenceCommand::execute(const std::int32_t fd) {

        std::string::size_type pos{m_cmd.find(' ')};
        std::string::size_type npos{m_cmd.find(' ', pos+1)};
        Sequence sequence{};
        std::uint16_t initial{static_cast<std::uint16_t>(std::stoul(m_cmd.substr(pos,npos - pos)))};

        sequence[m_index] = {0, initial, static_cast<std::uint16_t>(std::stoul(m_cmd.substr(npos))), initial};
        if(sequence[m_index].step && sequence[m_index].part)
            sequence[m_index].flag = 1;

        m_server.setSequence(sequence, m_index, fd);
    }

} /* namespace sgs */
