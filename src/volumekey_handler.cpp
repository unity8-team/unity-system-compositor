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

#include "volumekey_handler.h"
#include <QDebug>
namespace mi = mir::input;

VolumeKeyHandler::VolumeKeyHandler()
{
}

VolumeKeyHandler::~VolumeKeyHandler() = default;

bool VolumeKeyHandler::handle(MirEvent const& event)
{
    qDebug() << "JOSH: they key code is: " << event.key.key_code; 
    return true;
}

void VolumeKeyHandler::volume_decrease_key_down()
{
}

void VolumeKeyHandler::volume_decrease_key_up()
{
}

void VolumeKeyHandler::volume_increase_key_down()
{
}

void VolumeKeyHandler::volume_increase_key_up()
{
}

