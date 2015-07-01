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
 * Authored by: Andreas Pokorny <andreas.pokorny@canonical.com>
 */

#define MIR_INCLUDE_DEPRECATED_EVENT_HEADER

#include "src/cursor_enabler.h"

#include "mir_toolkit/event.h"
#include "mir/events/event_builders.h"
#include "mir/graphics/cursor.h"
#include "mir/graphics/cursor_image.h"
#include "mir_toolkit/events/input/input_event.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <vector>
#include <tuple>
#include <cstring>

namespace mev = mir::events;

namespace
{
struct MockCursor : mir::graphics::Cursor
{
    MOCK_METHOD0(show, void());
    MOCK_METHOD1(show, void(mir::graphics::CursorImage const&));
    MOCK_METHOD0(hide, void());
    MOCK_METHOD1(move_to, void(mir::geometry::Point));
};

struct TestCursorEnabler : ::testing::Test
{
    std::shared_ptr<::testing::NiceMock<MockCursor>> mock_cursor = std::make_shared<::testing::NiceMock<MockCursor>>();
    std::chrono::milliseconds never_remove_pointer{0};
    std::chrono::milliseconds remove_after_five_seconds_of_non_pointing_events{5000};

    MirEvent *mouse_event;
    MirEvent *touch_event;
    MirEvent *key_event;
    TestCursorEnabler()
    {
        // Naughty Gerry, dropping pointers!
        mouse_event = mev::make_event(mir_input_event_type_pointer, 10, mir_input_event_modifier_none,
                                      mir_pointer_action_motion, {}, 1, 1, 0, 0).release();

        touch_event = mev::make_event(mir_input_event_type_touch, 6000, 0).release();
        mev::add_touch(*touch_event, 0, mir_touch_action_change, mir_touch_tooltype_unknown, 1, 1, 0, 0, 0, 0);

        key_event = mev::make_event(mir_input_event_type_key, 6000, mir_keyboard_action_up, 0, 0, mir_input_event_modifier_none).release();
    }
};
}

TEST_F(TestCursorEnabler, disables_cursor_on_start)
{
    using namespace testing;
    EXPECT_CALL(*mock_cursor, hide());

    CursorEnabler enabler(mock_cursor, never_remove_pointer);
}

TEST_F(TestCursorEnabler, enable_cursor_on_mouse_event)
{
    using namespace testing;
    EXPECT_CALL(*mock_cursor, hide());
    EXPECT_CALL(*mock_cursor, show());

    CursorEnabler enabler(mock_cursor, never_remove_pointer);
    enabler.handle(*mouse_event);
}

TEST_F(TestCursorEnabler, disable_cursor_on_touch_event)
{
    using namespace testing;
    InSequence seq;
    EXPECT_CALL(*mock_cursor, hide());
    EXPECT_CALL(*mock_cursor, show());
    EXPECT_CALL(*mock_cursor, hide());

    CursorEnabler enabler(mock_cursor, remove_after_five_seconds_of_non_pointing_events);
    enabler.handle(*mouse_event);
    enabler.handle(*touch_event);
}

