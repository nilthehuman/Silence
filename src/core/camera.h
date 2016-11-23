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

// Camera class, defines the point of view and the screen
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_CAMERA
#define SILENCE_CAMERA

#include <fstream>
#include <iostream>

#include "triplet.h"
#include "zone.h"

namespace Silence {

    class Plane;
    class Scene;

    struct ScreenPoint {
        ScreenPoint( int col, int row )
            : col( col )
            , row( row )
        { }

        friend std::ostream& operator<<( std::ostream& os, const ScreenPoint& screenpoint );

        int col, row; // Can represent points outside the actual screen too
    };
    struct BoundingBox {
        BoundingBox( const ScreenPoint& topLeft, const ScreenPoint& bottomRight )
            : topLeft( topLeft )
            , bottomRight( bottomRight )
        { }
        ScreenPoint topLeft, bottomRight;
    };

    class Camera {
    public:
        enum Axis { AXIS_X, AXIS_Y, AXIS_Z };

        // The GUI's going to need these
        struct RenderInfo {
            int  clockError;
            bool sceneChanged;
        };

    private:
        struct Screen {
            Screen() { }
            Screen( Vector topLeft, Vector topRight, Vector bottomLeft, Vector bottomRight, int width, int height )
                : gridwidth( width )
                , gridheight( height )
            {
                window[0] = topLeft;
                window[1] = topRight;
                window[2] = bottomLeft;
                window[3] = bottomRight;
            }

            Vector window[4];
            int gridwidth;
            int gridheight;
        };

    public:
        Camera( const Scene* scene )
            : scene( scene )
            , pixels( NULL )
            , skyMask( NULL )
            , rendering( false )
        { }
        Camera( const Scene* scene, Vector viewpoint, Screen screen, int width, int height )
            : scene( scene )
            , viewpoint( viewpoint )
            , screen( screen )
            , rendering( false )
        {
            pixels = new RGB*[height];
            for ( int i = 0; i < height; ++i )
                pixels[i] = new RGB[width];
            skyMask = new double*[height];
            for ( int i = 0; i < height; ++i )
                skyMask[i] = new double[width];
        }
        ~Camera()
        {
            if ( pixels )
            {
                for ( int i = 0; i < screen.gridheight; ++i )
                    delete[] pixels[i];
                delete[] pixels;
            }
            if ( skyMask )
            {
                for ( int i = 0; i < screen.gridheight; ++i )
                    delete[] skyMask[i];
                delete[] skyMask;
            }
        }

        void clear();
        void paintSky();
        void gammaCorrect( double gamma );

        void writePixels( std::ostream& os ) const;

        void move( double delta, Axis chosenAxis );
        void turn( double theta, Axis chosenAxis );

        const Scene*  getScene()      const { return scene; }
        int           getGridwidth () const { return screen.gridwidth; }
        int           getGridheight() const { return screen.gridheight; }
        const Vector& getViewpoint()  const { return viewpoint; }
        const RGB**   getPixels()     const { return (const RGB**)pixels; }

        const Plane   getPlane()                     const;
        Vector        getScreenX()                   const;
        Vector        getScreenY()                   const;
        Vector        getLeftEdge ( int row )        const;
        Vector        getRightEdge( int row )        const;
        ScreenPoint   project( const Vector& point ) const;
        bool          behind ( const Vector& point ) const;

    private:
        friend void Zone::rasterize( Camera* ) const;

        friend std::istream& operator>>( std::istream& is, Camera& camera );

    private:
        const Scene* const scene;

        Vector   viewpoint;
        Screen   screen;
        RGB**    pixels;  // The end results go here
        double** skyMask; // Empty parts of the screen space
        bool     rendering;
    };

}

#endif // SILENCE_CAMERA

