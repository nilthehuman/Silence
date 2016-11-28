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

// Beam class methods
// Part of Silence, an experimental rendering engine

#include "beam.h"

#include "camera.h"
#include "scene.h"
#include "tree.h"

namespace Silence {

    bool Beam::contains( const Vector& point ) const
    {
        const Vector direction = point - apex;
        const Ray ray( scene, apex, direction );
        const Vector testPoint = ray[ source->intersect(ray) ];
        if ( direction.length() + EPSILON < (testPoint - apex).length() )
            return false;
        if ( edges.size() < 3 )
            return true;
        // en.wikipedia.org/wiki/Point_in_polygon#Ray_casting_algorithm
        // TODO: make this work for spherical surfaces too
        const Vector rayCast = (edges[1].getOrigin() + edges[0].getOrigin()) * 0.5 - testPoint;
        bool inside = false;
        std::vector< Ray >::const_iterator edgeA, edgeB;
        for ( edgeA = edges.begin(), edgeB = edges.begin() + 1;
              edgeA != edges.end();
              ++edgeA, ++edgeB == edges.end() ? edgeB = edges.begin() : edgeB )
        {
            const Vector a = (*edgeA).getOrigin() - testPoint;
            const Vector b = (*edgeB).getOrigin() - testPoint;
            const double dotA = rayCast * a.normalized();
            const double dotB = rayCast * b.normalized();
            if ( dotA + dotB < 0 )
                continue; // the surface edge runs behind the test point
            if ( rayCast.cross(a) * rayCast.cross(b) < 0 )
                inside = !inside;
        }
        return inside;
    }

    Triplet Beam::getColor( const Ray& eyeray ) const
    {
        if ( !contains(eyeray.getOrigin()) )
            return RGB::Black;
        return color * getIntensity( eyeray );
    }

    double Beam::getIntensity( const Ray& eyeray ) const
    {
        // WARNING: experimental code
        // LightPoints are a special case, they normally can't be hit
        const LightPoint* lightpoint = dynamic_cast<const LightPoint*>( source );
        // Need to test if we hit the emitter at all first
        const double sourceT = lightpoint ? (lightpoint->getPoint() - eyeray.getOrigin()).length() : source->intersect( eyeray );
        if ( sourceT < EPSILON )
            return 0;
        const Tree<Zone>* parent = zone->getNode()->getParent();
        if ( NULL == parent )
            return 1; // We are in a root Zone
        const Vector sourcePoint = eyeray[ sourceT ];
        const Beam&  parentBeam  = (**parent).getLight();
        const ThingPart* part = dynamic_cast<const ThingPart*>( source );
        assert( part );
        Vector nextDirection;
        switch ( kind )
        {
            case Material::DIFFUSE:  nextDirection = parentBeam.getPivot().getOrigin() - sourcePoint; break;
            case Material::METALLIC: nextDirection = eyeray.bounceMetallic( part, sourcePoint ).getDirection(); break;
            case Material::REFLECT:  nextDirection = eyeray.bounceReflect ( part, sourcePoint ).getDirection(); break;
            case Material::REFRACT:  nextDirection = eyeray.bounceRefract ( part, sourcePoint ).getDirection(); break;
            default: assert( false );
        }
        Ray nextEyeray( scene, sourcePoint, nextDirection );
        const double diffuseTerm = Material::DIFFUSE  == kind ? (*parentBeam.distribution)( parentBeam.pivot, nextEyeray.getOrigin() ) : 1;
        const double    tiltTerm = Material::DIFFUSE  == kind ? part->getTilt( sourcePoint, parentBeam ) : 1; // cos(angle of receiving surface)
        const double fresnelTerm = Material::METALLIC == kind ? fresnelIntensity( eyeray ) : 1;
        // Recursion
        const double aggregateIntensity = parentBeam.getIntensity( nextEyeray ) * diffuseTerm * tiltTerm * fresnelTerm;
        return aggregateIntensity;
    }

    void Beam::rasterizeRow( const Camera* camera, const BoundingBox& bb, int row, RGB* pixelBuffer, double* skyBlocked ) const
    {
        const int    gridwidth    = camera->getGridwidth();
        const Vector viewpoint    = camera->getViewpoint();
        const Vector leftEdge     = camera->getLeftEdge ( row );
        const Vector rowDirection = camera->getRightEdge( row ) - leftEdge;
        const double transparency = source->getParent()->getTransparency();
        for ( int col = max(0, bb.topLeft.col); col < min(gridwidth, bb.bottomRight.col); ++col )
        {
            const Vector screenPoint = leftEdge + rowDirection * ( (double)col/gridwidth );
            const Ray eyeray( scene, screenPoint, screenPoint - viewpoint );
            // -------- * -------- * -------- * -------- * -------- * -------- * -------- * --------
            //  WARNING: Temporary brute force rendering! The following loops will be replaced with
            //              a much more efficient algorithm when we have Shadows ready.
            // -------- * -------- * -------- * -------- * -------- * -------- * -------- * --------
            const double sourceT = source->intersect( eyeray );
            double nearestT = INF;
            for ( ThingIt thing = scene->thingsBegin(); thing != scene->thingsEnd(); thing++ )
                if ( (*thing)->isBackground() == false )
                {
                    for ( ThingPartIt part = (*thing)->partsBegin(); part != (*thing)->partsEnd(); part++ )
                    {
                        double t = (*part)->intersect(eyeray);
                        if ( 0 < t )
                            nearestT = min(nearestT, t);
                    }
                    if ( nearestT < sourceT )
                        break;
                }
            for ( LightIt light = scene->lightsBegin(); light != scene->lightsEnd(); light++ )
                if ( (*light)->isBackground() == false )
                {
                    for ( LightPartIt part = (*light)->partsBegin(); part != (*light)->partsEnd(); part++ )
                    {
                        double t = (*part)->intersect(eyeray);
                        if ( 0 < t )
                            nearestT = min(nearestT, t);
                    }
                    if ( nearestT < sourceT )
                        break;
                }
            if ( sourceT < nearestT )
            {
                for ( ThingIt thing = scene->thingsBegin(); thing != scene->thingsEnd(); thing++ )
                    if ( (*thing)->isBackground() == true )
                    {
                        for ( ThingPartIt part = (*thing)->partsBegin(); part != (*thing)->partsEnd(); part++ )
                        {
                            double t = (*part)->intersect(eyeray);
                            if ( 0 < t )
                                nearestT = min(nearestT, t);
                        }
                        if ( nearestT < sourceT )
                            break;
                    }
            }
            // -------- * -------- * -------- * -------- * -------- * -------- * -------- * --------
            //                      END of brute force intersection detection.
            // -------- * -------- * -------- * -------- * -------- * -------- * -------- * --------
            if ( 0 != sourceT && sourceT == nearestT )
            {
                pixelBuffer[col] = getColor( eyeray ).normalize(); // Squash values into (0, 0, 0)..(1, 1, 1)
                skyBlocked [col] = 1 - transparency;
            }
        }
    }

    void Beam::occlude()
    {
        // TODO
    }

    double Beam::fresnelIntensity( const Ray& eyeray, const Vector& point ) const
    {
        const Thing* medium = zone->getMedium();
        double n1, n2;
        if ( medium )
        {
            n1 = medium->getRefractiveIndex();
            n2 = 1.0;
        }
        else
        {
            n1 = 1.0;
            n2 = static_cast<const Thing*>(source->getParent())->getRefractiveIndex();
        }
        Vector hitPoint;
        if ( Vector::Invalid != point )
            hitPoint = point;
        else
        {
            const double t = source->intersect( eyeray );
            if ( t < EPSILON )
                return 0;
            hitPoint = eyeray[t];
        }
        const Vector surfaceNormal = source->getNormal( hitPoint );
        const Vector direction     = eyeray.getDirection() - surfaceNormal * (eyeray.getDirection() * surfaceNormal) * 2;
        const double cosTheta = direction * surfaceNormal;
        return schlick( n1, n2, cosTheta );
    }

    double Beam::schlick( double n1, double n2, double cosTheta )
    {
        // http://en.wikipedia.org/wiki/Schlick%27s_approximation
        const double R0 = (n1 - n2) * (n1 - n2) / ( (n1 + n2) * (n1 + n2) );
        return R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5);
    }

}

