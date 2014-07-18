/*
 * Copyright © 2014 Canonical Ltd.
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
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#ifndef USC_SERVER_CONFIGURATION_H_
#define USC_SERVER_CONFIGURATION_H_

#include <mir/default_server_configuration.h>
#include <mir/options/option.h>

namespace usc
{
class Spinner;
class SessionSwitcher;
class DMMessageHandler;
class DMConnection;

class ServerConfiguration : public mir::DefaultServerConfiguration
{
public:
    ServerConfiguration(int argc, char** argv);

    virtual std::shared_ptr<Spinner> the_spinner();
    virtual std::shared_ptr<DMMessageHandler> the_dm_message_handler();
    virtual std::shared_ptr<DMConnection> the_dm_connection();

    bool show_version()
    {
        return the_options()->is_set("version");
    }

    int inactivity_display_off_timeout()
    {
       return the_options()->get("inactivity-display-off-timeout", 60);
    }

    int inactivity_display_dim_timeout()
    {
       return the_options()->get("inactivity-display-dim-timeout", 45);
    }

    int shutdown_timeout()
    {
       return the_options()->get("shutdown-timeout", 5000);
    }

    int power_key_ignore_timeout()
    {
       return the_options()->get("power-key-ignore-timeout", 1500);
    }

    bool enable_hardware_cursor()
    {
        return the_options()->get("enable-hardware-cursor", false);
    }

    bool disable_inactivity_policy()
    {
        return the_options()->get("disable-inactivity-policy", false);
    }

    std::string blacklist()
    {
        auto x = the_options()->get("blacklist", "");
        //boost::trim(x);
        return x;
    }

    std::string spinner_executable()
    {
        // TODO: once our default spinner is ready for use everywhere, replace
        // default value with DEFAULT_SPINNER instead of the empty string.
        auto x = the_options()->get("spinner", "");
        //boost::trim(x);
        return x;
    }

    bool public_socket()
    {
        return !the_options()->is_set("no-file") && the_options()->get("public-socket", true);
    }

    std::string get_socket_file()
    {
        // the_socket_file is private, so we have to re-implement it here
        return the_options()->get("file", "/tmp/mir_socket");
    }

protected:
    virtual std::shared_ptr<SessionSwitcher> the_session_switcher();
    std::shared_ptr<mir::input::CursorListener> the_cursor_listener() override;
    std::shared_ptr<mir::ServerStatusListener> the_server_status_listener() override;
    std::shared_ptr<mir::scene::SessionCoordinator> wrap_session_coordinator(
        std::shared_ptr<mir::scene::SessionCoordinator> const& wrapped) override;
    std::shared_ptr<mir::scene::SurfaceCoordinator> wrap_surface_coordinator(
        std::shared_ptr<mir::scene::SurfaceCoordinator> const& wrapped) override;

    mir::CachedPtr<Spinner> spinner;
    mir::CachedPtr<DMConnection> dm_connection;
    mir::CachedPtr<SessionSwitcher> session_switcher;
};

}

#endif
