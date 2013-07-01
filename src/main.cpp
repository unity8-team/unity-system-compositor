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
#include <mir/report_exception.h>
#include <iostream>
#include <sys/stat.h>

/* Check the existence of a file which
 * will force X to be used instead of Mir.
 */
static bool should_use_x(void)
{
    std::string name = "/usr/share/mir/force-off";
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

int main(int argc, char const* argv[])
try
{
    if (should_use_x())
    {
        std::cerr << "Manually forcing the use of X"
                  << std::endl;
        return 1;
    }
    SystemCompositor system_compositor;
    system_compositor.run(argc, argv);

    return 0;
}
catch (...)
{
    mir::report_exception(std::cerr);
    return 1;
}
