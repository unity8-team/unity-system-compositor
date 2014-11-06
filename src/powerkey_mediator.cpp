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

#include "powerkey_mediator.h"
#include "dbus_screen_observer.h"
#include "power_state_change_reason.h"
#include "inactivty_tracker.h"
#include "system.h"

usc::PowerKeyMediator::PowerKeyMediator(DBusScreenObserver & observer, InactivtyTracker & tracker, System& sys)
    : screen_observer{observer}, inactivity_tracker(tracker), sys(sys)
{
}

void usc::PowerKeyMediator::power_key_down()
{
    inactivity_tracker.enable_inactivity_timers(false);
}

void usc::PowerKeyMediator::power_key_short()
{
    if (screen_toggle_enabled)
        screen_observer.toggle_screen_power_mode(PowerStateChangeReason::power_key);
}

void usc::PowerKeyMediator::power_key_long()
{
    // screen is turned on so that the shutdown/restart/cancel dialog that is displayed
    // by the shell can be seen
    screen_observer.set_screen_power_mode(
        MirPowerMode::mir_power_mode_on, PowerStateChangeReason::power_key);
}

void usc::PowerKeyMediator::power_key_very_long()
{
    screen_observer.set_screen_power_mode(
        MirPowerMode::mir_power_mode_off, PowerStateChangeReason::power_key);
    sys.shutdown();
}

void usc::PowerKeyMediator::power_key_up()
{
}

void usc::PowerKeyMediator::enable_screen_toggle()
{
    screen_toggle_enabled = true;
}

void usc::PowerKeyMediator::disable_screen_toggle()
{
    screen_toggle_enabled = false;
}
