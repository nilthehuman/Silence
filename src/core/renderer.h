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

// Renderer: a manager class to conduct the rendering process
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_RENDERER
#define SILENCE_RENDERER

#include <atomic>
#include <vector>

#include "tree.h"

namespace Silence {

    class Camera;
    class Scene;
    class Zone;

    class Renderer {

        typedef std::vector< Tree<Zone>* >::iterator ForestIt;
        typedef std::vector< Camera* >    ::iterator CameraIt;

    public:
        Renderer( const Scene* scene )
            : scene( scene )
            , cameras()
            , zoneForest()
            , zoneForestReady( false )
            , rendering( false )
            , pathsTotal( 0 )
        { }

        void addCamera( Camera* camera );
        void removeCamera( unsigned int i );

        void render( int time, int depth, int level = -1, double cutoff = 0, double gamma = 1 );

    private:
        // Phase One
        void buildZoneForest( int time, int depth, int level = -1, double cutoff = 0 );
        void clearZoneForest();

        // Phase Two
        void rasterizeByPixel( int time, int level, double gamma );
        void rasterizeByZone ( int time, int level, double gamma );

    private:
        const Scene* const scene;

        std::vector< Camera* >     cameras;
        std::vector< Tree<Zone>* > zoneForest;

        // State and housekeeping
        bool zoneForestReady;
        bool rendering;
        std::atomic< int > pathsTotal;
    };

}

#endif // SILENCE_RENDERER

