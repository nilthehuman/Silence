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

// Ray class for a single ray of light
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_RAY
#define SILENCE_RAY

#include <cassert>

#include "triplet.h"

namespace Silence {

    class ThingPart;
    class Scene;

    class Ray {
    public:
        Ray( const Scene* scene, const Vector& origin, const Vector& direction )
            : scene( scene )
            , origin( origin )
            , direction( direction )
            , thingPartHit( NULL )
        {
            assert( scene );
            this->direction.normalize();
        }

        const Vector& getOrigin()       const { return origin; }
        const Vector& getDirection()    const { return direction; }
        const Vector  getIntersection() const { return 0 < length ? (*this)[length] : Vector::Invalid; }

        Vector operator[]( double t )   const { return origin + direction * t; }

        double findNearestIntersection();

    private:
        const Scene* const scene;

        Vector  origin;
        Vector  direction;
        double  length;

        const ThingPart* thingPartHit;
    };

}

#endif // SILENCE_RAY

