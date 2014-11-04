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

#include "src/powerkey_mediator.h"
#include "src/inactivty_tracker.h"
#include "src/dbus_screen_observer.h"
#include "src/power_state_change_reason.h"
#include "src/system.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

class MockInactivityTracker : public InactivtyTracker
{
public:
    MOCK_METHOD1(enable_inactivity_timers, void(bool));
};

class MockSystem : public System
{
public:
    MOCK_METHOD0(shutdown,void());
};

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

class PowerKeyMediatorTest : public ::testing::Test
{
public:
    ::testing::NiceMock<MockInactivityTracker> mock_tracker;
    ::testing::NiceMock<MockDBusScreenObserver> mock_screen_observer;
    ::testing::NiceMock<MockSystem> mock_system;
    PowerKeyMediator key_mediator{mock_screen_observer, mock_tracker, mock_system};
};

TEST_F(PowerKeyMediatorTest, power_key_down_disables_inactivity_timers)
{
    EXPECT_CALL(mock_tracker, enable_inactivity_timers(false)).Times(3);

    key_mediator.power_key_down();
    key_mediator.power_key_short();
    key_mediator.power_key_up();

    key_mediator.power_key_down();
    key_mediator.power_key_long();
    key_mediator.power_key_up();

    key_mediator.power_key_down();
    key_mediator.power_key_very_long();
    key_mediator.power_key_up();
}

TEST_F(PowerKeyMediatorTest, power_key_short_sequence_toggles_screen_power_state)
{
    EXPECT_CALL(mock_screen_observer, toggle_screen_power_mode(PowerStateChangeReason::power_key));

    key_mediator.power_key_down();
    key_mediator.power_key_short();
    key_mediator.power_key_up();
}

TEST_F(PowerKeyMediatorTest, power_key_long_sequence_forces_power_on_screen_state)
{
    EXPECT_CALL(mock_screen_observer, set_screen_power_mode(
        MirPowerMode::mir_power_mode_on, PowerStateChangeReason::power_key));

    key_mediator.power_key_down();
    key_mediator.power_key_long();
    key_mediator.power_key_up();
}

TEST_F(PowerKeyMediatorTest, power_key_very_long_sequence_forces_power_off_screen_state_and_shutdown)
{
    EXPECT_CALL(mock_screen_observer, set_screen_power_mode(
        MirPowerMode::mir_power_mode_off, PowerStateChangeReason::power_key));
    EXPECT_CALL(mock_system,shutdown());

    key_mediator.power_key_down();
    key_mediator.power_key_very_long();
    key_mediator.power_key_up();
}
