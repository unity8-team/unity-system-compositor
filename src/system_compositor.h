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

#ifndef SYSTEM_COMPOSITOR_H_
#define SYSTEM_COMPOSITOR_H_

#include "dm_connection.h"

#include <thread>
#include <mir/default_server_configuration.h>
#include <mir/shell/session.h>

class Configuration;

class SystemCompositor : public DMMessageHandler
{
public:
    SystemCompositor(int from_dm_fd, int to_dm_fd);

    int run(int argc, char const* argv[]);

    std::shared_ptr<mir::shell::Session> get_active_session();

    void emit_change_notification();

private:
    std::shared_ptr<Configuration> config;
    boost::asio::io_service io_service;
    DMConnection dm_connection;
    std::thread thread;
    std::shared_ptr<mir::shell::Session> active_session;
    std::shared_ptr<mir::shell::Session> old_session;

    virtual void set_active_session(std::string client_name);
    void main();
};

#endif /* SYSTEM_COMPOSITOR_H_ */
