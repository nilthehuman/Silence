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

// Scene class methods
// Part of Silence, an experimental rendering engine

#include "scene.h"

#include <math.h>
#include <stdlib.h>
#include <cassert>

#include "beam.h"
#include "ray.h"
#include "tree.h"
#include "zone.h"

namespace Silence {

    void Surface::rotate( Vector& point, double theta, WorldAxis axis )
    {
        double c = cos( theta );
        double s = sin( theta );
        Vector newPoint;
        switch( axis )
        {
            case AXIS_X:
                newPoint.x = point.x;
                newPoint.y = c * point.y - s * point.z;
                newPoint.z = s * point.y + c * point.z;
                break;
            case AXIS_Y:
                newPoint.x =  c * point.x + s * point.z;
                newPoint.y = point.y;
                newPoint.z = -s * point.x + c * point.z;
                break;
            case AXIS_Z:
                newPoint.x = c * point.x - s * point.y;
                newPoint.y = s * point.x + c * point.y;
                newPoint.z = point.z;
                break;
            default:
                assert( false );
        }
        point = newPoint;
    }

    void Light::emitZones( std::vector< Tree<Zone>* >& out ) const
    {
        if ( RGB::Black == emission )
            return;
        for ( LightPartIt part = partsBegin(); part != partsEnd(); part++ )
            (*part)->emitZones( out );
    }
    void Light::move( const Vector& translation ) const
    {
        for ( LightPartIt part = partsBegin(); part != partsEnd(); part++ )
            (*part)->move( translation );
        scene->setChanged();
    }
    void Light::move( double theta, WorldAxis axis ) const
    {
        for ( LightPartIt part = partsBegin(); part != partsEnd(); part++ )
            (*part)->move( theta, axis );
        scene->setChanged();
    }

    void Thing::move( const Vector& translation ) const
    {
        for ( ThingPartIt part = partsBegin(); part != partsEnd(); part++ )
            (*part)->move( translation );
        scene->setChanged();
    }
    void Thing::move( double theta, WorldAxis axis ) const
    {
        for ( ThingPartIt part = partsBegin(); part != partsEnd(); part++ )
            (*part)->move( theta, axis );
        scene->setChanged();
    }

    double ISphere::intersect( const Ray& ray ) const
    {
        // Algorithm cribbed from smallpt
        // www.kevinbeason.com/smallpt/
        const Vector toCenter = center - ray.getOrigin();
        const double b = toCenter * ray.getDirection();
        const double discriminant = b * b - toCenter * toCenter + radius * radius;
        if ( discriminant < 0 )
            return 0;
        double t;
        const double sqrtDiscriminant = sqrt( discriminant );
        if ( (t = b - sqrtDiscriminant) > EPSILON )
            return t;
        if ( (t = b + sqrtDiscriminant) > EPSILON && !parent->isBackCulled() )
            return t;
        return 0;
    }

    double IPlane::intersect( const Ray& ray ) const
    {
        // Common ray/plane intersection algorithm, see Ogre for example
        // github.com/ehsan/ogre/blob/master/OgreMain/src/OgreMath.cpp
        double denominator = normal * ray.getDirection();
        if ( equal(denominator, 0) )
            return 0;
        else {
            double nominator = offset - normal * ray.getOrigin();
            if ( (NULL == parent || parent->isBackCulled()) && EPSILON < nominator )
                return 0;
            double t = nominator / denominator;
            if ( EPSILON < t )
                return t;
            else
                return 0;
        }
    }

    double ITriangle::intersect( const Ray& ray ) const
    {
        // Möller-Trumbore algorithm
        // en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        // www.scratchapixel.com/old/lessons/3d-basic-lessons/lesson-9-ray-triangle-intersection/m-ller-trumbore-algorithm/
        const Vector edge1 = points[1] - points[0];
        const Vector edge2 = points[2] - points[0];
        const Vector P = ray.getDirection().cross( edge2 );
        const double determinant = edge1 * P;
        if ( parent->isBackCulled() )
        {
            if ( determinant < EPSILON )
                return 0;
        }
        else
        {
            if ( equal(determinant, 0) )
                return 0;
        }
        const Vector T = ray.getOrigin() - points[0];
        double u = T * P / determinant;
        if ( u < 0 || 1 < u )
            return 0;
        const Vector Q = T.cross( edge1 );
        double v = ray.getDirection() * Q / determinant;
        if ( v < 0 || 1 < u + v )
            return 0;
        double t = edge2 * Q / determinant;
        if ( t < EPSILON )
            return 0;
        else
            return t;
    }

    const BoundingBox IPoint::getBoundingBox( const Camera* camera ) const
    {
        ScreenPoint screenpoint = camera->project( point );
        return BoundingBox( screenpoint, screenpoint );
    }

    std::vector< Vector > IPoint::getPoints( const Vector& ) const
    {
        std::vector< Vector > points;
        points.push_back( point );
        return points;
    }

    void LightPoint::emitZones( std::vector< Tree<Zone>* >& out ) const
    {
        const Scene* scene = parent->getScene();
        Zone* zone = new Zone( Beam(scene, point, (Surface*)this, Ray(scene, point, Vector::Zero), std::vector<Ray>(), ((Light*)parent)->getEmission(), &Beam::Spherical) );
        out.push_back( new Tree<Zone>(zone) );
    }

    bool ISphere::behind( const Surface* source ) const
    {
        // C++ lacks multi-dispatch. We make do with dynamic_cast instead
        if      ( const IPoint*    point    = dynamic_cast<const IPoint*   >(source) )
            return (point->getPoint() - center).length() < radius + EPSILON;
        else if ( const ISphere*   sphere   = dynamic_cast<const ISphere*  >(source) )
            return (sphere->center - center).length() + sphere->radius < radius + EPSILON;
        else if (                             dynamic_cast<const IPlane*   >(source) )
            return false;
        else if ( const ITriangle* triangle = dynamic_cast<const ITriangle*>(source) )
        {
            const std::vector< Vector > points = triangle->getPoints( Vector::Zero );
            for ( int i = 0; i < 3; ++i )
                if ( radius + EPSILON < (points[i] - center).length() )
                    return false;
            return true;
        }
        else
            assert( false );
    }

    const BoundingBox ISphere::getBoundingBox( const Camera* camera ) const
    {
        const Vector normal = (center - camera->getViewpoint()).normalize();
        Vector points[4];
        if ( camera->getScreenX() == normal || -camera->getScreenX() == normal )
            points[0] = center + normal.cross(camera->getScreenY()).normalize() * radius * 1.05; // TODO: Resolve this hack.
        else
            points[0] = center + normal.cross(camera->getScreenX()).normalize() * radius * 1.05;
        points[1] = center + normal.cross(points[0] - center).normalize() * radius * 1.05;
        points[2] = center + normal.cross(points[1] - center).normalize() * radius * 1.05;
        points[3] = center + normal.cross(points[2] - center).normalize() * radius * 1.05;
        ScreenPoint topLeft    ( camera->project(points[1]).col - 1, camera->project(points[2]).row - 1);
        ScreenPoint bottomRight( camera->project(points[3]).col + 1, camera->project(points[0]).row + 1);
        return BoundingBox( topLeft, bottomRight );
    }

    std::vector< Vector > ISphere::getPoints( const Vector& viewpoint ) const
    {
        const Vector normal = (center - viewpoint).normalize();
        std::vector< Vector > points;
        if ( Vector::UnitX == normal || -Vector::UnitX == normal )
            points.push_back( center + normal.cross(Vector::UnitY).normalize() * radius );
        else
            points.push_back( center + normal.cross(Vector::UnitX).normalize() * radius );
        points.push_back( center + normal.cross(points[0] - center).normalize() * radius );
        points.push_back( center + normal.cross(points[1] - center).normalize() * radius );
        points.push_back( center + normal.cross(points[2] - center).normalize() * radius );
        return points;
    }

    double Sphere::getTilt( const Vector& ) const
    {
        return 1;
    }

    Vector Sphere::mirror( const Vector& point ) const
    {
        // TODO: Right this wrong.
        return point;
    }

    Beam Sphere::bounce( const Beam& beam, const Material::Interaction& interaction ) const
    {
        const Ray adjustedPivot( beam.getScene(), beam.getPivot().getOrigin(), center - beam.getPivot().getOrigin() );
        const Vector hitPoint = adjustedPivot[ intersect(adjustedPivot) ];
        const Thing* thing = static_cast<const Thing*>( parent );

        Vector             newApex  = Vector::Invalid;
        const Ray*         newPivot = NULL;
        std::vector< Ray > newEdges;
        const Triplet      newColor = beam.getColor() * thing->getColor() * thing->interact( interaction );
        Beam::Distribution newDistribution;
        switch ( interaction )
        {
            case Material::DIFFUSE:
                newApex  = center;
                newPivot = new Ray( beam.getScene(), hitPoint, getNormal(hitPoint) );
                newDistribution = Beam::Spherical;
                break;
            case Material::METALLIC:
                newApex  = center;
                newPivot = new Ray( adjustedPivot.bounceMetallic(this, hitPoint) );
                for ( std::vector< Ray >::const_iterator e = beam.getEdges().begin(); e != beam.getEdges().end(); e++ )
                    newEdges.push_back( e->bounceMetallic(this) );
                newDistribution = beam.getDistribution();
                // Loss of intensity will be taken into account during rasterization,
                // see Beam::fresnelIntensity
                break;
            case Material::REFLECT:
                newApex  = center;
                newPivot = new Ray( adjustedPivot.bounceReflect(this, hitPoint) );
                for ( std::vector< Ray >::const_iterator e = beam.getEdges().begin(); e != beam.getEdges().end(); e++ )
                    newEdges.push_back( e->bounceReflect(this) );
                newDistribution = beam.getDistribution();
                break;
            case Material::REFRACT:
                newApex  = beam.getApex();
                newPivot = new Ray( adjustedPivot.bounceRefract(this, hitPoint) );
                for ( std::vector< Ray >::const_iterator e = beam.getEdges().begin(); e != beam.getEdges().end(); e++ )
                    newEdges.push_back( e->bounceRefract(this) );
                newDistribution = beam.getDistribution();
                break;
            default:
                assert( false );
        }
        Beam newBeam( beam.getScene(),
                      newApex, this, *newPivot, newEdges,
                      newColor, newDistribution, interaction );
        return newBeam;
    }

    void LightSphere::emitZones( std::vector< Tree<Zone>* >& out ) const
    {
        const Scene* scene = parent->getScene();
        Zone* zone = new Zone( Beam(scene, center, (Surface*)this, Ray(scene, center, Vector::Zero), std::vector<Ray>(), ((Light*)parent)->getEmission(), &Beam::Spherical) );
        out.push_back( new Tree<Zone>(zone) );
    }

    bool IPlane::behind( const Surface* source ) const
    {
        // C++ lacks multi-dispatch. We make do with dynamic_cast instead
        if      ( const IPoint*    point    = dynamic_cast<const IPoint*   >(source) )
            return normal * point->getPoint() < offset + EPSILON;
        else if ( const ISphere*   sphere   = dynamic_cast<const ISphere*  >(source) )
            return normal * sphere->getCenter() + sphere->getRadius() < offset + EPSILON;
        else if ( const IPlane*    plane    = dynamic_cast<const IPlane*   >(source) )
        {
            if      ( normal ==  plane->normal )
                return  plane->offset < offset + EPSILON;
            else if ( normal == -plane->normal )
                return -plane->offset < offset + EPSILON;
            else
                return false;
        }
        else if ( const ITriangle* triangle = dynamic_cast<const ITriangle*>(source) )
        {
            const std::vector< Vector > points = triangle->getPoints( Vector::Zero );
            for ( int i = 0; i < 3; ++i )
                if ( offset + EPSILON < normal * points[i] )
                    return false;
            return true;
        }
        else
            assert( false );
    }

    const BoundingBox IPlane::getBoundingBox( const Camera* camera ) const
    {
        const ScreenPoint topLeft( 0, 0 );
        const ScreenPoint bottomRight( camera->getGridwidth(), camera->getGridheight() );
        return BoundingBox( topLeft, bottomRight );
    }

    std::vector< Vector > IPlane::getPoints( const Vector& viewpoint ) const
    {
        const Vector viewnormal = (normal * offset - viewpoint).normalize();
        std::vector< Vector > points;
        if ( Vector::UnitX == viewnormal || -Vector::UnitX == viewnormal )
            points.push_back( normal * offset + viewnormal.cross(Vector::UnitY).normalize() );
        else
            points.push_back( normal * offset + viewnormal.cross(Vector::UnitX).normalize() );
        points.push_back( normal * offset + viewnormal.cross(points[0] - normal * offset) );
        points.push_back( normal * offset + viewnormal.cross(points[1] - normal * offset) );
        points.push_back( normal * offset + viewnormal.cross(points[2] - normal * offset) );
        for ( int i = 0; i < 4; ++i )
            points[i] *= INF;
        return points;
    }

    double Plane::getTilt( const Vector& direction ) const
    {
        return abs( normal * direction.normalized() );
    }

    Vector Plane::mirror( const Vector& point ) const
    {
        const double distance = point * normal - offset;
        return point - normal * 2 * distance;
    }

    Beam Plane::bounce( const Beam& beam, const Material::Interaction& interaction ) const
    {
        const Ray adjustedPivot( beam.getScene(), adjustedPivot.getDirection(), -normal );
        const Vector hitPoint = adjustedPivot[ intersect(adjustedPivot) ];
        const Thing* thing = static_cast<const Thing*>( parent );

        Vector             newApex  = Vector::Invalid;
        const Ray*         newPivot = NULL;
        std::vector< Ray > newEdges;
        const Triplet      newColor = beam.getColor() * thing->getColor() * thing->interact( interaction );
        Beam::Distribution newDistribution;
        switch ( interaction )
        {
            case Material::DIFFUSE:
                newApex  = mirror( beam.getApex() );
                newPivot = new Ray( beam.getScene(), hitPoint, getNormal(hitPoint) );
                newDistribution = Beam::Uniform;
                break;
            case Material::METALLIC:
                newApex  = mirror( beam.getApex() );
                newPivot = new Ray( adjustedPivot.bounceMetallic(this, hitPoint) );
                for ( std::vector< Ray >::const_iterator e = beam.getEdges().begin(); e != beam.getEdges().end(); e++ )
                    newEdges.push_back( e->bounceMetallic(this) );
                newDistribution = beam.getDistribution();
                break;
            case Material::REFLECT:
                newApex  = mirror( beam.getApex() );
                newPivot = new Ray( adjustedPivot.bounceReflect(this, hitPoint) );
                for ( std::vector< Ray >::const_iterator e = beam.getEdges().begin(); e != beam.getEdges().end(); e++ )
                    newEdges.push_back( e->bounceReflect(this) );
                newDistribution = beam.getDistribution();
                break;
            case Material::REFRACT:
                newApex  = beam.getApex();
                newPivot = new Ray( adjustedPivot.bounceRefract(this, hitPoint) );
                for ( std::vector< Ray >::const_iterator e = beam.getEdges().begin(); e != beam.getEdges().end(); e++ )
                    newEdges.push_back( e->bounceRefract(this) );
                newDistribution = beam.getDistribution();
                break;
            default:
                assert( false );
        }
        Beam newBeam( beam.getScene(),
                      newApex, this, *newPivot, newEdges,
                      newColor, newDistribution, interaction );
        return newBeam;
    }

    void LightPlane::emitZones( std::vector< Tree<Zone>* >& out ) const
    {
        const Scene* scene = parent->getScene();
        Zone* up = new Zone( Beam( scene, normal*offset, (Surface*)this, Ray(scene, normal*offset, normal), std::vector<Ray>(), ((Light*)parent)->getEmission(), &Beam::Uniform ) );
        out.push_back( new Tree<Zone>(up) );
        if ( !parent->isBackCulled() )
        {
            Zone* down = new Zone( Beam( scene, normal*offset, (Surface*)this, Ray(scene, normal*offset, -normal), std::vector<Ray>(), ((Light*)parent)->getEmission(), &Beam::Uniform ) );
            out.push_back( new Tree<Zone>(down) );
        }
    }

    bool ITriangle::behind( const Surface* source ) const
    {
        const Vector normal = ( points[1] - points[0] ).cross( points[2] - points[0] ).normalize();
        const double offset = normal * points[0];
        // C++ lacks multi-dispatch. We make do with dynamic_cast instead
        if      ( const IPoint*    point    = dynamic_cast<const IPoint*   >(source) )
            return normal * point->getPoint() < offset + EPSILON;
        else if ( const ISphere*   sphere   = dynamic_cast<const ISphere*  >(source) )
            return normal * sphere->getCenter() + sphere->getRadius() < offset + EPSILON;
        else if ( const IPlane*    plane    = dynamic_cast<const IPlane*   >(source) )
        {
            if      ( normal ==  plane->getNormal() )
                return  plane->getOffset() < offset + EPSILON;
            else if ( normal == -plane->getNormal() )
                return -plane->getOffset() < offset + EPSILON;
            else
                return false;
        }
        else if ( const ITriangle* triangle = dynamic_cast<const ITriangle*>(source) )
        {
            const std::vector< Vector > points = triangle->getPoints( Vector::Zero );
            for ( int i = 0; i < 3; ++i )
                if ( offset + EPSILON < normal * points[i] )
                    return false;
            return true;
        }
        else
            assert( false );
    }

    const BoundingBox ITriangle::getBoundingBox( const Camera* camera ) const
    {
        const ScreenPoint screenpoint0 = camera->project( points[0] );
        const ScreenPoint screenpoint1 = camera->project( points[1] );
        const ScreenPoint screenpoint2 = camera->project( points[2] );
        const int minCol = min( min(screenpoint0.col, screenpoint1.col), screenpoint2.col ) - 1;
        const int minRow = min( min(screenpoint0.row, screenpoint1.row), screenpoint2.row ) - 1;
        const int maxCol = max( max(screenpoint0.col, screenpoint1.col), screenpoint2.col ) + 1;
        const int maxRow = max( max(screenpoint0.row, screenpoint1.row), screenpoint2.row ) + 1;
        return BoundingBox( ScreenPoint(minCol, minRow), ScreenPoint(maxCol, maxRow) );
    }

    std::vector< Vector > ITriangle::getPoints( const Vector& ) const
    {
        std::vector< Vector > pointsVector;
        pointsVector.push_back( points[0] );
        pointsVector.push_back( points[1] );
        pointsVector.push_back( points[2] );
        return pointsVector;
    }

    double Triangle::getTilt( const Vector& direction ) const
    {
        const Vector normal = ( points[1] - points[0] ).cross( points[2] - points[0] ).normalize();
        return abs( normal * direction.normalized() );
    }

    Vector Triangle::mirror( const Vector& point ) const
    {
        const Vector normal = ( points[1] - points[0] ).cross( points[2] - points[0] ).normalize();
        const double offset = normal * points[0];
        const double distance = point * normal - offset;
        return point - normal * 2 * distance;
    }

    Beam Triangle::bounce( const Beam& beam, const Material::Interaction& interaction ) const
    {
        const Ray adjustedPivot( beam.getScene(), beam.getPivot().getOrigin(), (points[0] + points[1] + points[2]) * 0.333 - beam.getPivot().getOrigin() );
        const Vector hitPoint = adjustedPivot[ intersect(adjustedPivot) ];
        const Thing* thing = static_cast<const Thing*>( parent );

        Vector             newApex  = Vector::Invalid;
        const Ray*         newPivot = NULL;
        std::vector< Ray > newEdges;
        const Triplet      newColor = beam.getColor() * thing->getColor() * thing->interact( interaction );
        Beam::Distribution newDistribution;
        switch ( interaction )
        {
            case Material::DIFFUSE:
                newApex  = mirror( beam.getApex() );
                newPivot = new Ray( beam.getScene(), hitPoint, getNormal(hitPoint) );
                newDistribution = Beam::Planar;
                break;
            case Material::METALLIC:
                newApex  = mirror( beam.getApex() );
                newPivot = new Ray( adjustedPivot.bounceMetallic(this, hitPoint) );
                for ( std::vector< Ray >::const_iterator e = beam.getEdges().begin(); e != beam.getEdges().end(); e++ )
                    newEdges.push_back( e->bounceMetallic(this) );
                newDistribution = beam.getDistribution();
                break;
            case Material::REFLECT:
                newApex  = mirror( beam.getApex() );
                newPivot = new Ray( adjustedPivot.bounceReflect(this, hitPoint) );
                for ( std::vector< Ray >::const_iterator e = beam.getEdges().begin(); e != beam.getEdges().end(); e++ )
                    newEdges.push_back( e->bounceReflect(this) );
                newDistribution = beam.getDistribution();
                break;
            case Material::REFRACT:
                newApex  = beam.getApex();
                newPivot = new Ray( adjustedPivot.bounceRefract(this, hitPoint) );
                for ( std::vector< Ray >::const_iterator e = beam.getEdges().begin(); e != beam.getEdges().end(); e++ )
                    newEdges.push_back( e->bounceRefract(this) );
                newDistribution = beam.getDistribution();
                break;
            default:
                assert( false );
        }
        Beam newBeam( beam.getScene(),
                      newApex, this, *newPivot, newEdges,
                      newColor, newDistribution, interaction );
        return newBeam;
    }
    void LightTriangle::emitZones( std::vector< Tree<Zone>* >& out ) const
    {
        const Scene* scene = parent->getScene();
        const Vector apex = (points[0] + points[1] + points[2]) * 0.333;
        const Vector edge0 = points[1] - points[0];
        const Vector edge1 = points[2] - points[0];
        const Vector normal = edge0.cross( edge1 ).normalize();
        std::vector< Ray > edges;
        edges.push_back( Ray(scene, points[0], points[0]-apex) );
        edges.push_back( Ray(scene, points[1], points[1]-apex) );
        edges.push_back( Ray(scene, points[2], points[2]-apex) );
        Zone* up = new Zone( Beam(scene, apex, (Surface*)this, Ray(scene, apex, normal), edges, ((Light*)parent)->getEmission(), &Beam::Planar) );
        out.push_back( new Tree<Zone>(up) );
        if ( !parent->isBackCulled() )
        {
            std::vector< Ray > edges;
            edges.push_back( Ray(scene, points[0], points[0]-apex) );
            edges.push_back( Ray(scene, points[1], points[1]-apex) );
            edges.push_back( Ray(scene, points[2], points[2]-apex) );
            Zone* down = new Zone( Beam(scene, apex, (Surface*)this, Ray(scene, apex, normal), edges, ((Light*)parent)->getEmission(), &Beam::Planar) );
            out.push_back( new Tree<Zone>(down) );
        }
    }

}

