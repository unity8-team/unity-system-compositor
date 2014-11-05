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

#ifndef USC_CALL_STATE_LISTENER_H_
#define USC_CALL_STATE_LISTENER_H_

namespace usc
{

class CallStateListener
{
public:
    CallStateListener() = default;
    virtual ~CallStateListener() = default;
    virtual void call_active() = 0;
    virtual void no_call_active() = 0;
protected:
    CallStateListener(CallStateListener const&) = delete;
    CallStateListener& operator=(CallStateListener const&) = delete;
};
}

#endif
