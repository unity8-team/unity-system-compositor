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

#include "src/powerkey_handler.h"
#include "src/powerkey_state_listener.h"
#include "mir/time/alarm.h"
#include "mir/time/timer.h"

#include "gmock-fixes.h"
#include <gtest/gtest.h>

namespace mti = mir::time;

class MockAlarm : public mir::time::Alarm
{
public:
    std::function<void()> callback;

    MOCK_METHOD0(cancel,bool());
    MOCK_CONST_METHOD0(state, State());
    MOCK_METHOD1(reschedule_in, bool(std::chrono::milliseconds));
    MOCK_METHOD1(reschedule_for, bool(mti::Timestamp));
};

class MockTimer : public mti::Timer
{
public:
    MOCK_METHOD2(notify_in,
                 std::unique_ptr<mti::Alarm>(std::chrono::milliseconds,
                                             std::function<void()>));
    MOCK_METHOD2(notify_at, std::unique_ptr<mti::Alarm>(mti::Timestamp,
                                                        std::function<void()>));
    MOCK_METHOD1(create_alarm, std::unique_ptr<mti::Alarm>(std::function<void()>));
};

class MockPowerKeyStateListener : public PowerKeyStateListener
{
public:
    MOCK_METHOD0(power_key_down,void());
    MOCK_METHOD0(power_key_short, void());
    MOCK_METHOD0(power_key_long, void());
    MOCK_METHOD0(power_key_very_long, void());
    MOCK_METHOD0(power_key_up, void());
};

class TestPowerKeyHandler : public ::testing::Test
{
public:
    TestPowerKeyHandler()
    {
        using namespace ::testing;
        ON_CALL(mock_timer,create_alarm(_))
            .WillByDefault(Invoke([this](std::function<void()> callback) -> std::unique_ptr<mti::Alarm>
                                  {return create_alarm(callback);} )
                           );

        power_key_down_event.key.action = mir_key_action_down;
        power_key_down_event.key.key_code = PowerKeyHandler::POWER_KEY_CODE;

        power_key_up_event.key.action = mir_key_action_up;
        power_key_up_event.key.key_code = PowerKeyHandler::POWER_KEY_CODE;

        non_power_key.key.action = mir_key_action_down;
        non_power_key.key.key_code = PowerKeyHandler::POWER_KEY_CODE + 1;
    }

    std::unique_ptr<mti::Alarm> create_alarm(std::function<void()> callback)
    {
        // TODO find a better way to keep track of alarms created by KeyStateHandler
        if (!long_timeout)
        {
            long_timeout = new ::testing::NiceMock<MockAlarm>;
            long_timeout->callback = callback;
            // potentially violating against unique_ptr here
            return std::unique_ptr<mti::Alarm>(long_timeout);
        }
        if(!short_timeout)
        {
            short_timeout = new ::testing::NiceMock<MockAlarm>;
            short_timeout->callback = callback;
            // potentially violating against unique_ptr here
            return std::unique_ptr<mti::Alarm>(short_timeout);
        }
        return nullptr;
    }

    ::testing::NiceMock<MockAlarm> * short_timeout = nullptr;
    ::testing::NiceMock<MockAlarm> * long_timeout = nullptr;
    ::testing::NiceMock<MockTimer> mock_timer;
    ::testing::NiceMock<MockPowerKeyStateListener> mock_key_state_listener;

    std::chrono::milliseconds short_delay{1};
    std::chrono::milliseconds long_delay{2};
    std::chrono::milliseconds zero_timeout{0};

    MirEvent power_key_down_event{ mir_event_type_key };
    MirEvent power_key_up_event{ mir_event_type_key };
    MirEvent non_power_key{ mir_event_type_key };
};

TEST_F(TestPowerKeyHandler, creates_two_alarms)
{
    using namespace ::testing;
    EXPECT_CALL(mock_timer,create_alarm(_)).Times(2);
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);
}

TEST_F(TestPowerKeyHandler, schedules_timers_on_power_key_down)
{
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    EXPECT_CALL(*short_timeout,reschedule_in(short_delay)).Times(1);
    EXPECT_CALL(*long_timeout,reschedule_in(long_delay)).Times(1);

    key_handler.handle(power_key_down_event);
}

TEST_F(TestPowerKeyHandler, re_schedules_and_cancel_timers)
{
    using namespace ::testing;
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    Sequence s;
    EXPECT_CALL(*short_timeout,reschedule_in(short_delay)).InSequence(s);
    EXPECT_CALL(*long_timeout,reschedule_in(long_delay)).InSequence(s);
    EXPECT_CALL(*long_timeout,cancel()).InSequence(s);
    EXPECT_CALL(*short_timeout,cancel()).InSequence(s);
    EXPECT_CALL(*short_timeout,reschedule_in(short_delay)).InSequence(s);
    EXPECT_CALL(*long_timeout,reschedule_in(long_delay)).InSequence(s);
    EXPECT_CALL(*long_timeout,cancel()).InSequence(s);
    EXPECT_CALL(*short_timeout,cancel()).InSequence(s);
    EXPECT_CALL(*short_timeout,reschedule_in(short_delay)).InSequence(s);
    EXPECT_CALL(*long_timeout,reschedule_in(long_delay)).InSequence(s);
    EXPECT_CALL(*long_timeout,cancel()).InSequence(s);
    EXPECT_CALL(*short_timeout,cancel()).InSequence(s);

    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_up_event);
    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_up_event);
    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_up_event);
}

TEST_F(TestPowerKeyHandler, never_filters_key_events)
{
    using namespace testing;
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    EXPECT_THAT(key_handler.handle(power_key_down_event), Eq(false));
    EXPECT_THAT(key_handler.handle(power_key_up_event), Eq(false));
    EXPECT_THAT(key_handler.handle(non_power_key), Eq(false));
}

TEST_F(TestPowerKeyHandler, cancels_timers_on_power_key_up)
{
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    EXPECT_CALL(*long_timeout,cancel()).Times(1);
    EXPECT_CALL(*short_timeout,cancel()).Times(1);

    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_up_event);
}

TEST_F(TestPowerKeyHandler, throws_on_wrong_timeouts)
{
    EXPECT_THROW({
    PowerKeyHandler key_handler(mock_timer, long_delay, short_delay, mock_key_state_listener);
    }, std::invalid_argument);
}

TEST_F(TestPowerKeyHandler, ignores_spurious_up_down_events)
{
    using namespace ::testing;
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    Sequence seq;

    EXPECT_CALL(*short_timeout,reschedule_in(short_delay))
        .InSequence(seq);
    EXPECT_CALL(*long_timeout,reschedule_in(long_delay))
        .InSequence(seq);
    EXPECT_CALL(*long_timeout,cancel())
        .InSequence(seq);
    EXPECT_CALL(*short_timeout,cancel())
        .InSequence(seq);

    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_up_event);
    key_handler.handle(power_key_up_event);
    key_handler.handle(power_key_up_event);
}

TEST_F(TestPowerKeyHandler, forwards_down_events_to_key_state_listener)
{
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    EXPECT_CALL(mock_key_state_listener, power_key_down());
    key_handler.handle(power_key_down_event);
}

TEST_F(TestPowerKeyHandler, forwards_up_events_to_key_state_listener)
{
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    EXPECT_CALL(mock_key_state_listener, power_key_up());
    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_up_event);
}

TEST_F(TestPowerKeyHandler, down_short_up_up_comes_in_sequence)
{
    using namespace ::testing;
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    Sequence s;

    EXPECT_CALL(mock_key_state_listener, power_key_down()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_short()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_up()).InSequence(s);
    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_up_event);
}

TEST_F(TestPowerKeyHandler, down_long_up_comes_in_sequence)
{
    using namespace ::testing;
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    Sequence s;

    EXPECT_CALL(mock_key_state_listener, power_key_down()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_long()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_up()).InSequence(s);

    key_handler.handle(power_key_down_event);
    short_timeout->callback();
    key_handler.handle(power_key_up_event);
}

TEST_F(TestPowerKeyHandler, down_very_long_up_comes_in_sequence)
{
    using namespace ::testing;
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    Sequence s;

    EXPECT_CALL(mock_key_state_listener, power_key_down()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_long()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_very_long()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_up()).InSequence(s);

    key_handler.handle(power_key_down_event);
    short_timeout->callback();
    long_timeout->callback();
    key_handler.handle(power_key_up_event);
}

TEST_F(TestPowerKeyHandler, spurious_alarm_callbacks_dont_cause_calls)
{
    using namespace ::testing;
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    Sequence s;

    EXPECT_CALL(mock_key_state_listener, power_key_down()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_short()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_up()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_very_long()).Times(0);
    EXPECT_CALL(mock_key_state_listener, power_key_long()).Times(0);

    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_up_event);

    long_timeout->callback();
    long_timeout->callback();
    short_timeout->callback();
    short_timeout->callback();
}

TEST_F(TestPowerKeyHandler, spurious_alarm_callbacks_dont_cause_calls_after_first_timeout)
{
    using namespace ::testing;
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    Sequence s;

    EXPECT_CALL(mock_key_state_listener, power_key_down()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_long()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_up()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_very_long()).Times(0);
    EXPECT_CALL(mock_key_state_listener, power_key_short()).Times(0);

    key_handler.handle(power_key_down_event);
    short_timeout->callback();
    key_handler.handle(power_key_up_event);

    long_timeout->callback();
    long_timeout->callback();
    short_timeout->callback();
    short_timeout->callback();
}

TEST_F(TestPowerKeyHandler, multiple_sequences_work)
{
    using namespace ::testing;
    PowerKeyHandler key_handler(mock_timer, short_delay, long_delay, mock_key_state_listener);

    Sequence s;

    EXPECT_CALL(mock_key_state_listener, power_key_down()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_long()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_up()).InSequence(s);

    EXPECT_CALL(mock_key_state_listener, power_key_down()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_short()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_up()).InSequence(s);

    EXPECT_CALL(mock_key_state_listener, power_key_down()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_long()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_very_long()).InSequence(s);
    EXPECT_CALL(mock_key_state_listener, power_key_up()).InSequence(s);

    key_handler.handle(power_key_down_event);
    short_timeout->callback();
    key_handler.handle(power_key_up_event);

    key_handler.handle(power_key_down_event);
    key_handler.handle(power_key_up_event);

    key_handler.handle(power_key_down_event);
    short_timeout->callback();
    long_timeout->callback();
    key_handler.handle(power_key_up_event);
}

TEST_F(TestPowerKeyHandler, throws_on_zero_timeouts)
{
    using namespace ::testing;
    EXPECT_THROW({
                 PowerKeyHandler key_handler(mock_timer, zero_timeout, long_delay, mock_key_state_listener);
        }, std::invalid_argument
        );
    EXPECT_THROW({
                 PowerKeyHandler key_handler(mock_timer, short_delay, zero_timeout, mock_key_state_listener);
        }, std::invalid_argument
        );
}
