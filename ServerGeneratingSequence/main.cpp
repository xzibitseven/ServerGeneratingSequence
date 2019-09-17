#include <iostream>
#include "server.hpp"


int main(int argc, char* argv[]) {
    try {
        if(argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <port>\n";
            return 0;
        }

        const std::int64_t port = std::stol(argv[1]);
        if(port < 1 || port > 65535) {
            std::cerr << "Port number out of range\n";
        }

        sgs::Server server(static_cast<std::uint16_t>(port));
        server.run();

    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
    } catch( ... ) {
        std::cerr << "Unknown exception!!!\n";
    }
}
