/*
 * Copyright © 2013 Canonical Ltd.
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
 * Authored by: Robert Ancell <robert.ancell@canonical.com>
 */

#include "system_compositor.h"

#include <mir/run_mir.h>
#include <mir/abnormal_exit.h>
#include <mir/cached_ptr.h>
#include <mir/default_server_configuration.h>
#include <mir/shell/session.h>
#include <mir/shell/session_container.h>
#include <mir/compositor/compositing_strategy.h>
#include <mir/compositor/renderables.h>
#include <mir/graphics/display_buffer.h>
#include <cstdio>
#include <thread>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/asio.hpp>

namespace msh = mir::shell;
namespace mc = mir::compositor;
namespace mg = mir::graphics;

struct SCCompositingStrategy : mc::CompositingStrategy
{
    SCCompositingStrategy(SystemCompositor &system_compositor) :
        system_compositor(system_compositor)
    {
    }

    void render(mg::DisplayBuffer& display_buffer)
    {
        display_buffer.make_current();
        display_buffer.clear();

        /* Draw the active surface */
        auto active_session = system_compositor.get_active_session();
        if (active_session)
        {
            // Draw it
        }

        display_buffer.post_update();
    }

    SystemCompositor &system_compositor;
};

struct SCRenderables : mc::Renderables
{
    SCRenderables(SystemCompositor &system_compositor) :
        system_compositor(system_compositor)
    {
    }

    void for_each_if(mc::FilterForRenderables& filter, mc::OperatorForRenderables& renderable_operator)
    {
        /* We don't support this interface */
    }

    void set_change_callback(std::function<void()> const& f)
    {
        std::lock_guard<std::mutex> lock{notify_change_mutex};
        assert(f);
        notify_change = f;
    }
  
    void emit_change_notification()
    {
        /* Tell Mir that we need to render */
        std::lock_guard<std::mutex> lock{notify_change_mutex};
        notify_change();     
    }

    SystemCompositor &system_compositor;
    std::mutex notify_change_mutex;
    std::function<void()> notify_change;
};

struct Configuration : mir::DefaultServerConfiguration
{
    Configuration(SystemCompositor &system_compositor, int argc, char const* argv[]) :
        DefaultServerConfiguration(argc, argv),
        system_compositor(system_compositor)
    {
    }

    std::shared_ptr<mc::CompositingStrategy> the_compositing_strategy()
    {
        return compositing_strategy(
            [this]()
            {
                return std::make_shared<SCCompositingStrategy>(system_compositor);
            });
    }

    std::shared_ptr<mc::Renderables> the_renderables()
    {
        return the_system_compositor_renderables();
    }

    std::shared_ptr<SCRenderables> the_system_compositor_renderables()
    {
        return renderables(
            [this]()
            {
                return std::make_shared<SCRenderables>(system_compositor);
            });
    }

    SystemCompositor &system_compositor;
    mir::CachedPtr<SCRenderables> renderables;
};

SystemCompositor::SystemCompositor(int from_dm_fd, int to_dm_fd) :
        dm_connection(io_service, from_dm_fd, to_dm_fd)
{
}

int SystemCompositor::run(int argc, char const* argv[])
{
    /* Generate our configuration for Mir */
    config = std::make_shared<Configuration>(*this, argc, argv);

    /* Run Mir and start a thread for us when it is ready */
    auto return_value = 0;
    try
    {
        mir::run_mir(*config, [this](mir::DisplayServer&)
        {
            thread = std::thread(std::mem_fn(&SystemCompositor::main), this);
        });
    }
    catch (mir::AbnormalExit const& error)
    {
        std::cerr << error.what() << std::endl;
        return_value = 1;
    }
    catch (std::exception const& error)
    {
        std::cerr << "ERROR: " << boost::diagnostic_information(error) << std::endl;
        return_value = 1;
    }

    /* Stop our thread */
    io_service.stop();
    if (thread.joinable())
        thread.join();

    return return_value;
}

std::shared_ptr<mir::shell::Session> SystemCompositor::get_active_session()
{
    return active_session;
}

void SystemCompositor::emit_change_notification()
{
    config->the_system_compositor_renderables()->emit_change_notification();
}

void SystemCompositor::set_active_session(std::string client_name)
{
    std::cerr << "set_active_session" << std::endl;

    /* Find the session with this name */
    std::shared_ptr<msh::Session> session;
    config->the_shell_session_container()->for_each([&client_name, &session](std::shared_ptr<msh::Session> const& s)
    {
        if (s->name() == client_name)
            session = s;
    });

    /* Switch to it and re-render */
    if (session)
    {
        old_session = active_session;
        active_session = session;
        emit_change_notification();
    }
    else
        std::cerr << "Unable to set active session, unknown client name " << client_name << std::endl;
}

void SystemCompositor::main()
{
    /* Connect to the display manager */
    dm_connection.set_handler(this);
    dm_connection.start();
    dm_connection.send_ready();

    /* Wait for events */
    io_service.run();
}
