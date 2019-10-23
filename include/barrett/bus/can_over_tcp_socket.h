/**
 *	Copyright 2019-2119 Wingkou <wingkou@outlook.com>
 */

/**
 * @file can_over_tcp_socket.h
 * @date 10/22/2019
 * @author Wingkou
 */

#ifndef BARRETT_BUS_CAN_SOCKET_H_
#define BARRETT_BUS_CAN_SOCKET_H_

#include <stdexcept>

#include <boost/asio.hpp>

#include <barrett/detail/ca_macro.h>
#include <barrett/thread/real_time_mutex.h>
#include <barrett/bus/abstract/communications_bus.h>

namespace barrett
{
namespace bus
{
using boost::asio::ip::tcp;
class CANOverTCPSocket : public CommunicationsBus
{
public:
    static const size_t MAX_MESSAGE_LEN = 8; /** The maximum length of a CANbus message. Make sure to update CommunicationsBus::MAX_MESSAGE_LEN! */

    /** CANOverTCPSocket() Constructors
	 */
    CANOverTCPSocket();
    CANOverTCPSocket(int port) throw(std::runtime_error);
    ~CANOverTCPSocket();
    /** getMutex() method gets and locks interthread data exchange assuring nothing critical is happening in either thread.
	 */
    virtual thread::RealTimeMutex &getMutex() const { return mutex; }
    /** open() method creates socket communication on a specific port.
	 */
    virtual void open(int port) throw(std::logic_error, std::runtime_error);
    /** close() method destorys socket communication port.
	 */
    virtual void close();
    /** isOpen() method returns a flag signifying socket Communication is open.
	 */
    virtual bool isOpen() const;
    /** send() method pushes data onto socket. 
	 */
    virtual int send(int busId, const unsigned char *data, size_t len) const;
    /** receiveRaw() method loads data from socket buffer in a realtime safe manner.
	 */
    virtual int receiveRaw(int &busId, unsigned char *data, size_t &len, bool blocking = true) const;

protected:
    mutable thread::RealTimeMutex mutex;

    boost::asio::io_service io_service;
    tcp::socket *socket;

    mutable unsigned char packetData[64];

private:
    DISALLOW_COPY_AND_ASSIGN(CANOverTCPSocket);
};

} // namespace bus
} // namespace barrett

#endif /* BARRETT_BUS_CAN_SOCKET_H_ */
