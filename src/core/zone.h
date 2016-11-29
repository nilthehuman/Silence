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
#include "tree.h"

namespace Silence {

    struct BoundingBox;
    class  Plane;
    class  ThingPart;

    class Zone {
    public:
        Zone( const Beam& light )
            : node( NULL ) // To be set later
            , light( light )
            , shadows()
        {
            this->light.setZone( this );
        }
        Zone( const Beam& light, const std::vector< Shadow >& shadows )
            : node( NULL ) // To be set later
            , light( light )
            , shadows( shadows )
        {
            this->light.setZone( this );
        }

        ~Zone()
        { }

        const Tree<Zone>*    getNode()   const { return node; }
        const Beam&          getLight()  const { return light; }

        void                 occlude(); // Generate Shadow beams
        std::vector< Zone* > bounce();  // Generate child Zones

        void rasterize( Camera* camera ) const;

    private:
        void setNode( const Tree<Zone>* n ) { node = n; }

        friend class Renderer;

        bool hit     ( const ThingPart* part ) const; // Is a surface element reached by the light?
        bool eclipsed( const ThingPart* part ) const; // Is a surface element completely obscured?

        void rasterizeRow( const Camera* camera, const BoundingBox& bb, int row, RGB* pixelBuffer, double* skyBlocked ) const;

    private:
        const Tree<Zone>* node; // An unfortunate necessity

        Beam light; // Only a single light Beam per Zone is allowed
        std::vector< Shadow > shadows;
    };

}

#endif // SILENCE_ZONE

