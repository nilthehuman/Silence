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

// Beam class for a homogeneous bundle of light
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_BEAM
#define SILENCE_BEAM

#include <cassert>
#include <stack>
#include <vector>

#include "material.h"
#include "ray.h"
#include "triplet.h"

namespace Silence {

    class Camera;
    class Surface;
    class Thing;

    class Beam {
    public:
        typedef double (*Distribution)( const Ray& pivot, const Vector& point );

        static double Uniform( const Ray&, const Vector& )
        {
            return 1;
        }
        static double Spherical( const Ray& pivot, const Vector& point )
        {
            const double distance = (point - pivot.getOrigin()).length();
            return UNITDIST * UNITDIST / ( distance * distance );
        }

        Beam( const Scene* scene, const Vector& apex, const Surface* source, const Ray& pivot, const std::vector< Ray >& edges, const Triplet& color, Distribution distribution )
            : scene( scene )
            , apex( apex )
            , source( source )
            , pivot( pivot )
            , edges( edges )
            , color( color )
            , distribution( distribution )
        {
            assert( scene );
        }

        ~Beam()
        { }

        const   Scene*   getScene() const { return scene; }
        const   Vector&  getApex()  const { return apex;  }
        const   Triplet& getColor() const { return color; }

        bool    contains( const Vector& point ) const;
        Triplet getColor( const Vector& point ) const;
        void    rasterizeRow( const Camera* camera, int row, RGB* buffer ) const;

        void    occlude(); // Figure out what ThingParts obstruct the Beam
        Beam    bounce( const ThingPart* part, const Material::Interaction& interaction ) const; // Spawn next Beam after hitting a ThingPart

    private:
        void    paint( const Triplet& otherColor ) { color *= otherColor; } // Incorporate the color of a Surface that was hit

        inline Beam bounceDiffuse ( const ThingPart* part ) const;
        inline Beam bounceMetallic( const ThingPart* part ) const;
        inline Beam bounceReflect ( const ThingPart* part ) const;
        inline Beam bounceRefract ( const ThingPart* part ) const;

        static double schlick( double n1, double n2, double cosTheta );

    private:
        const Scene* const scene;

        Vector             apex;   // The point where all Rays meet
        const Surface*     source; // The Surface the Beam emanates from
        Ray                pivot;  // A representative Ray
        std::vector< Ray > edges;  // Rays to mark Beam boundaries
        Triplet            color;  // Current color of pivot Ray (may change with each bounce)
        Distribution       distribution; // Provides each point a relative light intensity
    };

}

#endif // SILENCE_BEAM

