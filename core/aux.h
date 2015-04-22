/*
 * Copyright 2015 Dániel Arató
 *
 * This file is part of Retra.
 *
 * Retra is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Retra is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Retra.  If not, see <http://www.gnu.org/licenses/>.
 */

// A few auxiliary constants and functions for handling doubles, plus global flags
// Part of Retra, the Reference Tracer

#ifndef RETRA_AUX
#define RETRA_AUX

namespace Retra {

    extern struct flags {
        bool verbose;
    } modeFlags;

    const double EPSILON    = 10e-10; // Ballpark values, tune later
    const double INF        = 10e+10;

    const double UNITDIST   = 10; // Points at this distance from a Light receive 1.0 times its intensity

    const int    SHADOWRAYS = 1; // The number of shadow rays only matters if you're using a small amount of samples.
                                 // 8 <= SHADOWRAYS should help eliminate any crude variance in the penumbrae

    inline double abs( double x )           { return x < 0 ? -x : x; }
    inline double min( double a, double b ) { return a < b ?  a : b; }
    inline double max( double a, double b ) { return a > b ?  a : b; }

    inline bool equal( double a, double b ) { return abs(a - b) < EPSILON; }

}

#endif // RETRA_AUX

