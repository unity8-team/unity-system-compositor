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

#include "cursor_enabler.h"

#include "mir/graphics/cursor.h"
#include "mir_toolkit/event.h"

namespace mi = mir::input;

CursorEnabler::CursorEnabler(
    std::shared_ptr<mir::graphics::Cursor> const& cursor,
    std::chrono::milliseconds remove_pointer_timeout)
    : cursor_shown(false), remove_delay{static_cast<uint32_t>(remove_pointer_timeout.count())}, last_cursor_movement(0),
    cursor(cursor)
{
    cursor->hide();
}

CursorEnabler::~CursorEnabler() = default;

bool CursorEnabler::handle(MirEvent const& event)
{
    if (mir_event_get_type(&event) != mir_event_type_input)
    {
        return false;
    }
    auto const* ev = mir_event_get_input_event(&event);
    if (mir_input_event_type_pointer == mir_input_event_get_type(ev))
    {
        enable_cursor();
        last_cursor_movement = mir_input_event_get_event_time(ev);
    }
    else if (remove_delay &&
             mir_input_event_type_touch == mir_input_event_get_type(ev) &&
             (mir_input_event_get_event_time(ev) - last_cursor_movement > remove_delay))
    {
        disable_cursor();
    }
    return false;
}

void CursorEnabler::enable_cursor()
{
    if (!cursor_shown)
    {
        cursor->show();
        cursor_shown = true;
    }
}

void CursorEnabler::disable_cursor()
{
    if (cursor_shown)
    {
        cursor->hide();
        cursor_shown = false;
    }
}
