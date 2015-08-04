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

#ifndef CLONEDISPLAYCONFIGURATIONPOLICY_H
#define CLONEDISPLAYCONFIGURATIONPOLICY_H

#include <mir/graphics/display_configuration_policy.h>
#include <memory>

class CloneDisplayConfigurationPolicy : public mir::graphics::DisplayConfigurationPolicy
{
public:
    CloneDisplayConfigurationPolicy(const std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> &wrapped);

    void apply_to(mir::graphics::DisplayConfiguration& conf) override;

private:
    const std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> wrapped;
};

#endif // CLONEDISPLAYCONFIGURATIONPOLICY_H
