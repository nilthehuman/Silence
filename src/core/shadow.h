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

// Shadows are used to track the unlit subvolumes of a Zone
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_SHADOW
#define SILENCE_SHADOW

#include "beam.h"

namespace Silence {

    class Shadow {
    public:
        Shadow( const Beam& umbra, const Beam& penumbra )
            : umbra( umbra )
            , penumbra( penumbra )
        {
            assert( umbra.getSource() == penumbra.getSource() );
        }

        const Surface* getSource() const { return umbra.getSource(); }

        double occluded( const Vector& point ) const;

    private:
        Beam umbra;    // Part completely occluded from the lightsource
        Beam penumbra; // Part partially  occluded from the lightsource
    };

}

#endif // SILENCE_SHADOW

