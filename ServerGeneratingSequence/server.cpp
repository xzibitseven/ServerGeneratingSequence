#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <limits>
#include <iomanip>
#include "command.hpp"
#include "server.hpp"

namespace sgs {

Server::Server(const std::uint16_t& port):
    m_work(true),
    m_listener(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) {

    if(m_listener < 0)
        throw std::runtime_error("Error create socket\n");

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
        
    std::string ipAdress{"127.0.0.7"};
    m_addr.sin_addr.s_addr = inet_addr(ipAdress.c_str());    

    if(bind(m_listener, reinterpret_cast<struct sockaddr *>(&m_addr), sizeof(m_addr)) < 0)
        throw std::runtime_error("Error bind\n");

    if(listen(m_listener, 1000) < 0)
        throw std::runtime_error("Error listen\n");

    if(fcntl(m_listener, F_SETFL, O_NONBLOCK) < 0)
        std::cerr << "Error fcntl\n";
        
    std::cout << "listening " << ipAdress << ":" << port << '\n';    
}

Server::~Server() {
    close(m_listener);
}

void Server::run() {

    FD_ZERO(&m_fdsRead);
    FD_SET(m_listener, &m_fdsRead);

    fd_set fdsRead;
    FD_ZERO(&fdsRead);

    std::int32_t fdMax{m_listener};
    std::int32_t tfdMax{};
    std::int32_t res{};

    while(m_work) {
        tfdMax = {fdMax};
        {
            std::shared_lock<SharedMutex> lock(m_mtxFdsRead);
            fdsRead = {m_fdsRead};
        }
        res = pselect(tfdMax+1, &fdsRead, nullptr, nullptr, &constants::timeout, nullptr);

        switch(res) {
            case EventFds::Error : {
                throw std::runtime_error("Error pselect\n");
            }
            case EventFds::Timeout : {
                continue;
            }
            default : {

                for(std::int32_t i=0; i<=tfdMax; ++i) {

                    if(FD_ISSET(i, &fdsRead)) {

                        if(i == m_listener) {
                            std::int32_t sock = accept(m_listener, nullptr, nullptr);
                            if(sock < 0)
                                throw std::runtime_error("Error accept\n");
                            else {
                                if(fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
                                    std::cerr << "Error fcntl\n";
                                {
                                    std::lock_guard<SharedMutex> lock(m_mtxFdsRead);
                                    FD_SET(sock, &m_fdsRead);
                                }

                                if(sock > fdMax)
                                    fdMax = sock;
                            }
                        } else {
                             m_threadPool.submit(std::bind(&Server::handlerClient, this, i, EventHandler::Read));
                             {
                                 std::lock_guard<SharedMutex> lock(m_mtxFdsRead);
                                 FD_CLR(i, &m_fdsRead);
                             }
                        }
                    }
                }

            } /* default */
        }
    }
}

void Server::setSequence(const Sequence& seq, const std::uint64_t& idx, const std::int32_t fd) {

    Sequence sequence{};
    if(!m_storage.get(fd,sequence)) {
        m_storage.set(fd,seq);
    } else {
        sequence[idx] = seq[idx];
        m_storage.set(fd,sequence);
    }

    return;
}

void Server::exportSequence(const std::int32_t fd) {

    Sequence seq{};
    if(!m_storage.get(fd,seq))
        return;

    bool generate{true};
    for(std::uint8_t i=0; i<constants::numberSubSequence; ++i)
        generate = generate && seq[i].flag;

    if(!generate)
        return;

    if(!onWrite(fd, generateSequence(seq)))
        return;

    m_storage.set(fd,seq);
    m_threadPool.submit(std::bind(&Server::handlerClient, this, fd, EventHandler::Write));

    return;
}

void Server::handlerClient(const std::int32_t fd, const EventHandler& event) {

    switch(event) {
        case EventHandler::Read : {
            onRead(fd);
            break;
        }
        case EventHandler::Write : {
            Sequence seq{};
            if(!m_storage.get(fd,seq))
                return;

            if(!onWrite(fd, generateSequence(seq)))
                return;

            m_storage.set(fd,seq);
            std::this_thread::sleep_for(std::chrono::nanoseconds(1)); // for testing with a small number of clients
            m_threadPool.submit(std::bind(&Server::handlerClient,this,fd, EventHandler::Write));
            break;
        }
    }
    return;
}

bool Server::onRead(const std::int32_t fd) {

    std::string buffer(constants::READ_BUFFER_SIZE, '\0');
    std::string cmd;
    std::uint64_t read_buffer_used{0};
    std::int64_t bytes_read{0};

    auto setFds = [&fd,this] () {
            std::lock_guard<SharedMutex> lock(m_mtxFdsRead);
            FD_SET(fd, &m_fdsRead);
    };

    for(;;) {
        bytes_read = read(fd, &buffer[0] + read_buffer_used, constants::READ_BUFFER_SIZE - read_buffer_used);
        if(bytes_read == 0) {
            closeClient(fd);
            return false;
        }
        if(bytes_read < 0) {
            if(errno == EINTR)
                continue;

            closeClient(fd);
            return false;
        }
        break; // read() succeeded
    }

    std::uint64_t check{read_buffer_used};
    std::uint64_t check_end{read_buffer_used + static_cast<std::uint64_t>(bytes_read)};
    std::unique_ptr<Command> upCmd;
    read_buffer_used = check_end;


    while(check < check_end) {
        if(buffer[check] != '\n'){
                check++;
                continue;
        }
        std::uint64_t length = check;
        buffer[length] = '\0';
        if((length > 0) && (buffer[length - 1] == '\r')) {
            buffer[length - 1] = '\0';
            length--;
        }

        cmd = { buffer.substr(0,length)};

        auto checkData = [&cmd, &upCmd, this]()->bool {
            upCmd = Command::createCommand(*this, cmd);
            return upCmd.get();
        };

        if(!checkData()) {
            std::cerr << "Client sent invalid data\n";
            setFds();
            return false;
        }

        read_buffer_used -= check + 1;
        check_end -= check;
        check = 0;
    }
    if(read_buffer_used == constants::READ_BUFFER_SIZE) {
        std::cerr << "Client sent a very long string, closing connection.\n";
        setFds();
        return false;
    }

    if(upCmd)
        upCmd->execute(fd);

    setFds();
    return true;
}

bool Server::onWrite(const std::int32_t fd, const std::string& buffer) {

    std::uint64_t writeBytes{0};
    const std::uint64_t sizeBuffer{buffer.size()};
    std::int64_t bytes{0};

    while(writeBytes < sizeBuffer) {
        bytes = write(fd, &buffer[0]+writeBytes, buffer.size() - writeBytes);
        if(bytes <= 0) {
            if(errno == EINTR)
                continue;

            closeClient(fd);
            return false;
        }
        writeBytes += static_cast<std::uint64_t>(bytes);
    }

    return true;
}

void Server::closeClient(std::int32_t fd) {
    close(fd);
    m_storage.erase(fd);
}

std::string Server::generateSequence(Sequence& seq) {

    std::string buffer(constants::WRITE_BUFFER_SIZE, '\0');

    for(std::uint8_t i=0; i<constants::numberSubSequence; ++i) {
        if(seq[i].flag) {
            buffer += std::to_string(seq[i].part) + ' ';
            if( (std::numeric_limits<std::uint64_t>::max() - seq[i].part) >= seq[i].step )
                seq[i].part += seq[i].step;
            else
                seq[i].part = seq[i].initial;

        }

    }
    std::string::size_type pos {buffer.rfind(' ')};
    if(pos == std::string::npos)
        return std::string{};

    buffer[pos] = '\n';

    return buffer;
}

} /* namespace sgs */
