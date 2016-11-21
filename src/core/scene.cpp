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
            if ( parent->isBackCulled() && EPSILON < nominator )
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

    void LightPoint::emitZones( std::vector< Tree<Zone>* >& out ) const
    {
        const Scene* scene = parent->getScene();
        Zone zone( Beam(scene, point, (Surface*)this, Ray(scene, point, Vector::Zero), std::vector<Ray>(), ((Light*)parent)->getEmission(), &Beam::Spherical) );
        out.push_back( new Tree<Zone>(zone) );
    }

    double Sphere::getTilt( const Vector& ) const
    {
        return 1;
    }

    Vector Sphere::mirror( const Vector& point ) const
    {
        return point;
    }

    std::vector< Vector > Sphere::getPoints( const Vector& viewpoint ) const
    {
        const Vector normal = (center - viewpoint).normalize();
        std::vector< Vector > points;
        if ( Vector::UnitX == normal || -Vector::UnitX == normal )
            points.push_back( center + normal.cross(Vector::UnitY).normalize() );
        else
            points.push_back( center + normal.cross(Vector::UnitX).normalize() );
        points.push_back( center + normal.cross(points[0] - center) );
        points.push_back( center + normal.cross(points[1] - center) );
        points.push_back( center + normal.cross(points[2] - center) );
        return points;
    }

    void LightSphere::emitZones( std::vector< Tree<Zone>* >& out ) const
    {
        const Scene* scene = parent->getScene();
        Zone zone( Beam(scene, center, (Surface*)this, Ray(scene, center, Vector::Zero), std::vector<Ray>(), ((Light*)parent)->getEmission(), &Beam::Spherical) );
        out.push_back( new Tree<Zone>(zone) );
    }

    double Plane::getTilt( const Vector& pivot ) const
    {
        if ( Vector::Zero == pivot )
            return 1;
        return abs( normal * pivot );
    }

    Vector Plane::mirror( const Vector& point ) const
    {
        const double distance = point * normal - offset;
        return point - normal * 2 * distance;
    }

    std::vector< Vector > Plane::getPoints( const Vector& viewpoint ) const
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

    void LightPlane::emitZones( std::vector< Tree<Zone>* >& out ) const
    {
        const Scene* scene = parent->getScene();
        Zone up( Beam( scene, normal*offset, (Surface*)this, Ray(scene, normal*offset, normal), std::vector<Ray>(), ((Light*)parent)->getEmission(), &Beam::Uniform ) );
        out.push_back( new Tree<Zone>(up) );
        if ( !parent->isBackCulled() )
        {
            Zone down( Beam( scene, normal*offset, (Surface*)this, Ray(scene, normal*offset, -normal), std::vector<Ray>(), ((Light*)parent)->getEmission(), &Beam::Uniform ) );
            out.push_back( new Tree<Zone>(down) );
        }
    }

    double Triangle::getTilt( const Vector& pivot ) const
    {
        if ( Vector::Zero == pivot )
            return 1;
        const Vector& normal = ( points[1] - points[0] ).cross( points[2] - points[0] );
        return abs( normal * pivot );
    }

    Vector Triangle::mirror( const Vector& point ) const
    {
        const Vector& normal = ( points[1] - points[0] ).cross( points[2] - points[0] );
        const double offset = normal * points[0];
        const double distance = point * normal - offset;
        return point - normal * 2 * distance;
    }

    std::vector< Vector > Triangle::getPoints( const Vector& ) const
    {
        std::vector< Vector > pointsVector;
        pointsVector.push_back( points[0] );
        pointsVector.push_back( points[1] );
        pointsVector.push_back( points[2] );
        return pointsVector;
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
        Zone up( Beam(scene, apex, (Surface*)this, Ray(scene, apex, normal), edges, ((Light*)parent)->getEmission(), &Beam::Spherical) );
        out.push_back( new Tree<Zone>(up) );
        if ( !parent->isBackCulled() )
        {
            std::vector< Ray > edges;
            edges.push_back( Ray(scene, points[0], points[0]-apex) );
            edges.push_back( Ray(scene, points[1], points[1]-apex) );
            edges.push_back( Ray(scene, points[2], points[2]-apex) );
            Zone down( Beam(scene, apex, (Surface*)this, Ray(scene, apex, normal), edges, ((Light*)parent)->getEmission(), &Beam::Spherical) );
            out.push_back( new Tree<Zone>(down) );
        }
    }

}

