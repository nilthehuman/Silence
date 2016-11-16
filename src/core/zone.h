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

// Zone class: the basic unit of the rendering algorithm
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_ZONE
#define SILENCE_ZONE

#include "beam.h"
#include "shadow.h"

namespace Silence {

    class Camera;
    class Plane;

    class Zone {
    public:
        Zone( Beam* light )
            : light( light )
            , shadows()
            , insideThings()
        { }
        Zone( Beam* light, const std::vector< Shadow* >& shadows )
            : light( light )
            , shadows( shadows )
            , insideThings()
        { }

        ~Zone()
        {
            std::vector< Shadow* >::const_iterator shadow;
            for ( shadow = shadows.begin(); shadow != shadows.end(); shadow++ )
                delete *shadow;
        }

        void rasterize( Camera* camera ) const;

        inline bool russianRoulette( double rrLimit ) const;

    private:
        void bounce();

        void rasterizeRow( const Plane* cameraPlane, const Vector& start, const Vector& end, int width, RGB* row ) const;

    private:
        Beam* light; // Only a single light Beam per Zone is allowed
        std::vector< Shadow* > shadows;

        std::stack< const Thing* > insideThings; // LIFO stack of penetrated Things (passed down from parent Zone)
    };

}

#endif // SILENCE_ZONE

