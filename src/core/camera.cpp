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

// Camera class methods
// Part of Silence, an experimental rendering engine

#include "camera.h"

#include <limits>

#include "scene.h"

namespace Silence {

    // Reset pixel buffer to all black
    void Camera::clear()
    {
        assert( !rendering );
        #pragma omp parallel for
        for ( int row = 0; row < screen.gridheight; ++row )
            for ( int col = 0; col < screen.gridwidth; ++col )
                pixels[row][col] = RGB::Black;
        if ( modeFlags.verbose )
            std::cerr << "                                            " << '\r' << std::flush;
    }

    // Return the dummy Plane the Screen lies on
    const Plane Camera::getPlane() const
    {
        const Vector& normal = ( screen.window[1] - screen.window[0] ).cross( screen.window[2] - screen.window[0] );
        // ax + by + cz = d
        const double offset = normal * screen.window[0];
        return Plane( normal, offset );
    }

    // Receive a Zone's contributions to the final image
    void Camera::contribute( const RGB** buffer )
    {
        for ( int row = 0; row < screen.gridheight; ++row )
            for ( int col = 0; col < screen.gridwidth; ++col )
                pixels[row][col] += buffer[row][col];
    }

    Vector Camera::getLeftEdge( int row ) const
    {
        const Vector leftEdge = screen.window[0] + (screen.window[2] - screen.window[0]) * ((0.5 + row) / screen.gridheight );
        return leftEdge;
    }

    Vector Camera::getRightEdge( int row ) const
    {
        const Vector rightEdge = getLeftEdge( row ) + (screen.window[1] - screen.window[0]);
        return rightEdge;
    }

    void Camera::gammaCorrect( double gamma )
    {
        assert( !rendering );
        if ( 1 == gamma )
            return;
        #pragma omp parallel for
        for ( int row = 0; row < screen.gridheight; ++row )
            for ( int col = 0; col < screen.gridwidth; ++col )
                pixels[row][col].gamma( gamma );
    }

    // Write rendering results to character stream in PPM format
    void Camera::writePixels( std::ostream& os ) const
    {
        assert( !rendering );
        if ( modeFlags.verbose )
            std::cerr << "Camera: writing pixels to stream... ";
        os << "P3\n" << screen.gridwidth << " " << screen.gridheight << "\n" << 255 << "\n";
        for ( int row = 0; row < screen.gridheight; ++row )
            for ( int col = 0; col < screen.gridwidth; ++col )
            {
                Triplet rgb255 = pixels[row][col].project( 0, 255 );
                os << int(rgb255.x + 0.5) << " " << int(rgb255.y + 0.5) << " " << int(rgb255.z + 0.5) << " ";
            }
        if ( modeFlags.verbose )
            std::cerr << "done." << std::endl;
    }

    // Translate both the viewpoint and the screen in camera space
    void Camera::move( double delta, Axis chosenAxis )
    {
        assert( !rendering );
        clear();
        // Screen position relative to the viewpoint
        const Screen relScreen( screen.window[0] - viewpoint,
                                screen.window[1] - viewpoint,
                                screen.window[2] - viewpoint,
                                screen.window[3] - viewpoint,
                                0, 0 );
        Vector translation;
        switch ( chosenAxis )
        {
            case AXIS_X:
                translation = (relScreen.window[1] - relScreen.window[0]).normalized();
                break;
            case AXIS_Y:
                translation = (relScreen.window[0] - relScreen.window[2]).normalized();
                break;
            case AXIS_Z:
                translation = (relScreen.window[0] + relScreen.window[1] +
                               relScreen.window[2] + relScreen.window[3]).normalized() * -1.0;
                break;
            default:
                assert( false );
        }
        viewpoint += translation * delta;
        for ( int i = 0; i < 4; ++i )
            screen.window[i] += translation * delta;
    }

    // Rotate the screen in the positive direction around one of the camera axes
    void Camera::turn( double theta, Axis chosenAxis )
    {
        assert( !rendering );
        clear();
        // Screen position relative to the viewpoint
        const Screen relScreen( screen.window[0] - viewpoint,
                                screen.window[1] - viewpoint,
                                screen.window[2] - viewpoint,
                                screen.window[3] - viewpoint,
                                0, 0 );
        // The actual axis direction
        Vector axisUnit;

        // The Camera always faces the -Z direction
        switch ( chosenAxis )
        {
            case AXIS_X:
                axisUnit = (relScreen.window[1] - relScreen.window[0]).normalized();
                break;
            case AXIS_Y:
                axisUnit = (relScreen.window[0] - relScreen.window[2]).normalized();
                break;
            case AXIS_Z:
                axisUnit = (relScreen.window[0] + relScreen.window[1] +
                            relScreen.window[2] + relScreen.window[3]).normalized() * -1.0;
                break;
            default:
                assert( false );
        }

        // https://www.fastgraph.com/makegames/3drotation/
        const double c = cos( theta );
        const double s = sin( theta );
        const double t = 1 - c;
        // The rotation matrix
        Vector rotation[3] = {
            Vector( t*axisUnit.x*axisUnit.x + c, t*axisUnit.x*axisUnit.y - s*axisUnit.z, t*axisUnit.x*axisUnit.z + s*axisUnit.y ),
            Vector( t*axisUnit.x*axisUnit.y + s*axisUnit.z, t*axisUnit.y*axisUnit.y + c, t*axisUnit.y*axisUnit.z - s*axisUnit.x ),
            Vector( t*axisUnit.x*axisUnit.z - s*axisUnit.y, t*axisUnit.y*axisUnit.z + s*axisUnit.x, t*axisUnit.z*axisUnit.z + c )
        };

        Vector topLeft(
            rotation[0].x * relScreen.window[0].x + rotation[0].y * relScreen.window[0].y + rotation[0].z * relScreen.window[0].z,
            rotation[1].x * relScreen.window[0].x + rotation[1].y * relScreen.window[0].y + rotation[1].z * relScreen.window[0].z,
            rotation[2].x * relScreen.window[0].x + rotation[2].y * relScreen.window[0].y + rotation[2].z * relScreen.window[0].z
        );
        Vector topRight(
            rotation[0].x * relScreen.window[1].x + rotation[0].y * relScreen.window[1].y + rotation[0].z * relScreen.window[1].z,
            rotation[1].x * relScreen.window[1].x + rotation[1].y * relScreen.window[1].y + rotation[1].z * relScreen.window[1].z,
            rotation[2].x * relScreen.window[1].x + rotation[2].y * relScreen.window[1].y + rotation[2].z * relScreen.window[1].z
        );
        Vector bottomLeft(
            rotation[0].x * relScreen.window[2].x + rotation[0].y * relScreen.window[2].y + rotation[0].z * relScreen.window[2].z,
            rotation[1].x * relScreen.window[2].x + rotation[1].y * relScreen.window[2].y + rotation[1].z * relScreen.window[2].z,
            rotation[2].x * relScreen.window[2].x + rotation[2].y * relScreen.window[2].y + rotation[2].z * relScreen.window[2].z
        );
        Vector bottomRight(
            rotation[0].x * relScreen.window[3].x + rotation[0].y * relScreen.window[3].y + rotation[0].z * relScreen.window[3].z,
            rotation[1].x * relScreen.window[3].x + rotation[1].y * relScreen.window[3].y + rotation[1].z * relScreen.window[3].z,
            rotation[2].x * relScreen.window[3].x + rotation[2].y * relScreen.window[3].y + rotation[2].z * relScreen.window[3].z
        );
        screen.window[0] = viewpoint + topLeft;
        screen.window[1] = viewpoint + topRight;
        screen.window[2] = viewpoint + bottomLeft;
        screen.window[3] = viewpoint + bottomRight;
    }

}

