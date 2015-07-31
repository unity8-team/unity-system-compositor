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

#include "window_manager.h"

#include "session_monitor.h"

#include "mir/graphics/display.h"
#include "mir/graphics/display_configuration.h"
#include "mir/geometry/rectangle.h"
#include "mir/scene/session.h"
#include "mir/scene/surface.h"
#include "mir/shell/display_layout.h"
#include "mir/shell/focus_controller.h"

#include "mir_toolkit/client_types.h"

#include <iostream>
#include <algorithm>

namespace mf = mir::frontend;
namespace ms = mir::scene;
namespace msh = mir::shell;
namespace mg = mir::graphics;

namespace
{

class UscSession : public usc::Session
{
public:
    UscSession(
        std::shared_ptr<ms::Session> const& scene_session,
        msh::FocusController& focus_controller)
        : scene_session{scene_session},
          focus_controller(focus_controller)
    {
    }

    std::string name()
    {
        return scene_session->name();
    }

    void show() override
    {
        scene_session->show();
    }

    void hide() override
    {
        scene_session->hide();
    }

    void raise_and_focus() override
    {
        auto const surface = scene_session->default_surface();
        focus_controller.raise({surface});
        focus_controller.set_focus_to(scene_session, surface);
    }

    bool corresponds_to(mir::frontend::Session const* s) override
    {
        return scene_session.get() == s;
    }

    std::shared_ptr<ms::Session> const scene_session;
    msh::FocusController& focus_controller;
};
}

usc::WindowManager::WindowManager(
    mir::shell::FocusController* focus_controller,
    std::shared_ptr<mir::graphics::Display> const& display,
    std::shared_ptr<mir::shell::DisplayLayout> const& display_layout,
    std::shared_ptr<ms::SessionCoordinator> const& session_coordinator,
    std::shared_ptr<SessionMonitor> const& session_monitor) :
    mir::shell::SystemCompositorWindowManager{
        focus_controller,
        display_layout,
        session_coordinator},
    session_monitor{session_monitor},
    display{display},
    display_layout{display_layout}
{
}

void usc::WindowManager::on_session_added(std::shared_ptr<mir::scene::Session> const& session) const
{
    std::cerr << "Opening session " << session->name() << std::endl;

    auto const usc_session = std::make_shared<UscSession>(session, *focus_controller);

    session_monitor->add(usc_session, session->process_id());
    sessions.push_back(session);
}

void usc::WindowManager::on_session_removed(std::shared_ptr<mir::scene::Session> const& session) const
{
    std::cerr << "Closing session " << session->name() << std::endl;

    session_monitor->remove(session);

    sessions.erase( std::remove(begin(sessions), end(sessions), session), end(sessions));
}

void usc::WindowManager::on_session_ready(std::shared_ptr<mir::scene::Session> const& session) const
{
    session_monitor->mark_ready(session.get());
}

void usc::WindowManager::add_display(mir::geometry::Rectangle const& area)
{
    resize_scene_to_cloned_display_intersection();
}

void usc::WindowManager::remove_display(mir::geometry::Rectangle const& area)
{
    resize_scene_to_cloned_display_intersection();
}

void usc::WindowManager::resize_scene_to_cloned_display_intersection()
{
    using namespace mir::geometry;
    // Determine display intersection geometry - they should already be cloned and have
    // orientation (i.e. both landscape)

    Size intersection{999999, 999999};
    auto displayConfig = display->configuration();
    displayConfig->for_each_output(
                [&](mg::DisplayConfigurationOutput const& displayConfigOutput){
        if (!displayConfigOutput.connected || !displayConfigOutput.used)
            return;

        Size size = displayConfigOutput.extents().size;
        if (size.width < intersection.width)
            intersection.width = size.width;
        if (size.height < intersection.height)
            intersection.height = size.height;
    });

    std::cout << "Intersection" << intersection.width << intersection.height << std::endl;

    // Now resize all surfaces in scene to this value
    for (auto &session : sessions)
        for (auto s = session->default_surface(); s; s = session->surface_after(s))
        {
            s->resize(intersection);
        }
}
