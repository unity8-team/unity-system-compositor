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

#ifndef POWERKEY_HANDLER_H_
#define POWERKEY_HANDLER_H_

#include "mir/input/event_filter.h"

#include <mutex>
#include <memory>
#include <atomic>
#include <vector>
#include <chrono>
#include <initializer_list>

namespace mir
{
namespace time
{
class Alarm;
class Timer;
}
}

namespace usc
{
class PowerKeyStateListener;

class PowerKeyHandler : public mir::input::EventFilter
{
public:
    PowerKeyHandler(mir::time::Timer& timer,
                    std::chrono::milliseconds power_key_ignore_timeout,
                    std::chrono::milliseconds shutdown_timeout,
                    std::initializer_list<PowerKeyStateListener*> listener);

    ~PowerKeyHandler();

    bool handle(MirEvent const& event) override;

    static const int32_t POWER_KEY_CODE = 26;

private:
    void power_key_up();
    void power_key_down();
    void shutdown_alarm_notification();
    void long_press_notification();

    enum class KeyState
    {
        Released,
        Pressed,
        LongPressed,
        VeryLongPressed
    };
    bool try_transistion(KeyState from, KeyState to);

    std::atomic<KeyState> power_key;

    std::chrono::milliseconds power_key_ignore_timeout;
    std::chrono::milliseconds shutdown_timeout;

    std::unique_ptr<mir::time::Alarm> shutdown_alarm;
    std::unique_ptr<mir::time::Alarm> long_press_alarm;
    std::vector<PowerKeyStateListener*> const key_state_listener;
};
}

#endif
