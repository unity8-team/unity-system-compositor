/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef USC_WINDOW_MANAGER_H_
#define USC_WINDOW_MANAGER_H_

#include <mir/shell/window_manager.h>
#include <vector>

namespace mir
{
namespace graphics { class Display; }
namespace scene { class PlacementStrategy; class SessionCoordinator; }
namespace shell { class FocusController; class DisplayLayout; }
}

namespace usc
{
class SessionSwitcher;

class WindowManager : public mir::shell::WindowManager
{
public:
    explicit WindowManager(
        mir::shell::FocusController* focus_controller,
        std::shared_ptr<mir::graphics::Display> const& display,
        std::shared_ptr<mir::shell::DisplayLayout> const& display_layout,
        std::shared_ptr<mir::scene::SessionCoordinator> const& session_coordinator,
        std::shared_ptr<SessionSwitcher> const& session_switcher);

    void add_session(std::shared_ptr<mir::scene::Session> const& session) override;

    void remove_session(std::shared_ptr<mir::scene::Session> const& session) override;

    mir::frontend::SurfaceId add_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        mir::scene::SurfaceCreationParameters const& params,
        std::function<mir::frontend::SurfaceId(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& params)> const& build) override;

    void modify_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        std::shared_ptr<mir::scene::Surface> const& surface,
        mir::shell::SurfaceSpecification const& modifications) override;

    void remove_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        std::weak_ptr<mir::scene::Surface> const& surface) override;

    void add_display(mir::geometry::Rectangle const& area) override;

    void remove_display(mir::geometry::Rectangle const& area) override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;

    bool handle_touch_event(MirTouchEvent const* event) override;

    bool handle_pointer_event(MirPointerEvent const* event) override;

    int set_surface_attribute(
        std::shared_ptr<mir::scene::Session> const& session,
        std::shared_ptr<mir::scene::Surface> const& surface,
        MirSurfaceAttrib attrib,
        int value) override;

private:
    void resize_scene_to_cloned_display_intersection();

    mir::shell::FocusController* const focus_controller;
    std::shared_ptr<mir::graphics::Display> display;
    std::shared_ptr<mir::shell::DisplayLayout> const display_layout;
    std::shared_ptr<mir::scene::SessionCoordinator> const session_coordinator;
    std::shared_ptr<SessionSwitcher> const session_switcher;
    std::vector<std::shared_ptr<mir::scene::Surface>> surfaces;
};
}

#endif /* USC_WINDOW_MANAGER_H_ */
