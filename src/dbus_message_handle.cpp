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

#include "dbus_message_handle.h"

#include <stdexcept>
#include <boost/throw_exception.hpp>

usc::DBusMessageHandle::DBusMessageHandle(DBusMessage* message)
    : message{message}, owned{true}
{
}

usc::DBusMessageHandle::DBusMessageHandle(UnownedDBusMessage message)
    : message{message.message}, owned{false}
{
}

usc::DBusMessageHandle::DBusMessageHandle(::DBusMessage* message, int first_arg_type, ...)
    : DBusMessageHandle{message}
{
    if (!message)
        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid dbus message"));

    va_list args;
    va_start(args, first_arg_type);
    auto appended = dbus_message_append_args_valist(message, first_arg_type, args);
    va_end(args);

    if (!appended)
    {
        dbus_message_unref(message);
        BOOST_THROW_EXCEPTION(
            std::runtime_error("dbus_message_append_args_valist: Failed to append args"));
    }
}

usc::DBusMessageHandle::DBusMessageHandle(
    ::DBusMessage* message, int first_arg_type, va_list args)
    : DBusMessageHandle{message}
{
    if (!message)
        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid dbus message"));

    if (!dbus_message_append_args_valist(message, first_arg_type, args))
    {
        dbus_message_unref(message);
        BOOST_THROW_EXCEPTION(
            std::runtime_error("dbus_message_append_args_valist: Failed to append args"));
    }
}

usc::DBusMessageHandle::DBusMessageHandle(DBusMessageHandle&& other) noexcept
    : message{other.message}, owned{other.owned}
{
    other.message = nullptr;
}

usc::DBusMessageHandle::~DBusMessageHandle()
{
    if (message && owned)
        dbus_message_unref(message);
}

usc::DBusMessageHandle::operator ::DBusMessage*() const
{
    return message;
}

usc::DBusMessageHandle::operator bool() const
{
    return message != nullptr;
}

void usc::DBusMessageHandle::for_each_argument(std::function<void(int,void*)> const& func) const
{
    if (!message) return;

    DBusMessageIter iter;
    int arg_type = DBUS_TYPE_INVALID;

    dbus_message_iter_init(message, &iter);
    while ((arg_type = dbus_message_iter_get_arg_type(&iter)) != DBUS_TYPE_INVALID)
    {
        if (arg_type == DBUS_TYPE_STRING)
        {
            char* arg{nullptr};
            dbus_message_iter_get_basic(&iter, &arg);
            func(arg_type, &arg);
        }
        else if (arg_type == DBUS_TYPE_INT32)
        {
            int32_t arg{0};
            dbus_message_iter_get_basic(&iter, &arg);
            func(arg_type, &arg);
        }
        else if (arg_type == DBUS_TYPE_BOOLEAN)
        {
            dbus_bool_t arg{FALSE};
            dbus_message_iter_get_basic(&iter, &arg);
            func(arg_type, &arg);
        }

        dbus_message_iter_next(&iter);
    }
}
