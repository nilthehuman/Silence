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

    void Renderer::render( int /*time*/, int depth, double gamma )
    {
        assert( !rendering );
        rendering = true;
        /* TODO: time control... */
        buildZoneForest( 0, depth );
        rasterize(0, gamma);
        /*...*/
        rendering = false;
    }

    void Renderer::buildZoneForest( int /*time*/, int depth )
    {
        if ( zoneForestReady )
            return;
        if ( modeFlags.verbose )
            std::cerr << "Renderer: tracing Zones from lightsources... " << std::flush;
        zoneForest.clear();
        /* TODO: time control... */
        for ( LightIt light = scene->lightsBegin(); light != scene->lightsEnd(); light++ )
            (*light)->emitZones( zoneForest );
        for ( ForestIt tree = zoneForest.begin(); tree != zoneForest.end(); tree++ )
        {
            (*tree)->getValue()->setNode( *tree );
            (*tree)->getValue()->occlude();
            for ( int d = 1; d < depth; ++d )
            {
                std::vector< Tree<Zone>* > leaves = (*tree)->getLeaves();
                for ( std::vector< Tree<Zone>* >::iterator leaf = leaves.begin(); leaf != leaves.end(); leaf++ )
                {
                    std::vector< Zone* > children = (*leaf)->getValue()->bounce();
                    for ( std::vector< Zone* >::iterator child = children.begin(); child != children.end(); child++ )
                    {
                        Tree< Zone >* node = (*leaf)->addChild(*child);
                        (*child)->setNode( node );
                    }
                }
            }
        }

        if ( modeFlags.verbose )
            std::cerr << "done." << std::endl;
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
            std::cerr << "Renderer: rasterizing Zones to Cameras... " << std::flush;
        /* TODO: time control... */
        for ( CameraIt camera = cameras.begin(); camera != cameras.end(); camera++ )
        {
            (*camera)->clear();
            for ( ForestIt tree = zoneForest.begin(); tree != zoneForest.end(); tree++ )
            {
                std::vector< Tree<Zone>* > children;
                children.push_back( *tree );
                while ( !children.empty() )
                {
                    std::vector< Tree<Zone>* > grandchildren;
                    for ( Tree<Zone>::TreeIt child = children.begin(); child != children.end(); child++ )
                    {
                        (*child)->getValue()->rasterize( *camera );
                        for ( Tree<Zone>::TreeIt grandchild = (*child)->childrenBegin(); grandchild != (*child)->childrenEnd(); grandchild++ )
                            grandchildren.push_back( *grandchild );
                    }
                    children = grandchildren;
                }
            }
            (*camera)->paintSky();
            (*camera)->gammaCorrect( gamma );
        }
        if ( modeFlags.verbose )
            std::cerr << "done." << std::endl;
    }

}
