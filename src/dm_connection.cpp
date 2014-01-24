/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Robert Ancell <robert.ancell@canonical.com>
 */

#include "dm_connection.h"

#include <boost/signals2.hpp>
#include <iostream>

namespace ba = boost::asio;
namespace bs = boost::system;

void DMConnection::start()
{
    std::cerr << "dm_connection_start" << std::endl;
    read_header();
}

void DMConnection::send_ready()
{
    send(USCMessageID::ready, "");
}

void DMConnection::read_header()
{
    ba::async_read(dm_socket,
                   ba::buffer(message_header_bytes),
                   boost::bind(&DMConnection::on_read_header,
                               this,
                               ba::placeholders::error));
}

void DMConnection::on_read_header(const bs::error_code& ec)
{
    if (!ec)
    {
        size_t const payload_length = message_header_bytes[2] << 8 | message_header_bytes[3];
        ba::async_read(dm_socket,
                       message_payload_buffer,
                       ba::transfer_exactly(payload_length),
                       boost::bind(&DMConnection::on_read_payload,
                                   this,
                                   ba::placeholders::error));
    }
    else
        std::cerr << "Failed to read header" << std::endl;
}

void DMConnection::on_read_payload(const bs::error_code& ec)
{
    if (!ec)
    {
        auto message_id = (USCMessageID) (message_header_bytes[0] << 8 | message_header_bytes[1]);
        size_t const payload_length = message_header_bytes[2] << 8 | message_header_bytes[3];

        switch (message_id)
        {
        case USCMessageID::ping:
        {
            std::cerr << "ping" << std::endl;
            send(USCMessageID::pong, "");
            break;
        }
        case USCMessageID::pong:
        {
            std::cerr << "pong" << std::endl;
            break;
        }
        case USCMessageID::set_active_session:
        {
            std::ostringstream ss;
            ss << &message_payload_buffer;
            auto client_name = ss.str();
            std::cerr << "set_active_session '" << client_name << "'" << std::endl;
            if (handler)
                handler->set_active_session(client_name);
            break;
        }
        case USCMessageID::set_next_session:
        {
            std::ostringstream ss;
            ss << &message_payload_buffer;
            auto client_name = ss.str();
            std::cerr << "set_next_session '" << client_name << "'" << std::endl;
            if (handler)
                handler->set_next_session(client_name);
            break;
        }
        case USCMessageID::add_session:
        {
            std::cerr << "add_session" << std::endl;
            if (handler)
            {
                auto fd = handler->add_session();
                send(USCMessageID::session_added, "");
                send_fd(fd);
            }
            break;
        }
        default:
            std::cerr << "Ignoring unknown message " << (uint16_t) message_id << " with " << payload_length << " octets" << std::endl;
            break;
        }
    }
    else
        std::cerr << "Failed to read payload" << std::endl;

    read_header();
}

void DMConnection::send(USCMessageID id, std::string const& body)
{
    const size_t size = body.size();
    const uint16_t _id = (uint16_t) id;
    const unsigned char header_bytes[4] =
    {
        static_cast<unsigned char>((_id >> 8) & 0xFF),
        static_cast<unsigned char>((_id >> 0) & 0xFF),
        static_cast<unsigned char>((size >> 8) & 0xFF),
        static_cast<unsigned char>((size >> 0) & 0xFF)
    };

    write_buffer.resize(sizeof header_bytes + size);
    std::copy(header_bytes, header_bytes + sizeof header_bytes, write_buffer.begin());
    std::copy(body.begin(), body.end(), write_buffer.begin() + sizeof header_bytes);

    // FIXME: Make asynchronous
    ba::write(dm_socket, ba::buffer(write_buffer));
}

void DMConnection::send_fd(int fd)
{
    char dummy_iov_data = '\0';
    struct iovec iov;
    struct msghdr header;
    struct cmsghdr *control_header;
    int *control_data;
    void *message;

    /* Send dummy data */
    iov.iov_base = &dummy_iov_data;
    iov.iov_len = 1;

    /* Send control message with file descriptor */
    memset (&header, 0, sizeof (header));
    header.msg_iov = &iov;
    header.msg_iovlen = 1;
    header.msg_controllen = sizeof (struct cmsghdr) + sizeof (int);
    message = malloc (header.msg_controllen);
    header.msg_control = message;
    control_header = CMSG_FIRSTHDR (&header);
    control_header->cmsg_len = header.msg_controllen;
    control_header->cmsg_level = SOL_SOCKET;
    control_header->cmsg_type = SCM_RIGHTS;

    control_data = (int*) CMSG_DATA (control_header);
    *control_data = fd;

    if (sendmsg (dm_socket.native_handle(), &header, 0) < 0)
        std::cerr << "Failed to send file descriptor: " << strerror (errno) << std::endl;
  
    free (message);
}
