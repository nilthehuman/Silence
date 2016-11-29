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

    struct BoundingBox;
    class  Camera;
    class  Surface;
    class  Thing;
    class  Zone;

    class Beam {
    public:
        typedef double (*Distribution)( const Ray& pivot, const Vector& point );

        static double Uniform( const Ray& pivot, const Vector& point )
        {
            const double distance = pivot.getDirection() * point - pivot.getDirection() * pivot.getOrigin();
            return UNITDIST * UNITDIST / ( distance * distance );
        }
        static double Spherical( const Ray& pivot, const Vector& point )
        {
            const double distance = (point - pivot.getOrigin()).length();
            return UNITDIST * UNITDIST / ( distance * distance );
        }
        static double Planar( const Ray& pivot, const Vector& point )
        {
            const Vector toPoint  = point - pivot.getOrigin();
            const double distance = toPoint.length();
            const double cosine   = pivot.getDirection() * toPoint.normalized();
            return cosine * UNITDIST * UNITDIST / ( distance * distance );
        }

        Beam( const Scene* scene,
              const Vector& apex, const Surface* source, const Thing* medium, const Ray& pivot, const std::vector< Ray >& edges,
              const Triplet& color, Distribution distribution, Material::Interaction kind = Material::DIFFUSE )
            : scene( scene )
            , zone( NULL ) // To be set later
            , apex( apex )
            , source( source )
            , medium( medium )
            , pivot( pivot )
            , edges( edges )
            , color( color )
            , distribution( distribution )
            , kind( kind )
        {
            assert( scene );
        }

        ~Beam()
        { }

        const   Scene*                getScene()        const { return scene;  }
        const   Vector&               getApex()         const { return apex;   }
        const   Surface*              getSource()       const { return source; }
        const   Thing*                getMedium()       const { return medium; }
        const   Ray&                  getPivot()        const { return pivot;  }
        const   std::vector< Ray >&   getEdges()        const { return edges;  }
        const   Triplet&              getColor()        const { return color;  }
                Distribution          getDistribution() const { return distribution; }
                Material::Interaction getKind()         const { return kind;   }

        bool    contains    ( const Vector& point ) const;
        Triplet getColor    ( const Ray&   eyeray ) const;
        double  getIntensity( const Ray&   eyeray ) const;
        void    rasterizeRow( const Camera* camera, const BoundingBox& bb, int row, RGB* buffer, double* skyBlocked ) const;

        void    occlude(); // Figure out what ThingParts obstruct the Beam

    private:
        void    setZone( const Zone* z ) { zone = z; }

        friend class Zone;

        void    paint( const Triplet& otherColor ) { color *= otherColor; } // Incorporate the color of a Surface that was hit

        double  fresnelIntensity( const Ray& eyeray, const Vector& point = Vector::Invalid ) const;
        static double schlick( double n1, double n2, double cosTheta );

    private:
        const Scene* const scene;
        const Zone*        zone;

        Vector                apex;   // The point where all Rays meet
        const Surface*        source; // The Surface the Beam emanates from
        const Thing*          medium; // The Thing the Beam travels inside (if any)
        Ray                   pivot;  // A representative Ray
        std::vector< Ray >    edges;  // Rays to mark Beam boundaries
        Triplet               color;  // Current color of pivot Ray (may change with each bounce)
        Distribution          distribution; // Provides each point a relative light intensity
        Material::Interaction kind;   // The reason for the latest bounce
    };

}

#endif // SILENCE_BEAM

