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

// A parser for importing JSON motion descriptions.
// The motion description determines how Objects move around in the Scene
// Part of Retra, the Reference Tracer

#ifndef RETRA_PARSEMOTIONS
#define RETRA_PARSEMOTIONS

#include <istream>
#include <vector>

namespace Retra {

    class Motion;
    class Scene;

    std::vector< Motion* > parseMotions( std::istream& is, const Scene* const scene );

}

#endif // RETRA_PARSEMOTIONS

