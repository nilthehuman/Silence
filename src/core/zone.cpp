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

// Zone class methods
// Part of Silence, an experimental rendering engine

#include "zone.h"

#include <algorithm>
#include <cstdlib>

#include "scene.h"

namespace Silence {

    // Add a Surface obstructing the light Beam
    void Zone::occlude( const Surface* surface )
    {
        const Vector& apex = light.getApex();
        // The "outline" of the occluder
        std::vector< Vector > points = surface->getPoints( apex );
        Vector center;
        for ( std::vector< Vector >::const_iterator p = points.begin(); p != points.end(); p++ )
            center += *p;
        center /= points.size();
        // The "outline" of the source of the light Beam
        std::vector< Vector > umbraLightPoints = light.getSource()->getPoints( center );
        std::vector< Vector > penumbraLightPoints( umbraLightPoints );
        std::vector< Vector > umbraPoints, penumbraPoints;
        std::vector< Vector > umbraPairs,  penumbraPairs;
        // Find the corresponding and the opposing lightPoint for each occluder point
        if ( 1 == umbraLightPoints.size() )
            for ( std::vector< Vector >::const_iterator p = points.begin(); p != points.end(); p++ )
            {
                umbraPoints.push_back( *p );
                umbraPairs .push_back( apex );
                penumbraPoints.push_back( *p );
                penumbraPairs .push_back( apex );
            }
        else
        {
            // Perhaps these loops could be refactored in a more functional style?
            // Find the corresponding lightPoint for each occluder point
            while ( umbraLightPoints.size() && points.size() )
            {
                double maxProduct = -1;
                Vector umbraPoint = Vector::Invalid;
                Vector umbraPair  = Vector::Invalid;
                for ( std::vector< Vector >::const_iterator p = points.begin(); p != points.end(); p++ )
                    for ( std::vector< Vector >::const_iterator ulp = umbraLightPoints.begin(); ulp != umbraLightPoints.end(); ulp++ )
                    {
                        const double product = (*ulp-apex).normalized() * (*p-center).normalized();
                        if ( maxProduct < product )
                        {
                            maxProduct = product;
                            umbraPoint = *p;
                            umbraPair  = *ulp;
                        }
                    }
                assert( 0 == umbraLightPoints.size() || Vector::Invalid != umbraPair );
                if ( Vector::Invalid != umbraPair )
                {
                    umbraPoints.push_back( umbraPoint );
                    umbraPairs .push_back( umbraPair );
                }
                points.erase( std::remove(points.begin(), points.end(), umbraPoint), points.end() );
                umbraLightPoints.erase( std::remove(umbraLightPoints.begin(), umbraLightPoints.end(), umbraPair), umbraLightPoints.end() );
            }
            points = surface->getPoints( apex );
            // Find the opposing lightPoint for each occluder point
            while ( penumbraLightPoints.size() && points.size() )
            {
                double minProduct    = 2;
                Vector penumbraPoint = Vector::Invalid;
                Vector penumbraPair  = Vector::Invalid;
                for ( std::vector< Vector >::const_iterator p = points.begin(); p != points.end(); p++ )
                    for ( std::vector< Vector >::const_iterator plp = penumbraLightPoints.begin(); plp != penumbraLightPoints.end(); plp++ )
                    {
                        const double product = (*plp-apex).normalized() * (*p-center).normalized();
                        if ( product < minProduct )
                        {
                            minProduct    = product;
                            penumbraPoint = *p;
                            penumbraPair  = *plp;
                        }
                    }
                assert( 0 == penumbraLightPoints.size() || Vector::Invalid != penumbraPair );
                if ( Vector::Invalid != penumbraPair )
                {
                    penumbraPoints.push_back( penumbraPoint );
                    penumbraPairs .push_back( penumbraPair );
                }
                points.erase( std::remove(points.begin(), points.end(), penumbraPoint), points.end() );
                penumbraLightPoints.erase( std::remove(penumbraLightPoints.begin(), penumbraLightPoints.end(), penumbraPair), penumbraLightPoints.end() );
            }
        }
        std::vector< Ray > umbraEdges, penumbraEdges;
        for ( unsigned int i = 0; i < umbraPoints.size(); ++i )
            umbraEdges.push_back( Ray(scene, umbraPoints[i], umbraPoints[i]-umbraPairs[i]) );
        for ( unsigned int i = 0; i < penumbraPoints.size(); ++i )
            penumbraEdges.push_back( Ray(scene, penumbraPoints[i], penumbraPoints[i]-penumbraPairs[i]) );
        Beam umbra   ( scene, apex, surface, NULL, Ray(scene, center, center-apex),    umbraEdges, RGB::Black, Beam::Zero );
        Beam penumbra( scene, apex, surface, NULL, Ray(scene, center, center-apex), penumbraEdges, RGB::Black, Beam::Zero );
        Shadow newShadow( umbra, penumbra );
        shadows.push_back( newShadow );
    }

    // Create all Zones stemming from this one
    std::vector< Zone* > Zone::bounce()
    {
        std::vector< Beam > newBeams;
        std::vector< Zone* > newZones;
        for ( ThingIt thing = scene->thingsBegin(); thing != scene->thingsEnd(); thing++ )
            for ( ThingPartIt part = (*thing)->partsBegin(); part != (*thing)->partsEnd(); part++ )
            {
                if ( Material::REFRACT == light.getKind() )
                {
                    if ( (*part)->getParent() != light.getSource()->getParent() )
                        continue; // Need to hit the same Thing again
                }
                else if ( (*part) == light.getSource() )
                    continue; // Can't hit the same ThingPart twice in a row

                if ( hit(*part) && !eclipsed(*part) )
                {
                    occlude( *part ); // This Zone is blocked by the Surface
                    const Thing* thing = static_cast<const Thing*>( (*part)->getParent() );
                    // Spawn a separate Zone for each type of Material the Thing has
                    for ( int i = 0; i <= Material::REFRACT; i++ )
                    {
                        const Material::Interaction interaction = Material::Interaction( i );
                        if ( (Material::METALLIC == light.getKind() || Material::REFLECT == light.getKind()) && Material::DIFFUSE == interaction )
                            continue; // A "reasonable hack". Mirrors contribute extremely little to the illumination of diffuse surfaces
                        if ( !equal(0, thing->interact(interaction) ) )
                            newBeams.push_back( (*part)->bounce(light, interaction) );
                    }
                }
            }
        // Filter out shadows that are completely in the dark anyway
        std::vector< Shadow > newShadows( shadows );
        shadows.clear();
        for ( std::vector< Shadow >::const_iterator shadow = newShadows.begin(); shadow != newShadows.end(); shadow++ )
            if ( !eclipsed(shadow->getSource()) )
                shadows.push_back( *shadow );

        for ( std::vector< Beam >::const_iterator beam = newBeams.begin(); beam != newBeams.end(); beam++ )
            newZones.push_back( new Zone(*beam) );
        return newZones;
    }

    // Contribute to the final image in a Camera
    int Zone::rasterize( Camera* camera ) const
    {
        const int width  = camera->getGridwidth();
        const int height = camera->getGridheight();
        const Vector viewpoint = camera->getViewpoint();

        bool cameraHit = light.contains( viewpoint ) && !camera->behind( light.getApex() );

        if ( cameraHit )
            for ( std::vector< Shadow >::const_iterator shadow = shadows.begin(); shadow != shadows.end(); ++shadow )
            {
                if ( equal( 1, (*shadow).occluded(viewpoint) ) )
                {
                    cameraHit = false;
                    break;
                }
            }

        if ( cameraHit )
        {
            RGB** pixelBuffer = new RGB*[height];
            for ( int row = 0; row < height; ++row )
                pixelBuffer[row] = new RGB[width];
            double** skyBlocked = new double*[height];
            for ( int row = 0; row < height; ++row )
                skyBlocked[row] = new double[width];

            const BoundingBox bb = light.source->getBoundingBox( camera );
            const int rowMin = max( 0, bb.topLeft.row );
            const int rowMax = min( height, bb.bottomRight.row );
            const int colMin = max( 0, bb.topLeft.col );
            const int colMax = min( width, bb.bottomRight.col );
            for ( int row = rowMin; row < rowMax; ++row )
                rasterizeRow( camera, bb, row, pixelBuffer[row], skyBlocked[row] );
            // Write results directly in Camera's pixels array:
            // contributions from all Zones will be superimposed on each other
            for ( int row = 0; row < height; ++row )
                for ( int col = 0; col < width; ++col )
                {
                    if ( RGB::Black != pixelBuffer[row][col] )
                        camera->pixels[row][col] += pixelBuffer[row][col];
                    if ( !equal(0, skyBlocked[row][col]) )
                        camera->skyMask[row][col] -= skyBlocked[row][col];
                }

            for ( int i = 0; i < height; ++i )
                delete[] pixelBuffer[i];
            delete[] pixelBuffer;
            for ( int i = 0; i < height; ++i )
                delete[] skyBlocked[i];
            delete[] skyBlocked;

            return (rowMax - rowMin) * (colMax - colMin);
        }

        return 0;
    }

    bool Zone::hit( const Surface* surface ) const
    {
        // TODO: accurate algorithm for narrow Beams.
        if ( surface->getParent()->isBackCulled() && surface->behind(light.getSource()) )
            return false;
        const std::vector< Vector > points = surface->getPoints( light.getApex() );
        for ( std::vector< Vector >::const_iterator point = points.begin(); point != points.end(); point++ )
            if ( light.contains(*point) )
                return true;
        return false;
    }

    bool Zone::eclipsed( const Surface* surface ) const
    {
        for ( std::vector< Shadow >::const_iterator shadow = shadows.begin(); shadow != shadows.end(); shadow++ )
        {
            if ( surface == shadow->getSource() )
                continue; // No Surface can occlude itself
            if ( dynamic_cast<const Plane*>(surface) )
                continue; // Infinite planes cannot be eclipsed
            if ( !surface->getParent()->isBackground() && shadow->getSource()->getParent()->isBackground() )
                continue; // Backgrounds cannot occlude non-backgrounds
            bool eclipsed = true;
            const std::vector< Vector > points = surface->getPoints( light.getApex() );
            for ( std::vector< Vector >::const_iterator point = points.begin(); point != points.end(); point++ )
                if ( !equal(1, occluded( surface, *point, surface->getParent()->isBackground())) )
                {
                    eclipsed = false;
                    break;
                }
            if ( eclipsed )
                return true;
        }
        return false;
    }

    Triplet Zone::getColor( const Ray& eyeray ) const
    {
        if ( !light.contains(eyeray.getOrigin()) )
            return RGB::Black;
        return light.getColor() * getIntensity( NULL, eyeray );
    }

    // Walk back up the Zone tree to see how much light is radiated in the viewing direction
    double Zone::getIntensity( const Surface* surface, const Ray& eyeray ) const
    {
        const Surface*    source = light.getSource();
        const Tree<Zone>* parent = node->getParent();
        // Check total occlusion before going any further
        const double shadowTerm = ( 1 - occluded(surface, eyeray.getOrigin(), source->getParent()->isBackground()) );
        if ( equal(0, shadowTerm) )
            return 0; // Point is fully in the dark
        // LightPoints are a special case, they normally can't be hit
        const LightPoint* lightpoint = dynamic_cast<const LightPoint*>( source );
        // Need to test if we hit the emitter at all first
        const double sourceT = lightpoint ? (lightpoint->getPoint() - eyeray.getOrigin()).length() : source->intersect( eyeray );
        if ( sourceT < EPSILON )
            return 0; // No hit
        if ( NULL == parent )
            return shadowTerm; // We are in a root Zone
        const Vector sourcePoint = eyeray[ sourceT ];
        const Beam&  parentBeam  = (**parent).light;
        const ThingPart* part = dynamic_cast<const ThingPart*>( source );
        assert( part );
        Vector       nextDirection;
        const Thing* nextMedium = NULL;
        const Material::Interaction kind = light.getKind();
        switch ( kind )
        {
            case Material::DIFFUSE:  nextDirection = parentBeam.getPivot().getOrigin() - sourcePoint; break;
            case Material::METALLIC: nextDirection = eyeray.bounceMetallic( part, sourcePoint ).getDirection(); break;
            case Material::REFLECT:  nextDirection = eyeray.bounceReflect ( part, sourcePoint ).getDirection(); break;
            case Material::REFRACT:  nextDirection = eyeray.bounceRefract ( part, sourcePoint ).getDirection();
                                     if ( !light.getMedium() ) nextMedium = parentBeam.getMedium(); break;
            default: assert( false );
        }
        Ray nextEyeray( scene, sourcePoint, nextDirection, nextMedium );
        const double diffuseTerm = Material::DIFFUSE  == kind ? (*parentBeam.distribution)( parentBeam.pivot, nextEyeray.getOrigin() ) : 1;
        const double    tiltTerm = Material::DIFFUSE  == kind ? part->getTilt( sourcePoint, parentBeam ) : 1; // cos(angle of receiving surface)
        const double fresnelTerm = Material::METALLIC == kind ? light.fresnelIntensity( eyeray ) : 1;
        // Recursion
        const double aggregateIntensity = (**parent).getIntensity( source, nextEyeray ) * shadowTerm *
                                          diffuseTerm * tiltTerm * fresnelTerm;
        return aggregateIntensity;
    }

    double Zone::occluded( const Surface* surface, const Vector& point, bool background ) const
    {
        double occlusion = 0;
        for ( std::vector< Shadow >::const_iterator shadow = shadows.begin(); shadow != shadows.end(); shadow++ )
        {
            if ( surface == shadow->getSource() )
                continue; // Nothing can occlude itself
            if ( !background && shadow->getSource()->getParent()->isBackground() )
                continue; // Backgrounds cannot occlude non-backgrounds
            // This ignores the complications arising from overlapping occluders
            occlusion += (*shadow).occluded( point );
            if ( 1 <= occlusion )
                break;
        }
        return min( 1, occlusion );
    }

    void Zone::rasterizeRow( const Camera* camera, const BoundingBox& bb, int row, RGB* pixelBuffer, double* skyBlocked ) const
    {
        const int    gridwidth    = camera->getGridwidth();
        const Vector viewpoint    = camera->getViewpoint();
        const Vector leftEdge     = camera->getLeftEdge ( row );
        const Vector rowDirection = camera->getRightEdge( row ) - leftEdge;
        const double transparency = light.getSource()->getParent()->getTransparency();
        for ( int col = max(0, bb.topLeft.col); col < min(gridwidth, bb.bottomRight.col); ++col )
        {
            const Vector screenPoint = leftEdge + rowDirection * ( (double)col/gridwidth );
            const Ray eyeray( scene, screenPoint, screenPoint - viewpoint );
            const double sourceT = light.getSource()->intersect( eyeray );
            if ( 0 != sourceT  )
            {
                pixelBuffer[col] = getColor( eyeray ).normalize(); // Squash values into (0, 0, 0)..(1, 1, 1)
                skyBlocked [col] = 1 - transparency;
            }
        }
    }

}

