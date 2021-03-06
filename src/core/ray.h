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

    class Thing;
    class ThingPart;
    class Scene;

    class Ray {
    public:
        Ray( const Scene* scene, const Vector& origin, const Vector& direction, const Thing* medium = NULL )
            : scene( scene )
            , origin( origin )
            , direction( Vector::Zero == direction ? direction : direction.normalized() )
            , medium( medium )
        {
            assert( scene || (Vector::Invalid == origin && Vector::Invalid == direction) );
        }

        static const Ray Invalid;

        friend std::ostream& operator<<( std::ostream& os, const Ray& ray );

        const Vector& getOrigin()     const { return origin; }
        const Vector& getDirection()  const { return direction; }

        Vector operator[]( double t ) const { return origin + direction * t; }

        Ray    bounceDiffuse ( const ThingPart* part, const Vector& point = Vector::Invalid ) const;
        Ray    bounceMetallic( const ThingPart* part, const Vector& point = Vector::Invalid ) const;
        Ray    bounceReflect ( const ThingPart* part, const Vector& point = Vector::Invalid ) const;
        Ray    bounceRefract ( const ThingPart* part, const Vector& point = Vector::Invalid ) const;

        double findNearestIntersection();

    private:
        const Scene* const scene;

        const Vector origin;
        const Vector direction;
        const Thing* medium; // The Thing the Ray is born inside
    };

}

#endif // SILENCE_RAY

