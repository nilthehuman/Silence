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

// Zone class methods
// Part of Silence, an experimental rendering engine

#include "zone.h"

#include <cstdlib>

namespace Silence {

    // Contribute to the final image in a Camera
    void Zone::rasterize( Camera* camera ) const
    {
        const int width  = camera->getGridwidth();
        const int height = camera->getGridheight();
        RGB** buffer = new RGB*[height];
        for ( int row = 0; row < height; ++row )
            buffer[row] = new RGB[width];

        const Plane cameraPlane = camera->getPlane();
        #pragma omp parallel for
        for ( int row = 0; row < screen.gridheight; ++row )
        {
            const Vector leftEdge  = screen.window[0] + (screen.window[2] - screen.window[0]) * ((0.5 + row) / screen.gridheight );
            const Vector rightEdge = leftEdge + (screen.window[1] - screen.window[0]);
            rasterizeRow( cameraPlane, leftEdge, rightEdge, width, buffer[row] );
        }
        // Write results directly in Camera's pixels array:
        // contributions from all Zones will be superimposed on each other
        camera->contribute( buffer );

        for ( int i = 0; i < height; ++i )
            delete[] buffer[i];
        delete[] buffer;
    }

    bool Zone::russianRoulette( double rrLimit ) const
    {
        // Russian roulette is a common heuristic for path termination
        // Here we use a variant based on current color intensity
        // A lower rrLimit keeps more Zones alive
        const Triplet color = light->getColor();
        if ( max(color.x, max(color.y, color.z)) < (double)std::rand() * rrLimit / RAND_MAX )
            return true;
        return false;
    }

}

