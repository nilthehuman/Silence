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

// Interactive graphical interface powered by OpenGL and GLUT
// Part of Retra, the Reference Tracer

#ifndef RETRA_GUI
#define RETRA_GUI

#include <cassert>
#include <vector>

#include "motion.h"

namespace Retra {

    class Camera;

    class GUI {

        struct KeysPressed {
            bool w, a, s, d;
            bool h, j, k, l;
            bool up, down, left, right;
            bool x, X, y, Y, z, Z;
        };

    public:
        GUI( Camera* camera )
            : camera( camera )
            , windowId( -1 )
            , depth( -1 )
            , rrLimit( -1 )
            , gamma( -1 )
            , refreshTime( -1 )
            , lastMoveObjects( -1 )
            , clockError( -1 )
        {
            assert( !self );
            self = this;
            keys.w  = keys.a    = keys.s    = keys.d     = false;
            keys.h  = keys.j    = keys.k    = keys.l     = false;
            keys.up = keys.down = keys.left = keys.right = false;
            keys.x  = keys.X    = keys.y    = keys.Y     =
                                  keys.z    = keys.Z     = false;
        }
        ~GUI()
        {
            for ( std::vector< Motion* >::iterator m = motions.begin(); m != motions.end(); m++ )
                delete *m;
        }

        void initialize( int* argc, char* argv[] );
        void setup( int depth, double rrLimit, double gamma, int refreshTime /*millisecs*/, const std::vector< Motion* >& motions );
        void run();

    private:
        static void redisplay();
        static void refresh( int );
        static void moveObjects( int );
        static void moveAndTurnCamera( int );
        static void handleKeyPress( unsigned char key, int, int );
        static void handleKeyRelease( unsigned char key, int, int );
        static void handleArrowKeyPress( int key, int, int );
        static void handleArrowKeyRelease( int key, int, int );
        static void undoReshape( int, int );

    private:
        static const int    moveObjectsTime;
        static const int    moveAndTurnTime;
        static const double moveStep;
        static const double turnStep; // Radians

        // Member function pointers are not function pointers so we need this hack
        static GUI* self;

        Camera*     camera;
        int         windowId;
        KeysPressed keys;

        int         depth;
        double      rrLimit;
        double      gamma;
        int         refreshTime;
        std::vector< Motion* > motions;
        clock_t     lastMoveObjects;
        int         clockError; // Millisecs
    };

}

#endif // RETRA_GUI

