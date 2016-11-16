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

// Beam class methods
// Part of Silence, an experimental rendering engine

#include "beam.h"

#include "scene.h"

namespace Silence {

    bool Beam::contains( const Vector& point ) const
    {
        if ( edges.size() < 3 )
            return false;
        const Vector direction = point - apex;
        const Ray ray( scene, apex, direction );
        const Vector testPoint = ray[ source->intersect(ray) ];
        if ( direction.length() < (testPoint - apex).length() )
            return false;
        // en.wikipedia.org/wiki/Point_in_polygon#Ray_casting_algorithm
        // TODO: make this work for spherical surfaces too
        const Vector rayCast = (edges[1]->getOrigin() + edges[0]->getOrigin()) * 0.5 - testPoint;
        bool inside = false;
        std::vector< Ray* >::const_iterator edgeA, edgeB;
        for ( edgeA = edges.begin(), edgeB = edges.begin() + 1; edgeA != edges.end(); ++edgeA, ++edgeB == edges.end() ? edgeB = edges.begin() : edgeB )
        {
            const Vector a = (*edgeA)->getOrigin() - testPoint;
            const Vector b = (*edgeB)->getOrigin() - testPoint;
            const double dotA = rayCast * a.normalized();
            const double dotB = rayCast * b.normalized();
            if ( dotA + dotB < 0 )
                continue; // the surface edge runs behind the test point
            if ( rayCast.cross(a) * rayCast.cross(b) < 0 )
                inside = !inside;
        }
        return inside;
    }

    Triplet Beam::getColor( const Vector& point ) const
    {
        if ( contains(point) )
            return color * (*distribution)( pivot, point );
        else
            return RGB::Black;
    }

    double Beam::schlick( double n1, double n2, double cosTheta ) const
    {
        // http://en.wikipedia.org/wiki/Schlick%27s_approximation
        const double R0 = (n1 - n2) * (n1 - n2) / ( (n1 + n2) * (n1 + n2) );
        return R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5);
    }

}

