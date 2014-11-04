/*
 * Copyright © 2014 Canonical Ltd.
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
 * Authored by: Andreas Pokorny <andreas.pokorny@canonical.com>
 */

#ifndef UNITY_SYSTEM_COMPOSITOR_SYSTEM_H_
#define UNITY_SYSTEM_COMPOSITOR_SYSTEM_H_

/*!
 * Interfaces to low level c-library functionality.
 */
class System
{
public:
    System() = default;
    virtual ~System() = default;
    virtual void shutdown() = 0;
protected:
    System(System const&) = delete;
    System& operator=(System const&) = delete;
};

#endif

