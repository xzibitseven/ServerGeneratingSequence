#include "server.hpp"
#include "exportseqcommand.hpp"

namespace sgs {

    ExportSeqCommand::ExportSeqCommand(Server& server):
        Command(server){}

    /*virtual*/ void ExportSeqCommand::execute(const std::int32_t fd) {
        m_server.exportSequence(fd);
    }

} /* namespace sgs */

