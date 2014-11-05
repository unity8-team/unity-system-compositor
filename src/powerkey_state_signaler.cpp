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

#include "powerkey_state_signaler.h"

#include <QDBusMessage>
#include <QDBusConnection>

void PowerKeyStateSignaler::power_key_down()
{
    QDBusMessage message =  QDBusMessage::createSignal("/com/canonical/Unity/Screen",
        "com.canonical.Unity.Screen", "powerKeyDown");

    QDBusConnection bus = QDBusConnection::systemBus();
    bus.send(message);
}

void PowerKeyStateSignaler::power_key_up()
{
    QDBusMessage message =  QDBusMessage::createSignal("/com/canonical/Unity/Screen",
        "com.canonical.Unity.Screen", "powerKeyUp");

    QDBusConnection bus = QDBusConnection::systemBus();
    bus.send(message);
}

void PowerKeyStateSignaler::power_key_short()
{
}

void PowerKeyStateSignaler::power_key_long()
{
}

void PowerKeyStateSignaler::power_key_very_long()
{
}
