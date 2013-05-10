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

namespace mir
{
class ServerConfiguration;
namespace shell
{
class SessionContainer;
class FocusSetter;
}
}


class Configuration
{
public:
    Configuration() = default;
    virtual mir::ServerConfiguration&                      the_mir_server_configuration() = 0;
    virtual std::shared_ptr<mir::shell::SessionContainer>  the_shell_session_container() = 0;
    virtual std::shared_ptr<mir::shell::FocusSetter>       the_shell_focus_setter() = 0;

protected:
    virtual ~Configuration() = default;
    Configuration(Configuration const&) = delete;
    Configuration& operator=(Configuration const&) = delete;
};

class SystemCompositor : public DMMessageHandler
{
public:
    SystemCompositor(int from_dm_fd, int to_dm_fd, std::shared_ptr<Configuration> const& config);
    void run();

private:
    std::shared_ptr<Configuration> const config;
    boost::asio::io_service io_service;
    DMConnection dm_connection;

    virtual void set_active_session(std::string client_name);
    void main();
};

#endif /* SYSTEM_COMPOSITOR_H_ */
