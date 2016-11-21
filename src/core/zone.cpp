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

    // Find all Things obstructing the light Beam
    void Zone::occlude()
    {
        for ( ThingIt thing = light.getScene()->thingsBegin(); thing != light.getScene()->thingsEnd(); thing++ )
            for ( ThingPartIt part = (*thing)->partsBegin(); part != (*thing)->partsEnd(); part++ )
                light.occlude();
    }

    // Create all Zones stemming from this one
    std::vector< Zone > Zone::bounce()
    {
       std::vector< Beam > newBeams;
       std::vector< Zone > newZones;
       for ( ThingIt thing = light.getScene()->thingsBegin(); thing != light.getScene()->thingsEnd(); thing++ )
            for ( ThingPartIt part = (*thing)->partsBegin(); part != (*thing)->partsEnd(); part++ )
                if ( hit(*part) && !eclipsed(*part) )
                {
                    const Thing* thing = (const Thing*)( (*part)->getParent() );
                    if ( !equal(0, thing->interact(Material::DIFFUSE) ) )
                        newBeams.push_back( light.bounce(*part, Material::DIFFUSE) );
                    // if other Interactions...
                }
        for ( std::vector< Beam >::const_iterator beam = newBeams.begin(); beam != newBeams.end(); beam++ )
            newZones.push_back( Zone(*beam) );
        return newZones;
    }

    // Contribute to the final image in a Camera
    void Zone::rasterize( Camera* camera ) const
    {
        const int width  = camera->getGridwidth();
        const int height = camera->getGridheight();
        const Vector viewpoint = camera->getViewpoint();

        bool cameraHit = light.contains( viewpoint ) && !camera->behind( light.getApex() );

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
            RGB** pixelBuffer = new RGB*[height];
            for ( int row = 0; row < height; ++row )
                pixelBuffer[row] = new RGB[width];
            double** skyBlocked = new double*[height];
            for ( int row = 0; row < height; ++row )
                skyBlocked[row] = new double[width];

            for ( int row = 0; row < height; ++row )
                rasterizeRow( camera, row, pixelBuffer[row], skyBlocked[row] );
            // Write results directly in Camera's pixels array:
            // contributions from all Zones will be superimposed on each other
            for ( int row = 0; row < height; ++row )
                for ( int col = 0; col < width; ++col )
                {
                    if ( RGB::Black != pixelBuffer[row][col] )
                        camera->pixels[row][col] += pixelBuffer[row][col];
                    if ( !equal(0, skyBlocked[row][col]) )
                        camera->skyMask[row][col] -= skyBlocked[row][col];
                }

            for ( int i = 0; i < height; ++i )
                delete[] pixelBuffer[i];
            delete[] pixelBuffer;
            for ( int i = 0; i < height; ++i )
                delete[] skyBlocked[i];
            delete[] skyBlocked;
        }
    }

    bool Zone::hit( const ThingPart* part ) const
    {
        // Temporary solution.
        // TODO: accurate algorithm for narrow Beams.
        const std::vector< Vector > points = part->getPoints( light.getApex() );
        for ( std::vector< Vector >::const_iterator point = points.begin(); point != points.end(); point++ )
            if ( light.contains(*point) )
                return true;
        return false;
    }

    bool Zone::eclipsed( const ThingPart* part ) const
    {
        for ( std::vector< Shadow >::const_iterator shadow = shadows.begin(); shadow != shadows.end(); shadow++ )
        {
            bool eclipsed = true;
            const std::vector< Vector > points = part->getPoints( light.getApex() );
            for ( std::vector< Vector >::const_iterator point = points.begin(); point != points.end(); point++ )
                if ( !equal(1, (*shadow).occluded(*point)) )
                {
                    eclipsed = false;
                    break;
                }
            if ( eclipsed )
                return true;
        }
        return false;
    }

    void Zone::rasterizeRow( const Camera* camera, int row, RGB* pixelBuffer, double* skyBlocked ) const
    {
        // Basic light color
        light.rasterizeRow( camera, row, pixelBuffer, skyBlocked );

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
            pixelBuffer[col] *= (1 - shadowMask[col]);
        }
        delete[] shadowMask;
    }

}

