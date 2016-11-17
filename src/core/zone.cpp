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

#include "camera.h"
#include "scene.h"

namespace Silence {

    // Contribute to the final image in a Camera
    void Zone::rasterize( Camera* camera ) const
    {
        const int width  = camera->getGridwidth();
        const int height = camera->getGridheight();
        const Vector viewpoint = camera->getViewpoint();
        RGB** buffer = new RGB*[height];
        for ( int row = 0; row < height; ++row )
            buffer[row] = new RGB[width];

        bool cameraHit = light.contains( viewpoint );

        if ( cameraHit )
            for ( std::vector< Shadow >::const_iterator shadow = shadows.begin(); shadow != shadows.end(); ++shadow )
            {
                if ( equal( 1, (*shadow).occluded(viewpoint) ) )
                {
                    cameraHit = false;
                    break;
                }
            }

        if ( cameraHit )
        {
            for ( int row = 0; row < height; ++row )
                rasterizeRow( camera, row, buffer[row] );
            // Write results directly in Camera's pixels array:
            // contributions from all Zones will be superimposed on each other
            for ( int row = 0; row < height; ++row )
                for ( int col = 0; col < width; ++col )
                    if ( RGB::Black != buffer[row][col] )
                        camera->pixels[row][col] += buffer[row][col];
        }

        for ( int i = 0; i < height; ++i )
            delete[] buffer[i];
        delete[] buffer;
    }

    void Zone::rasterizeRow( const Camera* camera, int row, RGB* buffer ) const
    {
        // Basic light color
        light.rasterizeRow( camera, row, buffer );

        // Temper basic incoming light with the occlusion from Shadows
        const int gridwidth = camera->getGridwidth();
        const Vector leftEdge     = camera->getLeftEdge ( row );
        const Vector rowDirection = camera->getRightEdge( row ) - leftEdge;
        double* shadowMask = new double[gridwidth];
        for ( int col = 0; col < gridwidth; ++col )
        {
            const Vector screenPoint = leftEdge + rowDirection * ( (double)col/gridwidth );
            shadowMask[col] = 0;
            for ( std::vector< Shadow >::const_iterator shadow = shadows.begin(); shadow != shadows.end(); shadow++ )
                shadowMask[col] += (*shadow).occluded( screenPoint );
            buffer[col] *= (1 - shadowMask[col]);
        }
        delete[] shadowMask;
    }

    bool Zone::russianRoulette( double rrLimit ) const
    {
        // Russian roulette is a common heuristic for path termination
        // Here we use a variant based on current color intensity
        // A lower rrLimit keeps more Zones alive
        const Triplet color = light.getColor();
        if ( max(color.x, max(color.y, color.z)) < (double)std::rand() * rrLimit / RAND_MAX )
            return true;
        return false;
    }

}

