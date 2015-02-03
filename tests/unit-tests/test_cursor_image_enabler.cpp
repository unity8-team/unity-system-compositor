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

#include "src/cursor_image_enabler.h"

#include "mir_toolkit/event.h"
#include "mir/graphics/cursor.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <vector>
#include <tuple>

namespace
{
class MockCursor : public mir::graphics::Cursor
{
    MOCK_METHOD1(show, void(CursorImage const&));
    MOCK_METHOD1(hide, void());
    MOCK_METHOD1(move_to, void(geometry::Point));
};

class StubCursorImage : public mir::graphics::CursorImage
{
    void const *as_argb_8888() const override { return nullptr; }
    void mir::geometry::Size size() const override
    {
        return mir::geometry::Size{0, 0};
    }
    void mir::geometry::Displacement hotspot() const override
    {
        return mir::geometry::Displacement{0, 0};
    }
};

class TestCursorImageEnabler : public ::testing::Test
{
    std::shared_ptr<::testing::NiceMock<MockCursor>> mock_cursor = std::make_shared<::testing::NiceMock<MockCursor>>();
    std::shared_ptr<StubCursorImage> cursor_image = std::make_shared<StubCursorImage>();
    std::chrono::milliseconds never_remove_pointer{0};
    std::chrono::milliseconds remove_after_five_seconds_of_non_pointing_events{5000};

    MirEvent mouse_event;
    MirEvent touch_event;
    MirEvent key_event;
    TestCursorImageEnabler()
    {
        std::memset(&mouse_event, 0, sizeof mouse_event);
        std::memset(&touch_event, 0, sizeof touch_event);
        std::memset(&key_event, 0, sizeof key_event);

        mouse_event.type = mir_event_type_motion;
        mouse_event.motion.pointer_count = 1;
        mouse_event.motion.event_time = 0;
        mouse_event.motion.pointer_coordinates[0].tool_type = mir_motion_tool_type_mouse;

        touch_event.type = mir_event_type_motion;
        touch_event.motion.pointer_count = 1;
        touch_event.motion.event_time = 6000;
        touch_event.motion.pointer_coordinates[0].tool_type = mir_motion_tool_type_finger;

        key_event.type = mir_event_type_key;
        key_event.key.event_time = 6000;
    }
};
}

TEST_F(TestCursorImageEnabler, diables_cursor_on_start)
{
    using namespace testing;
    EXPECT_CALL(mock_cursor, hide());

    CursorImageEnabler enabler(mock_cursor, cursor_image, never_remove_pointer);
}

TEST_F(TestCursorImageEnabler, enable_cursor_on_mouse_event)
{
    using namespace testing;
    EXPECT_CALL(mock_cursor, hide());
    EXPECT_CALL(mock_cursor, show(cursor_image));

    CursorImageEnabler enabler(mock_cursor, cursor_image, never_remove_pointer);
    enabler.handle(mouse_event);
}

TEST_F(TestCursorImageEnabler, disable_cursor_on_touch_event)
{
    using namespace testing;
    EXPECT_CALL(mock_cursor, hide());
    EXPECT_CALL(mock_cursor, show(cursor_image));
    EXPECT_CALL(mock_cursor, hide());

    CursorImageEnabler enabler(mock_cursor, cursor_image, remove_after_five_seconds_of_non_pointing_events);
    enabler.handle(mouse_event);
    enabler.handle(touch_event);
}

#if 0
#endif
