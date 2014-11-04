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

#ifndef INACTIVITY_TRACKER_H_
#define INACTIVITY_TRACKER_H_

class  InactivtyTracker
{
public:
    InactivtyTracker() = default;
    virtual ~InactivtyTracker() = default;
    virtual void enable_inactivity_timers(bool enable) = 0;
protected:
    InactivtyTracker(InactivtyTracker const&) = delete;
    InactivtyTracker& operator=(InactivtyTracker const&) = delete;
};

#endif

