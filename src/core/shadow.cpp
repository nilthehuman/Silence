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

// Shadow class methods
// Part of Silence, an experimental rendering engine

#include "shadow.h"

namespace Silence {

    double Shadow::occluded( const Vector& point ) const
    {
        if ( umbra.contains(point) )
            return 1;
        // Temporary experimental solution; the real one will be continuous.
        if ( penumbra.contains(point) )
            return 0.5;
        return 0;
    }

}

