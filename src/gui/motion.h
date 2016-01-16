/*
 * Copyright 2015 Dániel Arató
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

// Motion classes to describe how a given Object is supposed to move around the Scene
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_MOTION
#define SILENCE_MOTION

#include "../core/scene.h"
#include "../core/triplet.h"

namespace Silence {

    class Object;

    class Motion {
    public:
        Motion( const Object* object ) : object( object ) { }
        virtual ~Motion() { }

        virtual void step( double dt ) const = 0; // Apply the displacement between the current and the next "frame" to the Object

    protected:
        const Object* const object;
    };

    // Random motion each time
    class BrownianMotion : public Motion {
    public:
        BrownianMotion( const Object* object, double scale = 1.0 )
            : Motion( object )
            , scale( scale )
        { }

        virtual void step( double dt ) const;

    private:
        double scale;
    };

    // Move in a constant direction at constant speed
    class LinearMotion : public Motion {
    public:
        LinearMotion( const Object* object, const Vector& delta, double stop = -1 )
            : Motion( object )
            , delta( delta )
            , stop( stop )
            , distance( 0 )
        { }

        virtual void step( double dt ) const;

    private:
        Vector delta;
        double stop; // Maximal distance traveled
        mutable double distance;
    };

    // Rotate around fixed world axis
    class OrbitingMotion : public Motion {
    public:
        OrbitingMotion( const Object* object, WorldAxis axis, double period )
            : Motion( object )
            , axis( axis )
            , period( period )
        { }

        virtual void step( double dt ) const;

    private:
        WorldAxis axis;
        double    period;
    };

    // Cosine wave between two points
    class OscillatingMotion : public Motion {
    public:
        OscillatingMotion( const Object* object, const Vector& begin, const Vector& end, double period )
            : Motion( object )
            , begin( begin )
            , end( end )
            , period( period )
            , phase( 0 )
        { }

        virtual void step( double dt ) const;

    private:
        Vector begin;
        Vector end;
        double period;
        mutable double phase; // Current state
    };

}

#endif // SILENCE_MOTION

