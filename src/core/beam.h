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

#include "ray.h"
#include "triplet.h"

namespace Silence {

    class Surface;
    class Thing;

    class Beam {
    public:
        typedef double (*Distribution)( const Ray& pivot, const Vector& point );

        Beam( const Scene* scene, const Vector& apex, const Surface* source, const Ray& pivot, const std::vector< Ray* >& edges, const RGB& color, Distribution distribution )
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
        {
            std::vector< Ray* >::const_iterator edge;
            for ( edge = edges.begin(); edge != edges.end(); edge++ )
                delete *edge;
        }

        const RGB& getColor() const { return color; }

        bool    contains( const Vector& point ) const;
        RGB     getColor( const Vector& point ) const;

    private:
        void    paint( const Triplet& otherColor ) { color *= otherColor; } // Incorporate the color of a Surface that was hit

        void    bounceDiffuse();
        void    bounceMetallic();
        void    bounceReflect();
        void    bounceRefract();

        double  schlick( double n1, double n2, double cosTheta ) const;

    private:
        const Scene* const  scene;

        Vector              apex;   // The point where all Rays meet
        const Surface*      source; // The Surface the Beam emanates from
        Ray                 pivot;  // A representative Ray
        std::vector< Ray* > edges;  // Rays to mark Beam boundaries
        RGB                 color;  // Current color of pivot Ray (may change with each bounce)
        Distribution        distribution; // Provides each point a relative light intensity
    };

}

#endif // SILENCE_BEAM

