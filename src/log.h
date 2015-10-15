/*
 * Copyright © 2015 Canonical Ltd.
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

#ifndef USC_LOG_H_
#define USC_LOG_H_

#include <string>
#include <initializer_list>
#include <sys/types.h>

namespace usc
{
class DBusMessageHandle;

namespace log
{

void unity_screen_service_request(
    DBusMessageHandle const& message, std::initializer_list<char const*> arg_names);
void unity_screen_service_request(DBusMessageHandle const& message);
void unity_screen_service_reply(
    DBusMessageHandle const& message, std::initializer_list<char const*> arg_names);
void unity_screen_service_reply(DBusMessageHandle const& message);
void unity_screen_service_peer_disappearance(std::string const& peer);

void powerd_method_invocation(DBusMessageHandle const& message);
void powerd_method_reply(std::string const& method, DBusMessageHandle const& message);

void power_key_press();
void power_key_release();
void power_key_shutdown();
void power_key_long_press();

void screen_power_off_alarm();
void screen_dim_alarm();

void dm_connection_start();
void dm_message_header_read_failure();
void dm_ping_message();
void dm_pong_message();
void dm_set_active_session_message(std::string const& session);
void dm_set_next_session_message(std::string const& session);
void dm_unknown_message(int message_id, size_t payload_length);
void dm_message_payload_read_failure();

void session_switcher_add_session(std::string const& name, pid_t pid);
void session_switcher_remove_session(std::string const& name);
void session_switcher_show_session(std::string const& name, std::string const& mode);
void session_switcher_hide_session(std::string const& name);
void session_switcher_show_spinner(std::string const& mode);
void session_switcher_hide_spinner();

void server_pause();
void server_resume();
void server_invalid_arguments(int argc, char const* const* argv);

}
}

#endif
