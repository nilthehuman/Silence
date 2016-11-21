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

// Renderer class methods
// Part of Silence, an experimental rendering engine

#include "renderer.h"

#include <chrono>

#include "camera.h"
#include "scene.h"
#include "zone.h"

namespace Silence {

    void Renderer::addCamera( Camera* camera )
    {
        assert( camera && camera->getScene() );
        cameras.push_back( camera );
    }

    void Renderer::removeCamera( unsigned int i )
    {
        assert( i < cameras.size() );
        cameras.erase( cameras.begin() + i );
    }

    void Renderer::render( int /*time*/, int /*depth*/, double gamma )
    {
        assert( !rendering );
        rendering = true;
        /* TODO: time control... */
        buildZoneForest(0, 0);
        rasterize(0, gamma);
        /*...*/
        rendering = false;
    }

    void Renderer::buildZoneForest( int /*time*/, int /*depth*/ )
    {
        if ( zoneForestReady )
            return;
        if ( modeFlags.verbose )
            std::cout << "Renderer: tracing Zones from lightsources... ";
        /* TODO: time control... */
        for ( LightIt light = scene->lightsBegin(); light != scene->lightsEnd(); light++ )
            (*light)->emitZones( zoneForest );
        for ( ForestIt tree = zoneForest.begin(); tree != zoneForest.end(); tree++ )
        {
            (*tree)->getValue().occlude();
            // Consider only root Zones for now (no recursion)
            std::vector< Zone > children = (*tree)->getValue().bounce();
            for ( std::vector< Zone >::iterator child = children.begin(); child != children.end(); child++ )
               (*tree)->addChild(*child);
        }

        if ( modeFlags.verbose )
            std::cout << "done." << std::endl;
        zoneForestReady = true;
    }

    void Renderer::clearZoneForest()
    {
        zoneForestReady = false;
        for ( ForestIt tree = zoneForest.begin(); tree != zoneForest.end(); tree++ )
            delete *tree;
        zoneForest.clear();
    }

    // Rasterize all Zones in zoneForest to each Camera
    void Renderer::rasterize( int /*time*/, double gamma )
    {
        assert( zoneForestReady );
        if ( modeFlags.verbose )
            std::cout << "Renderer: rasterizing Zones to Cameras... ";
        /* TODO: time control... */
        for ( CameraIt camera = cameras.begin(); camera != cameras.end(); camera++ )
        {
            (*camera)->clear();
            for ( ForestIt tree = zoneForest.begin(); tree != zoneForest.end(); tree++ )
            {
                (*tree)->getValue().rasterize( *camera );
                // TODO: Recursion!
                for ( Tree<Zone>::TreeIt child = (*tree)->childrenBegin(); child != (*tree)->childrenEnd(); child++ )
                    (*child)->getValue().rasterize( *camera );
            }
            (*camera)->paintSky();
            (*camera)->gammaCorrect( gamma );
        }
        if ( modeFlags.verbose )
            std::cout << "done." << std::endl;
    }

}
