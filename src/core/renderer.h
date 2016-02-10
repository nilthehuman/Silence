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

#include <map>

#include "tree.h"

namespace Silence {

    class Camera;
    class Scene;
    class Zone;

    class Renderer {
    public:
        Renderer( const Scene* scene )
            : scene( scene )
            , zoneForest()
            , zoneForestReady( false )
            , rendering( false )
        { }

        void addCamera( Camera* camera );
        void removeCamera( int i );

        void render( int time, int depth, double gamma = 1 );

    private:
        void buildZoneTree( int time, int depth );
        void clearZoneTree();

        void rasterize( int time );

    private:
        const Scene* const scene;

        std::map< Camera*, Tree<Zone> > zoneForest;

        // State and housekeeping
        bool zoneForestReady;
        bool rendering;
    };

}

#endif // SILENCE_RENDERER

