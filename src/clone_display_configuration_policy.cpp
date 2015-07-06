/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clone_display_configuration_policy.h"

#include <mir/graphics/display_configuration.h>
#include <cstdio>
namespace mg = mir::graphics;

CloneDisplayConfigurationPolicy::CloneDisplayConfigurationPolicy(
        const std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> &wrapped)
   : wrapped{wrapped}
{
}

void CloneDisplayConfigurationPolicy::apply_to(mir::graphics::DisplayConfiguration& conf)
{
    wrapped->apply_to(conf);

    conf.for_each_output(
                [&](mg::UserDisplayConfigurationOutput& displayConfigOutput) {
        if (displayConfigOutput.id.as_value() > 0) { printf("Here\n");
            displayConfigOutput.orientation = mir_orientation_right;
        }
    }
    );

    conf.for_each_output(
                [&](const mg::DisplayConfigurationOutput displayConfigOutput) {
        printf("Output %d: Orientation %d\n", displayConfigOutput.id.as_value(), displayConfigOutput.orientation);
    }
    );
}
