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

#include "src/screen_event_handler.h"
#include "src/power_state_change_reason.h"
#include "src/screen.h"

#include "advanceable_timer.h"

#include <mir/events/event_builders.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <atomic>

namespace
{

struct MockScreen : usc::Screen
{
    MOCK_METHOD1(enable_inactivity_timers, void(bool enable));
    MOCK_METHOD1(toggle_screen_power_mode, void(PowerStateChangeReason reason));
    MOCK_METHOD0(keep_display_on_temporarily, void());

    MOCK_METHOD2(set_screen_power_mode, void(MirPowerMode mode, PowerStateChangeReason reason));
    MOCK_METHOD1(keep_display_on, void(bool on));
    MOCK_METHOD1(set_brightness, void(int brightness));
    MOCK_METHOD1(enable_auto_brightness, void(bool enable));
    MOCK_METHOD2(set_inactivity_timeouts, void(int power_off_timeout, int dimmer_timeout));

    MOCK_METHOD1(set_touch_visualization_enabled, void(bool enabled));
    MOCK_METHOD1(register_power_state_change_handler, void(
                 usc::PowerStateChangeHandler const& handler));
};

struct AScreenEventHandler : testing::Test
{
    void press_power_key()
    {
        screen_event_handler.handle(*power_key_down_event);
    }

    void release_power_key()
    {
        screen_event_handler.handle(*power_key_up_event);
    }

    void touch_screen()
    {
        screen_event_handler.handle(*touch_event);
    }

    void move_pointer()
    {
        screen_event_handler.handle(*pointer_event);
    }

    static const int32_t POWER_KEY_CODE = 26;
    mir::EventUPtr power_key_down_event = mir::events::make_event(
        MirInputDeviceId{1}, 0, mir_key_input_event_action_down,
        POWER_KEY_CODE, 0, mir_input_event_modifier_none);
    mir::EventUPtr power_key_up_event = mir::events::make_event(
        MirInputDeviceId{1}, 0, mir_key_input_event_action_up,
        POWER_KEY_CODE, 0, mir_input_event_modifier_none);
    mir::EventUPtr touch_event = mir::events::make_event(
        MirInputDeviceId{1}, 0, mir_input_event_modifier_none);
    mir::EventUPtr pointer_event = mir::events::make_event(
        MirInputDeviceId{1}, 0, mir_input_event_modifier_none,
        mir_pointer_input_event_action_motion,
        {}, 0.0f, 0.0f, 0.0f, 0.0f);

    AdvanceableTimer timer;
    std::chrono::milliseconds const power_key_ignore_timeout{5000};
    std::chrono::milliseconds const shutdown_timeout{10000};
    testing::NiceMock<MockScreen> mock_screen;
    std::atomic<bool> shutdown_called{false};
    usc::ScreenEventHandler screen_event_handler{
        timer,
        power_key_ignore_timeout,
        shutdown_timeout,
        [&] { shutdown_called = true; },
        mock_screen};
};

}

TEST_F(AScreenEventHandler, turns_screen_on_on_long_press)
{
    auto const long_press_duration = power_key_ignore_timeout;

    EXPECT_CALL(mock_screen,
                set_screen_power_mode(MirPowerMode::mir_power_mode_on,
                                      PowerStateChangeReason::power_key));

    press_power_key();
    timer.advance_by(long_press_duration);
}

TEST_F(AScreenEventHandler, shuts_down_system_when_power_key_pressed_for_long_enough)
{
    using namespace testing;

    std::chrono::milliseconds const one_ms{1};

    press_power_key();

    timer.advance_by(shutdown_timeout - one_ms);
    ASSERT_FALSE(shutdown_called);

    timer.advance_by(one_ms);
    ASSERT_TRUE(shutdown_called);
}

TEST_F(AScreenEventHandler, turns_screen_off_when_shutting_down)
{
    press_power_key();

    testing::InSequence s;

    // First, screen turns on from long press
    EXPECT_CALL(mock_screen,
                set_screen_power_mode(MirPowerMode::mir_power_mode_on,
                                      PowerStateChangeReason::power_key));
    EXPECT_CALL(mock_screen,
                set_screen_power_mode(MirPowerMode::mir_power_mode_off,
                                      PowerStateChangeReason::power_key));
    timer.advance_by(shutdown_timeout);
}

TEST_F(AScreenEventHandler, keeps_display_on_temporarily_on_touch_event)
{
    EXPECT_CALL(mock_screen, keep_display_on_temporarily());

    touch_screen();
}

TEST_F(AScreenEventHandler, keeps_display_on_temporarily_on_pointer_event)
{
    EXPECT_CALL(mock_screen, keep_display_on_temporarily());

    move_pointer();
}

TEST_F(AScreenEventHandler, toggles_screen_mode_on_normal_press_release)
{
    std::chrono::milliseconds const normal_press_duration{100};

    EXPECT_CALL(mock_screen,
                toggle_screen_power_mode(PowerStateChangeReason::power_key));

    press_power_key();
    timer.advance_by(normal_press_duration);
    release_power_key();
}

TEST_F(AScreenEventHandler, does_not_toggle_screen_mode_on_long_press_release)
{
    using namespace testing;

    auto const long_press_duration = power_key_ignore_timeout;

    EXPECT_CALL(mock_screen,
                toggle_screen_power_mode(_))
        .Times(0);

    press_power_key();
    timer.advance_by(long_press_duration);
    release_power_key();
}

TEST_F(AScreenEventHandler, passes_through_all_handled_events)
{
    using namespace testing;

    EXPECT_FALSE(screen_event_handler.handle(*power_key_down_event));
    EXPECT_FALSE(screen_event_handler.handle(*power_key_up_event));
    EXPECT_FALSE(screen_event_handler.handle(*touch_event));
    EXPECT_FALSE(screen_event_handler.handle(*pointer_event));
}
