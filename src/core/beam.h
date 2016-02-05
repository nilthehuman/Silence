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
        Beam( const Scene* scene, const Vector& apex, const Surface* source, const Ray& pivot, const std::vector< Ray* >& edges, const RGB& color, int depth, double rrLimit )
            : scene( scene )
            , apex( apex )
            , source( source )
            , pivot( pivot )
            , edges( edges )
            , color( color )
            , depth( depth )
            , rrLimit( rrLimit )
            , insideThings()
        {
            assert( scene );
        }

        // TODO: which of these are really necessary?
        //const Vector& getApex()  const { return apex; }
        //const Vector& getPivot() const { return pivot; }
        //const RGB&    getColor() const { return color; }
        //int           getDepth() const { return depth; }

        RGB     trace(); // Trace the Beam until terminated

    private:
        void    paint( const Triplet& otherColor ) { color *= otherColor; } // Incorporate the color of a Surface that was hit

        inline bool russianRoulette() const;
        RGB     bounceDiffuse();
        RGB     bounceMetallic();
        RGB     bounceReflect();
        RGB     bounceRefract();

        double  schlick( double n1, double n2, double cosTheta ) const;

    private:
        const Scene* const  scene;

        Vector              apex;   // The point where all Rays meet
        const Surface*      source; // The Surface the Beam emanates from
        Ray                 pivot;  // A representative Ray
        std::vector< Ray* > edges;  // Rays to mark Beam boundaries
        RGB                 color;  // Current color of pivot Ray (may change with each bounce)

        int                 depth;  // Number of bounces to live (at most)
        const double        rrLimit;// Limit to keep weak Beams alive in Russian Roulette

        std::stack< const Thing* > insideThings; // LIFO stack of penetrated Things (passed down from parent Beam)
    };

}

#endif // SILENCE_BEAM

