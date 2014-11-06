/*
 * Copyright © 2013-2014 Canonical Ltd.
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

#ifndef USC_SCREEN_POWER_STATE_LISTENER_H_
#define USC_SCREEN_POWER_STATE_LISTENER_H_

#include <mir_toolkit/common.h>

enum class PowerStateChangeReason;

namespace usc
{

class ScreenPowerStateListener
{
public:
    ScreenPowerStateListener() = default;
    virtual ~ScreenPowerStateListener() = default;
    virtual void power_state_change(MirPowerMode mode, PowerStateChangeReason reason) = 0;
private:
    ScreenPowerStateListener(ScreenPowerStateListener const&) = delete;
    ScreenPowerStateListener& operator=(ScreenPowerStateListener const&) = delete;
};

}

#endif
