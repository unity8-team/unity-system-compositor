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

#ifndef USC_CURSOR_IMAGE_ENABLER_H_
#define USC_CURSOR_IMAGE_ENABLER_H_

#include "mir/input/event_filter.h"

#include <memory>
#include <chrono>

namespace mir
{
namespace graphics
{
class Cursor;
class CursorImage;
}
}

/*
 * Disables the cursor on construction and waits for the first mir pointer/cursor
 * event to come by for enabling the cursor again.
 */
class CursorImageEnabler : public mir::input::EventFilter
{
public:
    CursorImageEnabler(std::shared_ptr<mir::graphics::Cursor> const& cursor,
                       std::shared_ptr<mir::graphics::CursorImage> const& default_image,
                       std::chrono::milliseconds remove_pointer_timeout);

    ~CursorImageEnabler();

    bool handle(MirEvent const& event) override;

private:
    void enable_cursor();
    void disable_cursor();

    bool cursor_shown;
    uint32_t remove_delay;
    uint32_t last_cursor_movement;
    std::shared_ptr<mir::graphics::Cursor> const cursor;
    std::shared_ptr<mir::graphics::CursorImage> const default_image; // unpleasent
};

#endif
