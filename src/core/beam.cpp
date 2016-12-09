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

#include "camera.h"
#include "scene.h"

namespace Silence {

    bool Beam::contains( const Vector& point ) const
    {
        const Vector direction = point - apex;
        if ( pivot.getDirection() * direction < 0 )
            return false;
        const Ray ray( scene, apex, direction );
        const Vector testPoint = ray[ source->intersect(ray) ];
        if ( direction.length() + EPSILON < (testPoint - apex).length() )
            return false;
        if ( edges.size() < 3 )
            return true;
        // en.wikipedia.org/wiki/Point_in_polygon#Ray_casting_algorithm
        // TODO: make this work for spherical surfaces too
        const Vector rayCast = (edges[1].getOrigin() + edges[0].getOrigin()) * 0.5 - testPoint;
        bool inside = false;
        std::vector< Ray >::const_iterator edgeA, edgeB;
        for ( edgeA = edges.begin(), edgeB = edges.begin() + 1;
              edgeA != edges.end();
              ++edgeA, ++edgeB == edges.end() ? edgeB = edges.begin() : edgeB )
        {
            const Vector a = (*edgeA).getOrigin() - testPoint;
            const Vector b = (*edgeB).getOrigin() - testPoint;
            const double dotA = rayCast * a.normalized();
            const double dotB = rayCast * b.normalized();
            if ( dotA + dotB < 0 )
                continue; // the surface edge runs behind the test point
            if ( rayCast.cross(a) * rayCast.cross(b) < 0 )
                inside = !inside;
        }
        return inside;
    }

    bool Beam::containsNew( const Vector& point ) const
    {
        const Vector direction = point - apex;
        if ( pivot.getDirection() * direction < 0 )
            return false;
        if ( edges.size() < 3 )
            return true;
        // en.wikipedia.org/wiki/Point_in_polygon#Ray_casting_algorithm
        const Vector normal = -direction.normalized();
        const double offset = normal * point;
        Plane plane( normal, offset );
        std::vector< Vector > images;
        for ( std::vector< Ray >::const_iterator edge = edges.begin(); edge != edges.end(); edge++ )
        {
            const double t = plane.intersect( *edge );
            if ( equal(0, t) )
                return false;
            images.push_back( (*edge)[t] );
        }
        const Vector rayCast = (images[1] + images[0]) * 0.5 - point;
        bool inside = false;
        std::vector< Vector >::const_iterator imageA, imageB;
        for ( imageA = images.begin(), imageB = images.begin() + 1;
              imageA != images.end();
              ++imageA, ++imageB == images.end() ? imageB = images.begin() : imageB )
        {
            const Vector a = *imageA - point;
            const Vector b = *imageB - point;
            const double dotA = rayCast * a.normalized();
            const double dotB = rayCast * b.normalized();
            if ( dotA + dotB < 0 )
                continue; // the surface edge runs behind the test point
            if ( rayCast.cross(a) * rayCast.cross(b) < 0 )
                inside = !inside;
        }
        return inside;
    }

    double Beam::fresnelIntensity( const Ray& eyeray, const Vector& point ) const
    {
        double n1, n2;
        if ( medium )
        {
            n1 = medium->getRefractiveIndex();
            n2 = 1.0;
        }
        else
        {
            n1 = 1.0;
            n2 = static_cast<const Thing*>(source->getParent())->getRefractiveIndex();
        }
        Vector hitPoint;
        if ( Vector::Invalid != point )
            hitPoint = point;
        else
        {
            const double t = source->intersect( eyeray );
            if ( t < EPSILON )
                return 0;
            hitPoint = eyeray[t];
        }
        const Vector surfaceNormal = source->getNormal( hitPoint );
        const Vector direction     = eyeray.getDirection() - surfaceNormal * (eyeray.getDirection() * surfaceNormal) * 2;
        const double cosTheta = direction * surfaceNormal;
        return schlick( n1, n2, cosTheta );
    }

    double Beam::schlick( double n1, double n2, double cosTheta )
    {
        // http://en.wikipedia.org/wiki/Schlick%27s_approximation
        const double R0 = (n1 - n2) * (n1 - n2) / ( (n1 + n2) * (n1 + n2) );
        return R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5);
    }

}

