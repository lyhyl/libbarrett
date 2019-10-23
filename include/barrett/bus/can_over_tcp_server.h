/**
 *	Copyright 2019-2119 Wingkou <wingkou@outlook.com>
 */

/**
 * @file can_over_tcp_socket.h
 * @date 10/22/2019
 * @author Wingkou
 */

#ifndef CAN_OVER_TCP_SERVER_H_
#define CAN_OVER_TCP_SERVER_H_

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <barrett/bus/can_socket.h>

namespace barrett
{
namespace bus
{
using boost::asio::ip::tcp;
class session;
class CANOverTCPServer
{
public:
    CANOverTCPServer(boost::asio::io_service &io_service, short port, int canPort);
    void handle_accept(session *new_session, const boost::system::error_code &error);

private:
    CANSocket cansocket;
    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;
};
} // namespace bus
} // namespace barrett

#endif
