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

#ifndef VOLUMEKEY_HANDLER_H_
#define VOLUMEKEY_HANDLER_H_

#include "mir/input/event_filter.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>

namespace mir
{
namespace time
{
class Alarm;
class Timer;
}
}

class VolumeKeyHandler : public mir::input::EventFilter
{
public:
    VolumeKeyHandler();
    ~VolumeKeyHandler();

    bool handle(MirEvent const& event) override;

private:
    void volume_decrease_key_down();
    void volume_decrease_key_up();
    void volume_increase_key_down();
    void volume_increase_key_up();

    std::mutex guard;
};

#endif /* VOLUMEKEY_HANDLER_H_ */
