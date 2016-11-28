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

// A few auxiliary constants and functions for handling doubles, plus global flags
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_AUX
#define SILENCE_AUX

namespace Silence {

    extern struct flags {
        bool verbose;
    } modeFlags;

    const double EPSILON    = 10e-10; // Ballpark values, tune later
    const double INF        = 10e+10;

    const double UNITDIST   = 10; // Points at this distance from a Light receive 1.0 times its intensity

    const double PI         = 3.141592654;
    const double TANPIOVER6 = 0.57735;

    inline double  abs( double x )           { return x < 0 ? -x : x; }
    inline double sign( double x )           { return x < 0 ? -1 : x == 0 ? 0 : 1; }
    inline double  min( double a, double b ) { return a < b ?  a : b; }
    inline double  max( double a, double b ) { return a > b ?  a : b; }

    inline bool  equal( double a, double b ) { return abs(a - b) < EPSILON; }

}

#endif // SILENCE_AUX

