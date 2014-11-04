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
 */

#include "powerkey_handler.h"
#include "powerkey_state_listener.h"

#include <mir/time/timer.h>

namespace mi = mir::input;

PowerKeyHandler::PowerKeyHandler(mir::time::Timer& timer,
                                 std::chrono::milliseconds power_key_ignore_timeout,
                                 std::chrono::milliseconds shutdown_timeout,
                                 PowerKeyStateListener& listener)
    : power_key{KeyState::Released},
      power_key_ignore_timeout{power_key_ignore_timeout},
      shutdown_timeout{shutdown_timeout},
      shutdown_alarm{timer.create_alarm([this]{ shutdown_alarm_notification(); })},
      long_press_alarm{timer.create_alarm([this]{ long_press_notification(); })},
      key_state_listener{&listener}
{
    if (power_key_ignore_timeout > shutdown_timeout)
        throw std::invalid_argument("shutdown timeout must be larger than power key ignore timeout");
    if (power_key_ignore_timeout.count() == 0 ||
        shutdown_timeout.count() == 0)
        throw std::invalid_argument("timeouts must be non-zero");
}

PowerKeyHandler::~PowerKeyHandler() = default;

bool PowerKeyHandler::handle(MirEvent const& event)
{
    if (event.type == mir_event_type_key &&
        event.key.key_code == POWER_KEY_CODE)
    {
        if (event.key.action == mir_key_action_down)
            power_key_down();
        else if (event.key.action == mir_key_action_up)
            power_key_up();
    }
    return false;
}

void PowerKeyHandler::power_key_down()
{
    if (try_transistion(KeyState::Released, KeyState::Pressed))
    {
        long_press_alarm->reschedule_in(power_key_ignore_timeout);
        shutdown_alarm->reschedule_in(shutdown_timeout);
        key_state_listener->power_key_down();
    }
}

void PowerKeyHandler::power_key_up()
{
    if (try_transistion(KeyState::Pressed, KeyState::Released))
    {
        shutdown_alarm->cancel();
        long_press_alarm->cancel();
        key_state_listener->power_key_short();
    }
    else if (try_transistion(KeyState::LongPressed, KeyState::Released))
    {
        shutdown_alarm->cancel();
    }
    else if (try_transistion(KeyState::VeryLongPressed, KeyState::Released))
    {
        // both alarms already fired
    }
    key_state_listener->power_key_up();
}

void PowerKeyHandler::shutdown_alarm_notification()
{
    if (try_transistion(KeyState::LongPressed, KeyState::VeryLongPressed))
    {
        key_state_listener->power_key_very_long();
    }
}

void PowerKeyHandler::long_press_notification()
{
    if (try_transistion(KeyState::Pressed, KeyState::LongPressed))
    {
        key_state_listener->power_key_long();
    }
}

bool PowerKeyHandler::try_transistion(KeyState from, KeyState to)
{
    return std::atomic_compare_exchange_strong(&power_key, &from, to);
}
