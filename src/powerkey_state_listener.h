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
 */

#ifndef POWER_KEY_STATE_LISTENER_
#define POWER_KEY_STATE_LISTENER_

class PowerKeyStateListener
{
public:
    virtual void power_key_down() = 0;
    virtual void power_key_short() = 0;
    virtual void power_key_long() = 0;
    virtual void power_key_very_long() = 0;
    virtual void power_key_up() = 0;
};

#endif
