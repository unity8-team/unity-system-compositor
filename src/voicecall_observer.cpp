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
 * Authored by: Andreas Pokorny <andreas.pokorny@canonical.com>
 */

#include "voicecall_observer.h"
#include "call_state_listener.h"

#include <QtDBus/QtDBus>
#include <QtDBus/QtDBus>

#include <iostream>
#include <chrono>

namespace
{
struct Modem
{
    QDBusObjectPath path;
    QVariantMap properties;
};
typedef QList<Modem> ModemList;

inline std::ostream & log()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto val = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return std::cout << val << ":";
}

}

Q_DECLARE_METATYPE(Modem)
Q_DECLARE_METATYPE(ModemList)

QDBusArgument &operator<<(QDBusArgument& argument, Modem const& modem)
{
    argument.beginStructure();
    argument << modem.path << modem.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(QDBusArgument const& argument, Modem &modem)
{
    argument.beginStructure();
    argument >> modem.path >> modem.properties;
    argument.endStructure();
    return argument;
}

usc::VoiceCallObserver::VoiceCallObserver(CallStateListener& listener)
    : listener{listener}, active_calls(0)
{
    log() << __FUNCTION__ << std::endl;
    qDBusRegisterMetaType<Modem>();
    qDBusRegisterMetaType<ModemList>();

    query_existing_modems();
    subscribe_to_modem_manager();
}

void usc::VoiceCallObserver::query_existing_modems()
{
    log() << __FUNCTION__ << std::endl;
    auto request = QDBusMessage::createMethodCall("org.ofono",
					     "/", "org.ofono.Manager",
					     "GetModems");
    QDBusReply<ModemList> reply = QDBusConnection::systemBus().call(request);

    ModemList modems = reply;
    foreach(Modem modem, modems)
	subscribe_to_call_manager(modem.path.path());
}


void usc::VoiceCallObserver::subscribe_to_modem_manager()
{
    QDBusConnection::systemBus().connect("org.ofono","/","org.ofono.Manager",
					 "ModemAdded", this,
					 SLOT(modem_added(QDBusObjectPath const&, QVariantMap const&)));

}
void usc::VoiceCallObserver::subscribe_to_call_manager(QString const& modem)
{
    log() << __FUNCTION__ << modem.toStdString() << std::endl;
    QDBusConnection::systemBus().connect("org.ofono", modem, "org.ofono.VoiceCallManager",
                                         "CallAdded", this, SLOT(call_added(QDBusObjectPath const&, QVariantMap const&)));
    QDBusConnection::systemBus().connect("org.ofono", modem, "org.ofono.VoiceCallManager",
                                         "CallRemoved", this, SLOT(call_removed(QDBusObjectPath const&, QVariantMap const&)));

}

void usc::VoiceCallObserver::modem_added(QDBusObjectPath const& path, QVariantMap const&)
{
    log() << __FUNCTION__ << std::endl;
    subscribe_to_call_manager(path.path());
}


void usc::VoiceCallObserver::call_added(QDBusObjectPath const&, QVariantMap const& )
{
    log() << __FUNCTION__ << std::endl;
    if (++active_calls == 1)
    {
        listener.call_active();
    }
}

void usc::VoiceCallObserver::call_removed(QDBusObjectPath const&, QVariantMap const&)
{
    log() << __FUNCTION__ << std::endl;
    if (--active_calls == 0)
    {
        listener.no_call_active();
    }
}

