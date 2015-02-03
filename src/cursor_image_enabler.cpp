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
 */

#include "cursor_image_enabler.h"

#include "mir/graphics/cursor.h"
#include "mir_toolkit/event.h"

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
    if (mir_event_get_type(&event) != mir_event_type_input)
    {
        return false;
    }
    auto const* ev = mir_event_get_input_event(&event);
    if (mir_input_event_type_pointer == mir_input_event_get_type(ev))
    {
        enable_cursor();
        last_cursor_movement = event.motion.event_time;
    }
    else if (remove_delay &&
             (event.motion.event_time - last_cursor_movement > remove_delay))
    {
        disable_cursor();
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
