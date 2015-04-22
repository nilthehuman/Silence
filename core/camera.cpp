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

// Camera class methods
// Part of Retra, the Reference Tracer

#include "camera.h"

#include <ctime>
#include <limits>

#include <omp.h>

namespace Retra {

    static const std::string tokenErrorMessage = "unrecognized token: ";

    // Take a shot of the virtual Scene
    void Camera::capture( int spp, int depth, double rrLimit )
    {
        rendering = true;
        const time_t start = std::time( NULL );
        #pragma omp parallel
        {
            const int threadNum = omp_get_thread_num();
            const int rowBegin  = (double) threadNum      / cpuThreads * screen.gridheight;
            const int rowEnd    = (double)(threadNum + 1) / cpuThreads * screen.gridheight;
            // The engine's core loop
            for ( int row = rowBegin; row < rowEnd; ++row )
            {
                if ( modeFlags.verbose && 2 == threadNum ) // Usually the lower mid thread has the best idea of progress
                {
                    std::cerr << "Camera: rendering... " << (int)((double)(row - rowBegin) / (rowEnd - rowBegin) * 100) << "% done.";
                    if ( 3 == row % 4 )
                    {
                        const int estimate = (int)( (std::time( NULL ) - start) * (double)(rowEnd - row) / (row - rowBegin) );
                        const int m = estimate / 60;
                        const int s = estimate % 60;
                        std::cerr << " Time left: ";
                        if ( m ) std::cerr << m << " min(s) and ";
                        std::cerr << s << " sec(s).              ";
                    }
                    std::cerr << '\r' << std::flush;
                }
                for ( int col = 0; col < screen.gridwidth; ++col )
                {
                    Triplet pixelColorSum; // Adding up the results would yield an invalid (out-of-bounds) RGB
                    Ray ray = generateRay( row, col, depth, rrLimit );
                    ray.traceToNextIntersection(); // Initialize ray (sort of)
                    if ( ray.getDepth() )
                    {
                        for ( int s = 0; s < spp; ++s )
                        {
                            Ray rayClone( ray );
                            pixelColorSum += rayClone.trace();
                        }
                        pixels[row][col] = pixelColorSum / spp;
                    }
                    else
                        pixels[row][col] = ray.getColor();
                }
            }
        }
        if ( modeFlags.verbose )
            std::cerr << "\rCamera: rendering completed.   " << std::endl;
        rendering = false;
    }

    void Camera::gammaCorrect( double gamma )
    {
        assert( !rendering );
        for ( int row = 0; row < screen.gridheight; ++row )
            for ( int col = 0; col < screen.gridwidth; ++col )
                pixels[row][col].gamma( gamma );
    }

    // Shoot ray from camera viewpoint through a given pixel
    Ray Camera::generateRay( int row, int col, int depth, double rrLimit ) const
    {
        const Vector leftEdge  = screen.window[0] + (screen.window[2] - screen.window[0]) * ((0.5 + row) / screen.gridheight );
        const Vector origin    = leftEdge + (screen.window[1] - screen.window[0]) * ((0.5 + col) / screen.gridwidth );
        const Vector direction = origin - viewpoint;
        // Camera emits white Rays as if it's the real light source and Lights are the sinks.
        // This will produce the same RGB results as long as Ray::paint is commutative
        return Ray( scene, origin, direction, RGB::White, depth, rrLimit );
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

}

