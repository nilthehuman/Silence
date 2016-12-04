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

    void Renderer::render( int /*time*/, int depth, int level, double cutoff, double gamma )
    {
        assert( !rendering );
        rendering = true;
        /* TODO: time control... */
        buildZoneForest( 0, depth, level, cutoff );
        rasterizeByZone(0, level, gamma);
        /*...*/
        rendering = false;
    }

    void Renderer::buildZoneForest( int /*time*/, int depth, int level, double cutoff )
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
            for ( int d = 1; d < depth && (-1 == level || d - 1 < level); ++d )
            {
                std::vector< Tree<Zone>* > leaves = (*tree)->getLeaves();
                for ( std::vector< Tree<Zone>* >::iterator leaf = leaves.begin(); leaf != leaves.end(); leaf++ )
                {
                    const Triplet& color = (*leaf)->getValue()->getLight().getColor();
                    if ( color.x + color.y + color.z <= max(0, cutoff) )
                        continue;
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
        {
            std::cerr << "done." << std::endl;
            int totalCount = 0;
            for ( ForestIt tree = zoneForest.begin(); tree != zoneForest.end(); tree++ )
                totalCount += (*tree)->count();
            std::cerr << "Renderer: created " << totalCount << " Zones total in " << zoneForest.size() << " Trees." << std::endl;
        }
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
    // (Pixel by pixel method)
    /*
    void Renderer::rasterizeByPixel( int time, int level, double gamma )
    {
        TODO implement
    }
    */

    // Rasterize all Zones in zoneForest to each Camera
    // (Pixel by pixel method)
    void Renderer::rasterizeByZone( int /*time*/, int level, double gamma )
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
                for ( int thisLevel = 0; (-1 == level || thisLevel <= level) && !children.empty(); ++thisLevel )
                {
                    std::vector< Tree<Zone>* > grandchildren;
                    for ( Tree<Zone>::TreeIt child = children.begin(); child != children.end(); child++ )
                    {
                        if ( -1 == level || thisLevel == level )
                            pathsTotal += (*child)->getValue()->rasterize( *camera );
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
        {
            std::cerr << "done." << std::endl;
            std::cerr << "Renderer: total paths used: " << pathsTotal << std::endl;
        }
    }

}
