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
#include <mir/graphics/display.h>
#include <mir/graphics/display_buffer.h>

#include <cstdio>
#include <thread>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/asio.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLES2/gl2.h>

namespace msh = mir::shell;
namespace mc = mir::compositor;
namespace mg = mir::graphics;
namespace geom = mir::geometry;

const GLchar* vertex_shader_src =
{
    "attribute vec3 position;\n"
    "attribute vec2 texcoord;\n"
    "uniform mat4 screen_to_gl_coords;\n"
    "uniform mat4 transform;\n"
    "varying vec2 v_texcoord;\n"
    "void main() {\n"
    "   gl_Position = screen_to_gl_coords * transform * vec4(position, 1.0);\n"
    "   v_texcoord = texcoord;\n"
    "}\n"
};

const GLchar* fragment_shader_src =
{
    "precision mediump float;\n"
    "uniform sampler2D tex;\n"
    "uniform float alpha;\n"
    "varying vec2 v_texcoord;\n"
    "void main() {\n"
    "   vec4 frag = texture2D(tex, v_texcoord);\n"
    "   gl_FragColor = vec4(frag.xyz, frag.a * alpha);\n"
    "}\n"
};

struct VertexAttributes
{
    glm::vec3 position;
    glm::vec2 texcoord;
};

/*
 * The texture coordinates are y-inverted to account for the difference in the
 * texture and renderable pixel data row order. In particular, GL textures
 * expect pixel data in rows starting from the bottom and moving up the image,
 * whereas our renderables provide data in rows starting from the top and
 * moving down the image.
 */
VertexAttributes vertex_attribs[4] =
{
    {
        glm::vec3{-0.5f, -0.5f, 0.0f},
        glm::vec2{0.0f, 0.0f}
    },
    {
        glm::vec3{-0.5f, 0.5f, 0.0f},
        glm::vec2{0.0f, 1.0f},
    },
    {
        glm::vec3{0.5f, -0.5f, 0.0f},
        glm::vec2{1.0f, 0.0f},
    },
    {
        glm::vec3{0.5f, 0.5f, 0.0f},
        glm::vec2{1.0f, 1.0f}
    }
};

struct SCCompositingStrategy : mc::CompositingStrategy
{
    SCCompositingStrategy(SystemCompositor &system_compositor, const geom::Size& display_size) :
        system_compositor(system_compositor)
    {
        setup(display_size);
    }

    ~SCCompositingStrategy()
    {
        cleanup();
    }

    void render(mg::DisplayBuffer& display_buffer)
    {
        display_buffer.make_current();
        display_buffer.clear();

        draw();

        display_buffer.post_update();
    }

    SystemCompositor &system_compositor;

private:
    void setup(const geom::Size& display_size)
    {
        GLint param = 0;

        /* Create shaders and program */
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_src, 0);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &param);
        if (param == GL_FALSE)
        {
            GetObjectLogAndThrow(glGetShaderInfoLog,
                glGetShaderiv,
                "Failed to compile vertex shader:",
                vertex_shader);
        }

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_src, 0);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &param);
        if (param == GL_FALSE)
        {
            GetObjectLogAndThrow(glGetShaderInfoLog,
                glGetShaderiv,
                "Failed to compile fragment shader:",
                fragment_shader);
        }

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &param);
        if (param == GL_FALSE)
        {
            GetObjectLogAndThrow(glGetProgramInfoLog,
                glGetProgramiv,
                "Failed to link program:",
                program);
        }

        glUseProgram(program);

        /* Set up program variables */
        GLint mat_loc = glGetUniformLocation(program, "screen_to_gl_coords");
        GLint tex_loc = glGetUniformLocation(program, "tex");
        transform_uniform_loc = glGetUniformLocation(program, "transform");
        alpha_uniform_loc = glGetUniformLocation(program, "alpha");
        position_attr_loc = glGetAttribLocation(program, "position");
        texcoord_attr_loc = glGetAttribLocation(program, "texcoord");

        /*
         * Create and set screen_to_gl_coords transformation matrix.
         * The screen_to_gl_coords matrix transforms from the screen coordinate system
         * (top-left is (0,0), bottom-right is (W,H)) to the normalized GL coordinate system
         * (top-left is (-1,1), bottom-right is (1,-1))
         */
        glm::mat4 screen_to_gl_coords = glm::translate(glm::mat4{1.0f}, glm::vec3{-1.0f, 1.0f, 0.0f});
        screen_to_gl_coords = glm::scale(screen_to_gl_coords,
                glm::vec3{2.0f / display_size.width.as_uint32_t(),
                -2.0f / display_size.height.as_uint32_t(),
                1.0f});
        glUniformMatrix4fv(mat_loc, 1, GL_FALSE, glm::value_ptr(screen_to_gl_coords));

        /* Create the texture (temporary workaround until we can use the Renderable's texture) */
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glUniform1i(tex_loc, 0);

        /* Create VBO */
        glGenBuffers(1, &vertex_attribs_vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_attribs_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_attribs),
                glm::value_ptr(vertex_attribs[0].position), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
    }

    void draw()
    {
        auto active_session = system_compositor.get_active_session();
        if (!active_session)
            return;

        // ...
    }

    typedef void(*MirGLGetObjectInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);
    typedef void(*MirGLGetObjectiv)(GLuint, GLenum, GLint *);

    void GetObjectLogAndThrow(MirGLGetObjectInfoLog getObjectInfoLog,
                              MirGLGetObjectiv      getObjectiv,
                              std::string const &   msg,
                              GLuint                object)
    {
        GLint object_log_length = 0;
        (*getObjectiv)(object, GL_INFO_LOG_LENGTH, &object_log_length);

        const GLuint object_log_buffer_length = object_log_length + 1;
        std::string  object_info_log;

        object_info_log.resize(object_log_buffer_length);
        (*getObjectInfoLog)(object, object_log_length, NULL, const_cast<GLchar *>(object_info_log.data()));

        std::string object_info_err(msg + "\n");
        object_info_err += object_info_log;

        BOOST_THROW_EXCEPTION(std::runtime_error(object_info_err));
    }

    void cleanup()
    {
        if (vertex_shader)
            glDeleteShader(vertex_shader);
        if (fragment_shader)
            glDeleteShader(fragment_shader);
        if (program)
            glDeleteProgram(program);
        if (vertex_attribs_vbo)
            glDeleteBuffers(1, &vertex_attribs_vbo);
        if (texture)
            glDeleteTextures(1, &texture);
    }

    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;
    GLuint position_attr_loc;
    GLuint texcoord_attr_loc;
    GLuint transform_uniform_loc;
    GLuint alpha_uniform_loc;
    GLuint vertex_attribs_vbo;
    GLuint texture;
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
                return std::make_shared<SCCompositingStrategy>(system_compositor, the_display()->view_area().size);
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
