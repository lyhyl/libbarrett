/**
 *	Copyright 2019-2119 Wingkou <wingkou@outlook.com>
 */

/**
 * @file can_over_tcp_socket.h
 * @date 10/22/2019
 * @author Wingkou
 */

#include <stdexcept>
#include <cstdio>
#include <cstring>

#include <errno.h>

#include <barrett/os.h>
#include <barrett/thread/real_time_mutex.h>
#include <barrett/products/puck.h>
#include "can_over_tcp_socket.h"

namespace barrett
{
namespace bus
{

CANOverTCPSocket::CANOverTCPSocket() : mutex()
{
}

CANOverTCPSocket::CANOverTCPSocket(const char *ip, const char *port) throw(std::runtime_error) : ip_(ip), port_(port)
{
    open(0);
}

CANOverTCPSocket::CANOverTCPSocket(int port) throw(std::runtime_error) : mutex()
{
    open(port);
}

CANOverTCPSocket::~CANOverTCPSocket()
{
    close();
}

void CANOverTCPSocket::open(int port) throw(std::logic_error, std::runtime_error)
{
    if (isOpen())
    {
        (logMessage("CANOverTCPSocket::%s(): This object is already associated with a CAN port.") % __func__).raise<std::logic_error>();
    }

    logMessage("CANOverTCPSocket::open(%d) using CAN Over TCP driver") % port;

    std::cout << "using CAN Over TCP driver" << std::endl;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), ip_, port_);
    tcp::resolver::iterator iterator = resolver.resolve(query);

    socket = new tcp::socket(io_service);
    socket->connect(*iterator);
}

void CANOverTCPSocket::close()
{
    if (isOpen())
    {
        socket->close();
        delete socket;
        socket = NULL;
    }
}

bool CANOverTCPSocket::isOpen() const
{
    return socket != NULL && socket->is_open();
}

int CANOverTCPSocket::send(int busId, const unsigned char *data, size_t len) const
{
    boost::unique_lock<thread::RealTimeMutex> ul(mutex);

    // msg : type(4) + busId(4) + len(4) + data
    int32_t type = 0;
    int32_t dbusId = busId;
    int32_t dlen = len;
    memcpy(packetData + 0, &type, 4);
    memcpy(packetData + 4, &dbusId, 4);
    memcpy(packetData + 8, &dlen, 4);
    memcpy(packetData + 12, data, len);
    boost::asio::write(*socket, boost::asio::buffer(packetData, len + 12));

    // printf("<<bus id:%d len:%d data:", busId, (int)len);
    // for (size_t i = 0; i < len; i++)
    // {
    //     printf("%X", (unsigned char)data[i]);
    // }
    // putchar('\n');

    // msg : ret(4)
    const size_t reply_length = 4;
    char reply[reply_length];
    size_t actual_reply_length = boost::asio::read(*socket, boost::asio::buffer(reply, reply_length));

    assert(actual_reply_length == reply_length);

    int ret = *reinterpret_cast<int32_t *>(reply);
    // printf(">>ret:%d\n", ret);

    return ret;
}

int CANOverTCPSocket::receiveRaw(int &busId, unsigned char *data, size_t &len, bool blocking) const
{
    BARRETT_SCOPED_LOCK(mutex);

    // printf("<<blocking:%d\n", blocking);

    // msg : type(4) + blocking(4)
    int32_t type = 1;
    int32_t dblocking = blocking ? 1 : 0;
    memcpy(packetData + 0, &type, 4);
    memcpy(packetData + 4, &dblocking, 4);
    boost::asio::write(*socket, boost::asio::buffer(packetData, 8));

    // msg : ret(4) + busId(4) + len(4) + data
    const size_t header_length = 12;
    char header[header_length];
    size_t actual_header_length = boost::asio::read(*socket, boost::asio::buffer(header, header_length));

    assert(actual_header_length == header_length);

    int ret = *reinterpret_cast<int32_t *>(header);
    busId = *reinterpret_cast<int32_t *>(header + 4);
    len = *reinterpret_cast<int32_t *>(header + 8);

    size_t actual_data_length = boost::asio::read(*socket, boost::asio::buffer(data, len));

    assert(actual_data_length == len);

    // printf(">>ret:%d bus id:%d len:%d data:", ret, busId, (int)len);
    // for (size_t i = 0; i < len; i++)
    // {
    //     printf("%X", (unsigned char)data[i]);
    // }
    // putchar('\n');

    return ret;
}

} // namespace bus
} // namespace barrett
