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

#ifndef POWER_KEY_MEDIATOR_
#define POWER_KEY_MEDIATOR_

#include "powerkey_state_listener.h"

class InactivtyTracker;
class DBusScreenObserver;
class System;

class PowerKeyMediator : public PowerKeyStateListener
{
public:
    PowerKeyMediator(DBusScreenObserver& observer, InactivtyTracker& handler, System& sys);
    void power_key_down() override;
    void power_key_short() override;
    void power_key_long() override;
    void power_key_very_long() override;
    void power_key_up() override;
private:
    DBusScreenObserver& screen_observer;
    InactivtyTracker& inactivity_tracker;
    System& sys;
};

#endif
