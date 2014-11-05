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

#ifndef USC_MOCK_DBUS_SCREEN_OBSERER_H_
#define USC_MOCK_DBUS_SCREEN_OBSERER_H_

#include "src/dbus_screen_observer.h"

#include <gmock/gmock.h>

class MockDBusScreenObserver : public DBusScreenObserver
{
public:
    MOCK_METHOD2(set_screen_power_mode, void(MirPowerMode, PowerStateChangeReason));
    MOCK_METHOD1(toggle_screen_power_mode, void(PowerStateChangeReason));
    MOCK_METHOD1(keep_display_on, void(bool));
    MOCK_METHOD1(set_brightness, void(int));
    MOCK_METHOD1(enable_auto_brightness, void(bool));
    MOCK_METHOD2(set_inactivity_timeouts, void(int, int));
    MOCK_METHOD1(set_touch_visualization_enabled, void(bool));
};

#endif
