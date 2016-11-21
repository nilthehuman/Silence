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

// Common RGB values
// Part of Silence, an experimental rendering engine

#include "triplet.h"

#include <cstdlib>
#include <limits>

namespace Silence {

    std::ostream& operator<<( std::ostream& os, const Triplet& triplet )
    {
        os << "( " << triplet.x << ", " << triplet.y << ", " << triplet.z << ")";
        return os;
    }

    Triplet& Triplet::normalize()
    {
        return (*this).cap( RGB::White ).raise( RGB::Black );
    }

    Triplet Triplet::normalized() const
    {
        Triplet newTriplet( *this );
        return newTriplet.normalize();
    }

    // Returns a random unit vector whose dot product with 'normal' is non-negative
    Vector Vector::random( const Vector& normal )
    {
        double x;
        double y;
        double z;
        do {
            x = (double)std::rand() / RAND_MAX * 2 - 1;
            y = (double)std::rand() / RAND_MAX * 2 - 1;
            z = (double)std::rand() / RAND_MAX;
        } while ( 1 < x*x + y*y + z*z || (!x && !y && !z) );

        Vector tangentialX, tangentialY;
        if ( Vector::UnitZ == normal || -Vector::UnitZ == normal )
        {
            tangentialX = Vector::UnitX;
            tangentialY = Vector::UnitY;
        }
        else
        {
            tangentialX = normal.cross( Vector::UnitZ ).normalize();
            tangentialY = normal.cross( tangentialX   ).normalize();
        }
        Vector result = tangentialX * x + tangentialY * y + normal * z;
        return result.normalize();
    }

    const RGB RGB::Black = RGB(0, 0, 0); // == RGB()
    const RGB RGB::Red   = RGB(1, 0, 0);
    const RGB RGB::Green = RGB(0, 1, 0);
    const RGB RGB::Blue  = RGB(0, 0, 1);
    const RGB RGB::White = RGB(1, 1, 1);

    const Vector Vector::Zero    = Vector(0, 0, 0); // == Vector()
    const Vector Vector::UnitX   = Vector(1, 0, 0);
    const Vector Vector::UnitY   = Vector(0, 1, 0);
    const Vector Vector::UnitZ   = Vector(0, 0, 1);
    const Vector Vector::Invalid = Vector(-INF, -INF, -INF);

}

