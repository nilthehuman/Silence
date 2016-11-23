/*
 * Copyright 2016 Dániel Arató
 *
 * This file is part of Silence.
 *
 * Silence is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Silence is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Silence.  If not, see <http://www.gnu.org/licenses/>.
 */

// Ray class methods
// Part of Silence, an experimental rendering engine

#include "ray.h"

#include <cstdlib>

#include "aux.h"
#include "scene.h"

namespace Silence {

    double Ray::findNearestIntersection()
    {
        double nearestT = INF;
        double t        = INF;

        // Check foreground Surfaces
        for ( ThingIt thing = scene->thingsBegin(); thing != scene->thingsEnd(); thing++ )
            if ( (*thing)->isBackground() == false )
                for ( ThingPartIt part = (*thing)->partsBegin(); part != (*thing)->partsEnd(); part++ )
                    if ( (t = (*part)->intersect(*this)) && t < nearestT )
                        nearestT = t;

        if ( thingPartHit )
            return nearestT;

        // Check background Surfaces
        for ( ThingIt thing = scene->thingsBegin(); thing != scene->thingsEnd(); thing++ )
            if ( (*thing)->isBackground() == true )
                for ( ThingPartIt part = (*thing)->partsBegin(); part != (*thing)->partsEnd(); part++ )
                    if ( (t = (*part)->intersect(*this)) && t < nearestT )
                        nearestT = t;

        return nearestT;
    }

}

