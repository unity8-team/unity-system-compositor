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

#include "cursor_image_enabler.h"

#include "mir/graphics/cursor.h"

namespace mi = mir::input;

CursorImageEnabler::CursorImageEnabler(
    std::shared_ptr<mir::graphics::Cursor> const& cursor,
    std::shared_ptr<mir::graphics::CursorImage> const& default_image,
    std::chrono::milliseconds remove_pointer_timeout)
    : cursor_shown(false), remove_delay{static_cast<uint32_t>(remove_pointer_timeout.count())}, last_cursor_movement(0),
    cursor(cursor), default_image(default_image)
{
    cursor->hide();
}

CursorImageEnabler::~CursorImageEnabler() = default;

bool CursorImageEnabler::handle(MirEvent const& event)
{
    // TODO this needs update when MirEvent 2.0: pointer events land
    if (event.type == mir_event_type_motion)
    {
        if (event.motion.pointer_count > 0 && (
                event.motion.pointer_coordinates[0].tool_type == mir_motion_tool_type_mouse ||
                event.motion.pointer_coordinates[0].tool_type == mir_motion_tool_type_stylus ))
        {
            enable_cursor();
            last_cursor_movement = event.motion.event_time;
        }
        else if (remove_delay &&
                (event.motion.event_time - last_cursor_movement > remove_delay))
        {
            disable_cursor();
        }
    }
    return false;
}

void CursorImageEnabler::enable_cursor()
{
    if (!cursor_shown)
    {
        cursor->show(*default_image);
        cursor_shown = true;
    }
}

void CursorImageEnabler::disable_cursor()
{
    if (cursor_shown)
    {
        cursor->hide();
        cursor_shown = false;
    }
}
