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

#include "log.h"
#include "dbus_message_handle.h"
#include "power_state_change_reason.h"

#include <mir/log.h>

#include <dbus/dbus.h>

#include <sstream>
#include <iostream>
#include <stdarg.h>

namespace
{

std::string reason_to_string(int32_t reason_int)
{
    if (reason_int < 0 || reason_int >= static_cast<int32_t>(PowerStateChangeReason::num_reasons))
        return "invalid";

    switch(static_cast<PowerStateChangeReason>(reason_int))
    {
    case PowerStateChangeReason::unknown: return "unknown";
    case PowerStateChangeReason::inactivity: return "inactivity";
    case PowerStateChangeReason::power_key: return "power_key";
    case PowerStateChangeReason::proximity: return "proximity";
    case PowerStateChangeReason::notification: return "notification";
    case PowerStateChangeReason::snap_decision: return "snap_decision";
    case PowerStateChangeReason::call_done: return "call_done";
    default: return "invalid";
    };
}

void append_dbus_message_arguments(
    std::stringstream& ss, usc::DBusMessageHandle const& message,
    std::initializer_list<char const*> arg_names)
{
    auto arg_name_iter = arg_names.begin();

    message.for_each_argument(
        [&] (int arg_type, void* arg)
        {
            bool is_power_state_change_reason{false};

            if (arg_name_iter != arg_names.end())
            {
                ss << *arg_name_iter << ": ";
                is_power_state_change_reason =
                    *arg_name_iter == std::string{"power_state_change_reason"};
                ++arg_name_iter;
            }

            if (arg_type == DBUS_TYPE_STRING)
            {
                ss << *static_cast<char**>(arg) << " ";
            }
            else if (arg_type == DBUS_TYPE_INT32)
            {
                auto const arg_as_int32 = *static_cast<int32_t*>(arg);

                if (is_power_state_change_reason)
                    ss << reason_to_string(arg_as_int32) << " ";
                else
                    ss << arg_as_int32 << " ";
                
            }
            else if (arg_type == DBUS_TYPE_BOOLEAN)
            {
                ss << *static_cast<dbus_bool_t*>(arg) << " ";
            }
        });
}


}

void usc::log::unity_screen_service_request(
    DBusMessageHandle const& message, std::initializer_list<char const*> arg_names)
{
    auto const sender = dbus_message_get_sender(message);
    auto const member = dbus_message_get_member(message);
    auto const serial = dbus_message_get_serial(message);

    std::stringstream ss;

    ss << "Received request with serial " << serial
       << " from " << (sender ? sender : "(unknown)") << " : "
       << (member ? member : "(none)") << " ";

    append_dbus_message_arguments(ss, message, arg_names);

    mir::log(
        mir::logging::Severity::informational,
        "usc::UnityScreenService",
        ss.str());
}

void usc::log::unity_screen_service_request(DBusMessageHandle const& message)
{
    usc::log::unity_screen_service_request(message, {});
}

void usc::log::unity_screen_service_reply(
    DBusMessageHandle const& message, std::initializer_list<char const*> arg_names)
{
    auto const destination = dbus_message_get_destination(message);
    auto const serial = dbus_message_get_reply_serial(message);

    std::stringstream ss;

    ss << "Sending reply for serial " << serial
       << " to " << (destination ? destination : "(unknown)") << " ";

    append_dbus_message_arguments(ss, message, arg_names);

    mir::log(
        mir::logging::Severity::informational,
        "usc::UnityScreenService",
        ss.str());
}

void usc::log::unity_screen_service_reply(DBusMessageHandle const& message)
{
    usc::log::unity_screen_service_reply(message, {});
}

void usc::log::unity_screen_service_peer_disappearance(std::string const& peer)
{
    mir::log(mir::logging::Severity::informational,
        "usc::UnityScreenService",
        "peer %s disappeared from bus, removing all of its display on requests",
        peer.c_str());
}

void usc::log::powerd_method_invocation(DBusMessageHandle const& message)
{
    std::stringstream ss;

    auto const member = dbus_message_get_member(message);

    ss << "Invoking powerd method " << (member ? member : "(unknown)") << " ";

    append_dbus_message_arguments(ss, message, {});

    mir::log(
        mir::logging::Severity::informational,
        "usc::PowerdMediator",
        ss.str());
}

void usc::log::powerd_method_reply(std::string const& method, DBusMessageHandle const& message)
{
    std::stringstream ss;

    ss << "Received powerd reply for " << method << " ";

    append_dbus_message_arguments(ss, message, {});

    mir::log(
        mir::logging::Severity::informational,
        "usc::PowerdMediator",
        ss.str());
}

void usc::log::power_key_press()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::ScreenEventHandler",
        "power key pressed");
}

void usc::log::power_key_release()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::ScreenEventHandler",
        "power key released");
}

void usc::log::power_key_shutdown()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::ScreenEventHandler",
        "power key shutdown");
}

void usc::log::power_key_long_press()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::ScreenEventHandler",
        "power key long pressed");
}

void usc::log::screen_power_off_alarm()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::MirScreen",
        "power off alarm triggered");

}

void usc::log::screen_dim_alarm()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::MirScreen",
        "dim alarm triggered");
}

void usc::log::dm_connection_start()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::AsioDMConnection",
        "starting");
}

void usc::log::dm_message_header_read_failure()
{
    mir::log(
        mir::logging::Severity::error,
        "usc::AsioDMConnection",
        "failed to read message header");
}

void usc::log::dm_ping_message()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::AsioDMConnection",
        "received ping message");
}

void usc::log::dm_pong_message()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::AsioDMConnection",
        "received pong message");
}

void usc::log::dm_set_active_session_message(std::string const& session)
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::AsioDMConnection",
        "received set_active_session message with session: " + session);
}

void usc::log::dm_set_next_session_message(std::string const& session)
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::AsioDMConnection",
        "received set_next_session message with session: " + session);
}

void usc::log::dm_unknown_message(int message_id, size_t payload_length)
{
    mir::log(
        mir::logging::Severity::warning,
        "usc::AsioDMConnection",
        "received unknown message with id: " + std::to_string(message_id) +
            " payload_length " + std::to_string(payload_length));
}


void usc::log::dm_message_payload_read_failure()
{
    mir::log(
        mir::logging::Severity::error,
        "usc::AsioDMConnection",
        "failed to read message payload");
}

void usc::log::session_switcher_add_session(std::string const& name, pid_t pid)
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::SessionSwitcher",
        "Adding session with name: " + name + " pid: " + std::to_string(pid));
}

void usc::log::session_switcher_remove_session(std::string const& name)
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::SessionSwitcher",
        "Removing session with name: " + name);
}

void usc::log::session_switcher_show_session(std::string const& name, std::string const& mode)
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::SessionSwitcher",
        "Showing session with name: " + name + " mode: " + mode);

}

void usc::log::session_switcher_hide_session(std::string const& name)
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::SessionSwitcher",
        "Hiding session with name: " + name);
}

void usc::log::session_switcher_show_spinner(std::string const& mode)
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::SessionSwitcher",
        "Showing spinner with mode: " + mode);
}

void usc::log::session_switcher_hide_spinner()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::SessionSwitcher",
        "Hiding spinner");
}

void usc::log::server_pause()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::Server",
        "paused");
}

void usc::log::server_resume()
{
    mir::log(
        mir::logging::Severity::informational,
        "usc::Server",
        "resumed");
}

void usc::log::server_invalid_arguments(int argc, char const* const* argv)
{
    std::stringstream ss;
    ss << "unrecognized arguments:";
    for (auto arg = argv; arg != argv + argc; ++arg)
        ss << " " << *arg;

    mir::log(
        mir::logging::Severity::warning,
        "usc::Server",
        ss.str());
}
