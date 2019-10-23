/**
 *	Copyright 2019-2119 Wingkou <wingkou@outlook.com>
 */

/**
 * @file can_over_tcp_socket.h
 * @date 10/22/2019
 * @author Wingkou
 */

#include <barrett/os.h>
#include "can_over_tcp_server.h"

namespace barrett
{
namespace bus
{

int32_t readInt32(const unsigned char *data)
{
    return *reinterpret_cast<const int32_t *>(data);
    // int32_t b0 = data[0];
    // int32_t b1 = data[1];
    // int32_t b2 = data[2];
    // int32_t b3 = data[3];
    // return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

class session
{
public:
    static const size_t header_length = 4;

    session(boost::asio::io_service &io_service, CANSocket *cansocket)
        : socket_(io_service), cansocket_(cansocket)
    {
        if (cansocket_ == NULL)
        {
            throw std::invalid_argument("CANSocket cannot be NULL");
        }
    }

    tcp::socket &socket()
    {
        return socket_;
    }

    void start()
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(data_, header_length),
                                boost::bind(&session::handle_read_header, this,
                                            boost::asio::placeholders::error));
    }

    void handle_read_header(const boost::system::error_code &error)
    {
        if (!error)
        {
            // msg : type(4)
            int32_t type = readInt32(reinterpret_cast<const unsigned char *>(data_));
            switch (type)
            {
            case 0:
                doCanSend();
                break;
            case 1:
                doCanReceive();
                break;
            default:
                (logMessage("session::%s(): Unknown CAN over TCP header.") % __func__).raise<std::runtime_error>();
                break;
            }
            boost::asio::async_read(socket_,
                                    boost::asio::buffer(data_, header_length),
                                    boost::bind(&session::handle_read_header, this,
                                                boost::asio::placeholders::error));
        }
        else
        {
            std::cout << "handle_read_header died" << std::endl;
            delete this;
        }
    }

    void doCanSend()
    {
        // msg : busId(4) + len(4) + data
        const size_t fixed_length = 8;
        size_t actual_fixed_length = boost::asio::read(socket_, boost::asio::buffer(data_, fixed_length));

        if (actual_fixed_length != fixed_length)
        {
            // TODO
        }

        int busId = readInt32(reinterpret_cast<const unsigned char *>(data_));
        int len = readInt32(reinterpret_cast<const unsigned char *>(data_ + 4));

        size_t actual_data_length = boost::asio::read(socket_, boost::asio::buffer(data_, len));

        if (actual_data_length != (size_t)len)
        {
            // TODO
        }

        // printf("<<bus id:%d len:%d data:", busId, len);
        // for (size_t i = 0; i < len; i++)
        // {
        //     printf("%X", (unsigned char)data_[i]);
        // }
        // putchar('\n');

        // msg : ret(4)
        int32_t ret = cansocket_->send(busId, reinterpret_cast<const unsigned char *>(data_), len);

        // printf(">>ret:%d\n", ret);

        memcpy(data_, &ret, 4);
        boost::asio::write(socket_, boost::asio::buffer(data_, 4));
    }

    void doCanReceive()
    {
        // msg : blocking(4)
        const size_t fixed_length = 4;
        size_t actual_fixed_length = boost::asio::read(socket_, boost::asio::buffer(data_, fixed_length));

        if (actual_fixed_length != fixed_length)
        {
            // TODO
        }

        int blocking = readInt32(reinterpret_cast<const unsigned char *>(data_));

        // printf("<<blocking:%d\n", blocking);

        // msg : ret(4) + busId(4) + len(4) + data
        int busId;
        size_t len;
        int32_t ret = cansocket_->receiveRaw(busId, reinterpret_cast<unsigned char *>(data_ + 12), len, blocking == 1);

        if (ret != 0)
        {
            len = 0;
        }

        // printf(">>ret:%d bus id:%d len:%d data:", ret, busId, (int)len);
        // for (size_t i = 0; i < len; i++)
        // {
        //     printf("%X", (unsigned char)data_[i + 12]);
        // }
        // putchar('\n');

        int32_t dbusId = busId;
        int32_t dlen = len;
        memcpy(data_, &ret, 4);
        memcpy(data_ + 4, &dbusId, 4);
        memcpy(data_ + 8, &dlen, 4);
        boost::asio::write(socket_, boost::asio::buffer(data_, 4 + 4 + 4 + len));
    }

private:
    tcp::socket socket_;
    CANSocket *cansocket_;
    enum
    {
        max_length = 64
    };
    char data_[max_length];
};

CANOverTCPServer::CANOverTCPServer(boost::asio::io_service &io_service, short port, int canPort)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    cansocket.open(canPort);

    std::cout << "CAN Socket is " << (cansocket.isOpen() ? "opened" : "closed") << std::endl;

    session *new_session = new session(io_service_, &cansocket);
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&CANOverTCPServer::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
}

void CANOverTCPServer::handle_accept(session *new_session,
                                     const boost::system::error_code &error)
{
    if (!error)
    {
        new_session->start();

        std::cout << "session started" << std::endl;

        new_session = new session(io_service_, &cansocket);
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&CANOverTCPServer::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }
    else
    {
        std::cout << "session died" << std::endl;

        delete new_session;
    }
}

} // namespace bus
} // namespace barrett