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

    const Ray Ray::Invalid = Ray( NULL, Vector::Invalid, Vector::Invalid );

    std::ostream& operator<<( std::ostream& os, const Ray& ray )
    {
        os << ray.origin << " -> " << ray.direction;
        return os;
    }

    Ray Ray::bounceDiffuse( const ThingPart* part, const Vector& point ) const
    {
        assert( part );
        Vector hitPoint;
        if ( Vector::Invalid != point )
            hitPoint = point;
        else
        {
            const double t = part->intersect( *this );
            if ( equal(0, t) )
                return Ray::Invalid;
            hitPoint = (*this)[t];
        }
        const Vector surfaceNormal = part->getNormal( hitPoint );
        return Ray( scene, hitPoint, surfaceNormal );
    }

    Ray Ray::bounceMetallic( const ThingPart* part, const Vector& point ) const
    {
        assert( part );
        Vector hitPoint;
        if ( Vector::Invalid != point )
            hitPoint = point;
        else
        {
            const double t = part->intersect( *this );
            if ( equal(0, t) )
                return Ray::Invalid;
            hitPoint = (*this)[t];
        }
        const Vector surfaceNormal = part->getNormal( hitPoint );
        const Vector newDirection  = direction - surfaceNormal * (direction * surfaceNormal) * 2;
        return Ray( scene, hitPoint, newDirection );
    }

    Ray Ray::bounceReflect( const ThingPart* part, const Vector& point ) const
    {
        assert( part );
        Vector hitPoint;
        if ( Vector::Invalid != point )
            hitPoint = point;
        else
        {
            const double t = part->intersect( *this );
            if ( equal(0, t) )
                return Ray::Invalid;
            hitPoint = (*this)[t];
        }
        const Vector surfaceNormal = part->getNormal( hitPoint );
        const Vector newDirection  = direction - surfaceNormal * (direction * surfaceNormal) * 2;
        return Ray( scene, hitPoint, newDirection );
    }

    Ray Ray::bounceRefract( const ThingPart* part, const Vector& point ) const
    {
        assert( part );
        Vector hitPoint;
        if ( Vector::Invalid != point )
            hitPoint = point;
        else
        {
            // Take it upon ourselves to find the contact point
            const double t = part->intersect( *this );
            if ( equal(0, t) )
                return Ray::Invalid;
            hitPoint = (*this)[t];
        }
        // en.wikipedia.org/wiki/Snell's_law
        // http://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
        double n1, n2;
        if ( medium )
        {
            n1 = medium->getRefractiveIndex();
            n2 = 1.0; // Vacuum
        }
        else
        {
            n1 = 1.0; // Vacuum
            n2 = static_cast<const Thing*>(part->getParent())->getRefractiveIndex();
        }
        const double eta = n1 / n2;
        const Vector surfaceNormal = part->getNormal( origin );
        const double cosTheta1 = abs( direction * surfaceNormal );
        const double sinTheta2Squared = eta * eta * ( 1.0 - cosTheta1 * cosTheta1 ); // sin(x)^2 + cos(x)^2 == 1
        if ( 1 < sinTheta2Squared )
        {
            // Total internal reflection
            const Vector newDirection = direction + surfaceNormal * (direction * surfaceNormal) * 2;
            return Ray( scene, hitPoint, newDirection );
        }
        // Actual refractive transmission
        const double cosTheta2 = sqrt( 1.0 - sinTheta2Squared );
        const Vector newDirection = direction * eta + surfaceNormal * ( eta * cosTheta1 - cosTheta2 ) * ( direction * surfaceNormal < 0 ? 1.0 : -1.0 );
        return Ray( scene, hitPoint, newDirection );
    }

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

        if ( !equal(nearestT, INF) )
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

