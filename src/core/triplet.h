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

// Triplet struct for color and 3D vectors, plus a bit of auxiliary math
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_TRIPLET
#define SILENCE_TRIPLET

#include <math.h>
#include <iostream>
#include <cassert>

#include "aux.h"

namespace Silence {

    struct Triplet {
        explicit Triplet( double x = 0, double y = 0, double z = 0 ) : x(x), y(y), z(z) { }
        Triplet( const Triplet& other ) : x(other.x), y(other.y), z(other.z) { }

        bool     operator==( const Triplet& other ) const { return  equal( x, other.x ) &&  equal( y, other.y ) &&  equal( z, other.z ); }
        bool     operator!=( const Triplet& other ) const { return !equal( x, other.x ) || !equal( y, other.y ) || !equal( z, other.z ); }

        Triplet  operator+ ( const Triplet& other ) const { return Triplet( x+other.x, y+other.y, z+other.z); }
        Triplet& operator+=( const Triplet& other )       { x += other.x; y += other.y; z += other.z; return *this; }
        Triplet  operator- ( const Triplet& other ) const { return Triplet( x-other.x, y-other.y, z-other.z); }
        Triplet& operator-=( const Triplet& other )       { x -= other.x; y -= other.y; z -= other.z; return *this; }

        Triplet  operator* ( double r ) const { return Triplet( x*r, y*r, z*r ); }
        Triplet& operator*=( double r )       { x *= r; y *= r; z *= r; return *this; }
        Triplet  operator/ ( double r ) const { return Triplet( x/r, y/r, z/r ); }
        Triplet& operator/=( double r )       { x /= r; y /= r; z /= r; return *this; }

        friend std::istream& operator>>( std::istream& is, Triplet& triplet );
        friend std::ostream& operator<<( std::ostream& os, const Triplet& triplet );

        double x, y, z;
    };

    struct RGB : public Triplet {
        explicit RGB( double x = 0, double y = 0, double z = 0 ) : Triplet(x, y, z)
        {
            assert( 0 <= x && x <= 1 );
            assert( 0 <= y && y <= 1 );
            assert( 0 <= z && z <= 1 );
        }
        explicit RGB( const Triplet& triplet ) : Triplet(triplet)
        {
            assert( 0 <= x && x <= 1 );
            assert( 0 <= y && y <= 1 );
            assert( 0 <= z && z <= 1 );
        }

        RGB& operator= ( const Triplet& triplet ) { x = triplet.x; y = triplet.y; z = triplet.z; return *this; }

        RGB  operator+ ( const RGB& other ) const { return RGB( min(x+other.x, 1), min(y+other.y, 1), min(z+other.z, 1) ); } // Can't use cap here
        RGB& operator+=( const RGB& other )       { x += other.x; y += other.y; z += other.z; cap( RGB::White ); return *this; }

        RGB  operator* ( double r ) const { return RGB( max(min(x*r, 1), 0), max(min(y*r, 1), 0), max(min(z*r, 1), 0) ); } // Can't use cap here
        RGB& operator*=( double r )       { x *= r; y *= r; z *= r; cap( RGB::White ); return *this; }
        RGB  operator/ ( double r ) const { return RGB( max(min(x/r, 1), 0), max(min(y/r, 1), 0), max(min(z/r, 1), 0) ); } // Can't use cap here
        RGB& operator/=( double r )       { x /= r; y /= r; z /= r; cap( RGB::White ); return *this; }

        RGB  operator* ( const Triplet& triplet ) const { return RGB( max(min(x*triplet.x, 1), 0), max(min(y*triplet.y, 1), 0), max(min(z*triplet.z, 1), 0) ); } // Can't use cap here
        RGB& operator*=( const Triplet& triplet )       { x *= triplet.x; y *= triplet.y; z *= triplet.z; cap( RGB::White ); raise( RGB::Black ); return *this; }

        RGB  operator* ( const RGB& other ) const { return RGB( x * other.x, y * other.y, z * other.z ); }
        RGB& operator*=( const RGB& other )       { x *= other.x; y *= other.y; z *= other.z; return *this; }

        RGB& cap       ( const RGB& other )       { x = min(x, other.x); y = min(y, other.y); z = min(z, other.z); return *this; }
        RGB& raise     ( const RGB& other )       { x = max(x, other.x); y = max(y, other.y); z = max(z, other.z); return *this; }
        // Values outside (0,0,0) through (1,1,1) are not valid RGB values. Return Triplet instead
        Triplet project( double min, double max ) const { return Triplet( min, min, min ) + ((Triplet)*this) * (max - min); }
        RGB& gamma     ( double gamma ) { x = pow( x, 1/gamma ); y = pow( y, 1/gamma ); z = pow( z, 1/gamma ); return *this; }

        static const RGB Black;
        static const RGB White;
        static const RGB Red;
        static const RGB Green;
        static const RGB Blue;
    };

    struct Vector : public Triplet {
        explicit Vector( double x = 0, double y = 0, double z = 0 ) : Triplet(x, y, z) { }

        // Redefine operators so that they return Vectors rather than Triplets
        Vector  operator+ ( const Vector& other ) const { return Vector( x+other.x, y+other.y, z+other.z); }
        Vector& operator+=( const Vector& other )       { x += other.x; y += other.y; z += other.z; return *this; }
        Vector  operator- ( const Vector& other ) const { return Vector( x-other.x, y-other.y, z-other.z); }
        Vector& operator-=( const Vector& other )       { x -= other.x; y -= other.y; z -= other.z; return *this; }

        Vector  operator* ( double r ) const { return Vector( x*r, y*r, z*r ); }
        Vector& operator*=( double r )       { x *= r; y *= r; z *= r; return *this; }
        Vector  operator/ ( double r ) const { return Vector( x/r, y/r, z/r ); }
        Vector& operator/=( double r )       { x /= r; y /= r; z /= r; return *this; }

        // Additional member functions
        Vector  operator- () const { return Vector( -x, -y, -z ); }
        double  length()     const { return sqrt( x*x + y*y + z*z ); }
        Vector  normalized() const { return *this / length(); }
        Vector& normalize()        { return ( *this /= length() ); }
        double  operator*( const Vector& other ) const { return x*other.x + y*other.y + z*other.z; } // Dot product
        Vector  cross    ( const Vector& other ) const { return Vector( y*other.z - other.y*z,
                                                                        z*other.x - other.z*x,
                                                                        x*other.y - other.x*y ); } // Vector product

        static Vector random( const Vector& normal );

        static const Vector Zero;
        static const Vector UnitX;
        static const Vector UnitY;
        static const Vector UnitZ;
    };

}

#endif // SILENCE_TRIPLET

