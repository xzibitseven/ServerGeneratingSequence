#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include "src/threadpool.hpp"
#include <functional> /* for C++17 */
#include "storage.hpp"


/*
 * namespace sgs- is server generating sequence
*/

namespace sgs {

    namespace constants {
        constexpr std::uint64_t READ_BUFFER_SIZE{16};
        constexpr std::uint64_t WRITE_BUFFER_SIZE{65};
        constexpr struct timespec timeout{
                                           0, /*seconds*/
                                           10 /* nanoseconds */
                                          };
    }

    class Server {
        enum EventFds : std::int32_t {
                                      Error = -1,
                                      Timeout = 0,
                                     };

        enum class EventHandler : std::uint8_t {
                                                Read,
                                                Write
                                               };

        public:
            explicit Server(const std::uint16_t& port);
            Server(const Server& ) = delete;
            Server& operator=(const Server& ) = delete;
            ~Server();

            void run();
            inline void stop() { m_work = false; }

            /*
             * Support server commands
             */
            void setSequence(const Sequence& seq, const std::uint64_t& idx, const std::int32_t fd);
            void exportSequence(const std::int32_t fd);

        private:
            void handlerClient(const std::int32_t fd, const EventHandler& event = EventHandler::Read);
            bool onRead(const std::int32_t fd);
            bool onWrite(const std::int32_t fd, const std::string& buffer);
            void closeClient(const std::int32_t fd);
            std::string generateSequence(Sequence& seq);

        private:
            bool m_work;
            std::int32_t m_listener;
            struct sockaddr_in m_addr;
            fd_set m_fdsRead;
            SharedMutex m_mtxFdsRead;
            Storage m_storage;
            ThreadPool<std::function<void()>> m_threadPool;
    };

} /* namespace sgs */

#endif // SERVER_HPP
