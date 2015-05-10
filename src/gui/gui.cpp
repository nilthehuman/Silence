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

#include "gui.h"

#include <GL/gl.h>
#include <GL/glut.h>

#include <ctime>

#include "../core/camera.h"
#include "../core/triplet.h"

namespace Retra {

    const int    GUI::moveObjectsTime = 10;
    const int    GUI::moveAndTurnTime = 10;
    const double GUI::moveStep        =  1;
    const double GUI::turnStep        =  0.1;

    GUI* GUI::self = NULL;

    void GUI::redisplay()
    {
        const RGB** pixels = self->camera->getPixels();
        const int   width  = self->camera->getGridwidth();
        const int   height = self->camera->getGridheight();
        float* floatPixels = new float[ width * height * 3 ];
        for ( int row = 0; row < height; ++row )
            for ( int col = 0; col < width; ++col )
            {
                // OpenGL counts rows from the bottom up
                floatPixels[ row*width*3 + col*3 + 0 ] = pixels[ height - row - 1 ][ col ].x;
                floatPixels[ row*width*3 + col*3 + 1 ] = pixels[ height - row - 1 ][ col ].y;
                floatPixels[ row*width*3 + col*3 + 2 ] = pixels[ height - row - 1 ][ col ].z;
            }
        glDrawPixels( self->camera->getGridwidth(), self->camera->getGridheight(), GL_RGB, GL_FLOAT, floatPixels );
        glutSwapBuffers();
        delete[] floatPixels;
    }

    void GUI::refresh( int )
    {
        if ( -1 == self->windowId )
            return;
        glutTimerFunc( self->refreshTime, &refresh, 0 );
        self->clockError += self->camera->render( self->refreshTime * 0.8, self->depth, self->rrLimit );
        glutPostRedisplay();
    }

    void GUI::moveObjects( int )
    {
        if ( -1 == self->windowId )
            return;
        glutTimerFunc( self->moveObjectsTime, &moveObjects, 0 );
        const clock_t now = clock();
        const double  elapsedTime = (double)(now - self->lastMoveObjects) / CLOCKS_PER_SEC - 0.001 * self->clockError;
        for ( std::vector< Motion* >::iterator m = self->motions.begin(); m != self->motions.end(); m++ )
            (*m)->step( elapsedTime );
        self->clockError      = 0;
        self->lastMoveObjects = now;
    }

    void GUI::moveAndTurnCamera( int )
    {
        glutTimerFunc( self->moveAndTurnTime, &moveAndTurnCamera, 0 );

        if ( self->keys.w     && !self->keys.s     ) self->camera->move( -moveStep, Camera::AXIS_Z );
        if ( self->keys.a     && !self->keys.d     ) self->camera->move( -moveStep, Camera::AXIS_X );
        if ( self->keys.s     && !self->keys.w     ) self->camera->move(  moveStep, Camera::AXIS_Z );
        if ( self->keys.d     && !self->keys.a     ) self->camera->move(  moveStep, Camera::AXIS_X );

        if ( self->keys.h     && !self->keys.l     ) self->camera->move( -moveStep, Camera::AXIS_X );
        if ( self->keys.j     && !self->keys.k     ) self->camera->move( -moveStep, Camera::AXIS_Y );
        if ( self->keys.k     && !self->keys.j     ) self->camera->move(  moveStep, Camera::AXIS_Y );
        if ( self->keys.l     && !self->keys.h     ) self->camera->move(  moveStep, Camera::AXIS_X );

        if ( self->keys.up    && !self->keys.down  ) self->camera->move(  moveStep, Camera::AXIS_Y );
        if ( self->keys.down  && !self->keys.up    ) self->camera->move( -moveStep, Camera::AXIS_Y );
        if ( self->keys.left  && !self->keys.right ) self->camera->move( -moveStep, Camera::AXIS_X );
        if ( self->keys.right && !self->keys.left  ) self->camera->move(  moveStep, Camera::AXIS_X );

        if ( self->keys.x )                          self->camera->turn(  turnStep, Camera::AXIS_X );
        if ( self->keys.X )                          self->camera->turn( -turnStep, Camera::AXIS_X );
        if ( self->keys.y )                          self->camera->turn(  turnStep, Camera::AXIS_Y );
        if ( self->keys.Y )                          self->camera->turn( -turnStep, Camera::AXIS_Y );
        if ( self->keys.z )                          self->camera->turn(  turnStep, Camera::AXIS_Z );
        if ( self->keys.Z )                          self->camera->turn( -turnStep, Camera::AXIS_Z );
    }

    void GUI::handleKeyPress( unsigned char key, int, int )
    {
        switch ( key ) {
            case 'w': self->keys.w = true; break;
            case 'a': self->keys.a = true; break;
            case 's': self->keys.s = true; break;
            case 'd': self->keys.d = true; break;
            case 'h': self->keys.h = true; break;
            case 'j': self->keys.j = true; break;
            case 'k': self->keys.k = true; break;
            case 'l': self->keys.l = true; break;

            case 'x':
                self->keys.x = true;
                if ( self->keys.X )
                    self->keys.X = false;
                break;
            case 'X':
                self->keys.X = true;
                if ( self->keys.x )
                    self->keys.x = false;
                break;
            case 'y':
                self->keys.y = true;
                if ( self->keys.Y )
                    self->keys.Y = false;
                break;
            case 'Y':
                self->keys.Y = true;
                if ( self->keys.y )
                    self->keys.y = false;
                break;
            case 'z':
                self->keys.z = true;
                if ( self->keys.Z )
                    self->keys.Z = false;
                break;
            case 'Z':
                self->keys.Z = true;
                if ( self->keys.z )
                    self->keys.z = false;
                break;
            case  27: // Escape key
            case 'q':
                glutDestroyWindow( self->windowId );
                self->windowId = -1;
                break;
            default :
                ;
        }
    }

    void GUI::handleKeyRelease( unsigned char key, int, int )
    {
        switch ( key ) {
            case 'w':
            case 'W': self->keys.w = false; break;
            case 'a':
            case 'A': self->keys.a = false; break;
            case 's':
            case 'S': self->keys.s = false; break;
            case 'd':
            case 'D': self->keys.d = false; break;
            case 'h':
            case 'H': self->keys.h = false; break;
            case 'j':
            case 'J': self->keys.j = false; break;
            case 'k':
            case 'K': self->keys.k = false; break;
            case 'l':
            case 'L': self->keys.l = false; break;

            case 'x': self->keys.x = false; break;
            case 'X': self->keys.X = false; break;
            case 'y': self->keys.y = false; break;
            case 'Y': self->keys.Y = false; break;
            case 'z': self->keys.z = false; break;
            case 'Z': self->keys.Z = false; break;
            case  27: // Escape key
            case 'q': glutDestroyWindow( self->windowId ); self->windowId = -1; break;
            default : ;
        }
    }

    void GUI::handleArrowKeyPress( int key, int, int )
    {
        switch ( key ) {
            case GLUT_KEY_UP:    self->keys.up    = true; break;
            case GLUT_KEY_DOWN:  self->keys.down  = true; break;
            case GLUT_KEY_LEFT:  self->keys.left  = true; break;
            case GLUT_KEY_RIGHT: self->keys.right = true; break;
            default: ;
        }
    }

    void GUI::handleArrowKeyRelease( int key, int, int )
    {
        switch ( key ) {
            case GLUT_KEY_UP:    self->keys.up    = false; break;
            case GLUT_KEY_DOWN:  self->keys.down  = false; break;
            case GLUT_KEY_LEFT:  self->keys.left  = false; break;
            case GLUT_KEY_RIGHT: self->keys.right = false; break;
            default: ;
        }
    }

    void GUI::undoReshape( int, int )
    {
        glutReshapeWindow( self->camera->getGridwidth(), self->camera->getGridheight() );
    }

    void GUI::initialize( int* argc, char* argv[] )
    {
        glutInitWindowPosition( 200, 100 );
        glutInitWindowSize( camera->getGridwidth(), camera->getGridheight() );
        glutInit( argc, argv );
        glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );
        windowId = glutCreateWindow( "Retra" );
        glutSetWindow( windowId );

        glutDisplayFunc( &redisplay );
        glutKeyboardFunc( &handleKeyPress );
        glutKeyboardUpFunc( &handleKeyRelease );
        glutSpecialFunc( &handleArrowKeyPress );
        glutSpecialUpFunc( &handleArrowKeyRelease );
        glutReshapeFunc( &undoReshape );

        glClearColor( 0, 0, 0, 1 );
        glClear( GL_COLOR_BUFFER_BIT );
        glutSwapBuffers();
    }

    void GUI::setup( int depth, double rrLimit, int refreshTime, const std::vector< Motion* >& motions )
    {
        this->depth       = depth;
        this->rrLimit     = rrLimit;
        this->refreshTime = refreshTime;
        this->motions     = motions;
    }

    void GUI::run()
    {
        this->clockError      = 0;
        this->lastMoveObjects = clock();
        glutTimerFunc( refreshTime,     &refresh,           0 );
        glutTimerFunc( moveObjectsTime, &moveObjects,       0 );
        glutTimerFunc( moveAndTurnTime, &moveAndTurnCamera, 0 );
        glutMainLoop();
    }

}

