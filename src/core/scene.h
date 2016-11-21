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

// Class hierarchy for representing a Scene and all parts of it
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_SCENE
#define SILENCE_SCENE

#include <vector>

#include "aux.h"
#include "material.h"
#include "triplet.h"

namespace Silence {

    enum WorldAxis { AXIS_X, AXIS_Y, AXIS_Z, INVALID };

    class Ray;
    class Scene;
    template< class T > class Tree;
    class Zone;

    class Thing;
    class ThingPart;
    class Light;
    class LightPart;
    typedef std::vector< Thing* >    ::const_iterator ThingIt;
    typedef std::vector< ThingPart* >::const_iterator ThingPartIt;
    typedef std::vector< Light* >    ::const_iterator LightIt;
    typedef std::vector< LightPart* >::const_iterator LightPartIt;

    class Point;
    class Sphere;
    class Plane;
    class Triangle;

    class LightPoint;
    class LightSphere;
    class LightPlane;
    class LightTriangle;

    // A set of Surfaces that delimit the same physical object
    class Object {
    public:
        bool isBackground() const { return background; }
        bool isBackCulled() const { return backCulled; }

        const Scene* getScene() const { return scene; }

        virtual double getTransparency() const = 0; // How much you can see through the Object

        virtual void move( const Vector& translation )    const = 0; // Move each part
        virtual void move( double theta, WorldAxis axis ) const = 0; // Rotate each part

    protected:
        Object( const Scene* scene ) : scene( scene ) { }
        virtual ~Object() { }

        friend std::istream& operator>>( std::istream&, Scene& );

        const Scene* const scene;
        bool background; // A background is a Surface that may only occlude other backgrounds from any direction
        bool backCulled; // Back-face culling makes the negative side of Surfaces invisible
    };

    // Generic surface primitive class. This is what every lightsource and thing in the Scene needs to be able to do
    class Surface {
    public:
        virtual double intersect( const Ray& ray )      const = 0; // Returns distance from Ray origin; a return value of zero will mean a miss
        virtual Vector getNormal( const Vector& point ) const = 0; // Returns the outward pointing surface normal
        virtual void   move( const Vector& translation )      = 0; // Translate Surface by an arbitrary world space vector
        virtual void   move( double theta, WorldAxis axis )   = 0; // Rotate Surface around one of the world coordinate axes

    protected:
        Surface( const Object* parent ) : parent( parent ) { }
        virtual ~Surface() { }

        static void rotate( Vector& point, double theta, WorldAxis axis ); // Helper function to rotate a point around a world axis

    protected:
        const Object* parent;
    };

    class ThingPart : virtual public Surface {
    public:
        ThingPart( const Thing* parent ) : Surface( (Object*)parent ) { }
        virtual ~ThingPart() { }

        // Return the dot product of the normal and a given pivot (for Phong reflection)
        virtual double getTilt( const Vector& pivot ) const = 0;
        // Return the reflection of a given point off the plane of the shape
        virtual Vector mirror ( const Vector& point ) const = 0;
        // Return the "outline" of the shape from a given direction
        virtual std::vector< Vector > getPoints( const Vector& viewpoint ) const = 0;

        const Thing* getParent() const { return (Thing*)parent; }
    };

    class LightPart : virtual public Surface {
    public:
        LightPart( const Light* parent ) : Surface( (Object*)parent ) { }
        virtual ~LightPart() { }

        virtual void emitZones( std::vector< Tree<Zone>* >& out ) const = 0;

        const Light* getParent() const { return (Light*)parent; }
    };

    // Non-light complex objects in the Scene
    class Thing : public Object {
    public:
        Thing( const Scene* scene ) : Object( scene ) { }
        ~Thing()
        {
            for ( ThingPartIt part = partsBegin(); part != partsEnd(); part++ )
                delete *part;
        }

        void push_back( ThingPart* part ) { parts.push_back( part ); }

        ThingPartIt partsBegin() const { return parts.begin(); }
        ThingPartIt partsEnd()   const { return parts.end();   }

        const RGB& getColor()            const { return material.getColor(); }
        virtual double getTransparency()    const { return interact(Material::REFRACT); }
        double         getRefractiveIndex() const { return material.getRefractiveIndex(); }

        double     interact( const Material::Interaction& interaction ) const { return material.interact(interaction); }

        virtual void move( const Vector& translation ) const;
        virtual void move( double theta, WorldAxis axis ) const;

        friend std::istream& operator>>( std::istream&, Sphere& );
        friend std::istream& operator>>( std::istream&, Plane& );
        friend std::istream& operator>>( std::istream&, Triangle& );
        friend std::istream& operator>>( std::istream&, Scene& );

    private:
        std::vector< ThingPart* > parts;

        Material material;
    };

    // Complex light objects
    class Light : public Object {
    public:
        Light( const Scene* scene ) : Object( scene ) { }
        ~Light()
        {
            for ( LightPartIt part = partsBegin(); part != partsEnd(); part++ )
                delete *part;
        }

        void push_back( LightPart* part ) { parts.push_back( part ); }

        LightPartIt    partsBegin()      const { return parts.begin(); }
        LightPartIt    partsEnd()        const { return parts.end();   }
        virtual double getTransparency() const { return 0;             }
        const Triplet& getEmission()     const { return emission;      }

        void emitZones( std::vector< Tree<Zone>* >& out ) const;

        virtual void move( const Vector& translation ) const;
        virtual void move( double theta, WorldAxis axis ) const;

        friend std::istream& operator>>( std::istream&, LightPoint& );
        friend std::istream& operator>>( std::istream&, LightSphere& );
        friend std::istream& operator>>( std::istream&, LightPlane& );
        friend std::istream& operator>>( std::istream&, LightTriangle& );
        friend std::istream& operator>>( std::istream&, Scene& );

    private:
        std::vector< LightPart* > parts;

        Triplet emission; // Light intensity emitted in all directions; not constrained between (0, 0, 0) and (1, 1, 1)
    };

    class IPoint : virtual public Surface {
    public:
        virtual double intersect( const Ray& )    const { return 0; } // Cannot be hit
        virtual Vector getNormal( const Vector& ) const { return Vector::Zero; }

    protected:
        IPoint( const Object* parent ) : Surface( parent ) { }
        IPoint( const Object* parent, const Vector& point )
            : Surface( parent )
            , point( point )
        { }

        virtual void move( const Vector& translation ) { point += translation; }
        virtual void move( double theta, WorldAxis axis ) { rotate( point, theta, axis ); }

        friend std::istream& operator>>( std::istream&, Point& );
        friend std::istream& operator>>( std::istream&, LightPoint& );

    protected:
        Vector point;
    };

    class LightPoint : public IPoint, public LightPart {
    public:
        LightPoint( const Light* parent )
            : Surface  ( parent )
            , IPoint   ( parent )
            , LightPart( parent )
        { }

        void emitZones( std::vector< Tree<Zone>* >& out ) const;
    };

    class ISphere : virtual public Surface {
    public:
        virtual double intersect( const Ray& ray ) const;
        virtual Vector getNormal( const Vector& point ) const { return (point - center).normalize(); }

    protected:
        ISphere( const Object* parent ) : Surface( parent ) { }
        ISphere( const Object* parent, const Vector& center, double radius )
            : Surface( parent )
            , center( center )
            , radius( radius )
        { }

        virtual void move( const Vector& translation ) { center += translation; }
        virtual void move( double theta, WorldAxis axis ) { rotate( center, theta, axis ); }

        friend std::istream& operator>>( std::istream&, Sphere& );
        friend std::istream& operator>>( std::istream&, LightSphere& );

    protected:
        Vector center;
        double radius;
    };

    class Sphere      : public ISphere, public ThingPart {
    public:
        Sphere     ( const Thing* parent )
            : Surface  ( parent )
            , ISphere  ( parent )
            , ThingPart( parent )
        { }

        double getTilt( const Vector& pivot ) const;
        Vector mirror ( const Vector& point ) const;
        std::vector< Vector > getPoints( const Vector& viewpoint ) const;
    };

    class LightSphere : public ISphere, public LightPart {
    public:
        LightSphere( const Light* parent )
            : Surface  ( parent )
            , ISphere  ( parent )
            , LightPart( parent )
        { }

        void emitZones( std::vector< Tree<Zone>* >& out ) const;
    };

    class IPlane : virtual public Surface {
    public:
        virtual double intersect( const Ray& ray ) const;
        virtual Vector getNormal( const Vector& ) const { return normal; }

    protected:
        IPlane( const Object* parent ) : Surface( parent ) { }
        IPlane( const Object* parent, const Vector& normal, double offset )
            : Surface( parent )
            , normal( normal )
            , offset( offset )
        {
            this->normal.normalize();
        }

        virtual void move( const Vector& translation ) { offset += normal * translation; }
        virtual void move( double theta, WorldAxis axis ) { rotate( normal, theta, axis ); }

        friend std::istream& operator>>( std::istream&, Plane& );
        friend std::istream& operator>>( std::istream&, LightPlane& );

    protected:
        Vector normal;
        double offset; // Signed distance from Origin: sign is `+' if the normal points AWAY from Origin, `-' if it points toward it
    };

    class Plane      : public IPlane, public ThingPart {
    public:
        Plane     ( const Thing* parent )
            : Surface  ( parent )
            , IPlane   ( parent )
            , ThingPart( parent )
        { }
        // Dummy Plane ctor
        Plane     ( const Vector& normal, double offset )
            : Surface  ( NULL )
            , IPlane   ( NULL, normal, offset )
            , ThingPart( NULL )
        { }

        double getTilt( const Vector& pivot ) const;
        Vector mirror ( const Vector& point ) const;
        std::vector< Vector > getPoints( const Vector& viewpoint ) const;
    };

    class LightPlane : public IPlane, public LightPart {
    public:
        LightPlane( const Light* parent )
            : Surface  ( parent )
            , IPlane   ( parent )
            , LightPart( parent )
        { }

        void emitZones( std::vector< Tree<Zone>* >& out ) const;
    };

    class ITriangle : virtual public Surface {
    public:
        virtual double intersect( const Ray& ray ) const;
        virtual Vector getNormal( const Vector& ) const
        {
            Vector edge0 = this->points[1] - this->points[0];
            Vector edge1 = this->points[2] - this->points[0];
            return edge0.cross( edge1 ).normalize();
        }

        virtual void move( const Vector& translation )
        {
            points[0] += translation;
            points[1] += translation;
            points[2] += translation;
        }
        virtual void move( double theta, WorldAxis axis )
        {
            rotate( points[0], theta, axis );
            rotate( points[1], theta, axis );
            rotate( points[2], theta, axis );
        }

        friend std::istream& operator>>( std::istream&, Triangle& );
        friend std::istream& operator>>( std::istream&, LightTriangle& );

    protected:
        ITriangle( const Object* parent ) : Surface( parent ) { }
        ITriangle( const Object* parent, const Vector& a, const Vector& b, const Vector& c )
            : Surface( parent )
        {
            points[0] = a;
            points[1] = b;
            points[2] = c;
        }

    protected:
        Vector points[3];
    };

    class Triangle      : public ITriangle, public ThingPart {
    public:
        Triangle     ( const Thing* parent )
            : Surface  ( parent )
            , ITriangle( parent )
            , ThingPart( parent )
        { }

        double getTilt( const Vector& pivot ) const;
        Vector mirror ( const Vector& point ) const;
        std::vector< Vector > getPoints( const Vector& viewpoint ) const;
    };

    class LightTriangle : public ITriangle, public LightPart {
    public:
        LightTriangle( const Light* parent )
            : Surface  ( parent )
            , ITriangle( parent )
            , LightPart( parent )
        { }

        void emitZones( std::vector< Tree<Zone>* >& out ) const;
    };

    struct Sky {
        RGB color; // Skies are not allowed to be emitters

        friend std::istream& operator>>( std::istream&, Sky& );
    };

    class Scene {
    public:
        Scene() : changed( false ) { }
        ~Scene()
        {
            for ( LightIt light = lightsBegin(); light != lightsEnd(); light++ )
                delete *light;
            for ( ThingIt thing = thingsBegin(); thing != thingsEnd(); thing++ )
                delete *thing;
        }

        LightIt lightsBegin() const { return lights.begin(); }
        LightIt lightsEnd()   const { return lights.end();   }
        ThingIt thingsBegin() const { return things.begin(); }
        ThingIt thingsEnd()   const { return things.end();   }

        const Sky& getSky() const { return sky; }

        bool isChanged()    const { return changed; }
        void clearChanged() const { changed = false; }

    private:
        void setChanged()   const { changed = true; }

        friend class Light;
        friend class Thing;
        friend std::istream& operator>>( std::istream&, Scene& );

    private:
        std::vector< Light* > lights;
        std::vector< Thing* > things;
        Sky sky;

        mutable bool changed;
    };

}

#endif // SILENCE_SCENE

