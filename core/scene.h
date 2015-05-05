/*
 * Copyright 2015 Dániel Arató
 *
 * This file is part of Retra.
 *
 * Retra is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Retra is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Retra.  If not, see <http://www.gnu.org/licenses/>.
 */

// Class hierarchy for representing a Scene and all parts of it
// Part of Retra, the Reference Tracer

#ifndef RETRA_SCENE
#define RETRA_SCENE

#include <vector>
#include <cassert>

#include "aux.h"
#include "material.h"
#include "triplet.h"

namespace Retra {

    class Ray;
    class Scene;

    class Thing;
    class ThingPart;
    class Light;
    class LightPart;
    typedef std::vector< Thing* >    ::const_iterator ThingIt;
    typedef std::vector< ThingPart* >::const_iterator ThingPartIt;
    typedef std::vector< Light* >    ::const_iterator LightIt;
    typedef std::vector< LightPart* >::const_iterator LightPartIt;

    // A set of Surfaces that delimit the same physical object
    class Object {
    public:
        bool isBackground() const { return background; }
        bool isBackCulled() const { return backCulled; }

    protected:
        Object() { }

        friend std::istream& operator>>( std::istream& is, Scene& );

        bool background; // A background is a Surface that may only occlude other backgrounds from any direction
        bool backCulled; // Back-face culling makes the negative side of Surfaces invisible
    };

    // Generic surface primitive class. This is what every lightsource and thing in the Scene needs to be able to do
    class Surface {
    public:
        virtual double intersect( const Ray& ray )      const = 0; // Returns distance from Ray origin; a return value of zero will mean a miss
        virtual Vector getRandomPoint()                 const = 0; // Yields a random point on the surface
        virtual Vector getNormal( const Vector& point ) const = 0; // Returns the outward pointing surface normal

    protected:
        Surface() : parent( NULL ) { }
        Surface( const Object* parent ) : parent( parent ) { }
        virtual ~Surface() { }

    protected:
        const Object* parent;
    };

    class ThingPart : virtual public Surface { };

    class Light;
    class LightPart : virtual public Surface {
    public:
        virtual Triplet getEmission( const Vector& ) const;
    };

    class Point;
    class Sphere;
    class Plane;
    class Triangle;
    // Non-light complex objects in the Scene
    class Thing : public Object {
    public:
        Thing() : parts() { }
        ~Thing()
        {
            for ( ThingPartIt it = partsBegin(); it != partsEnd(); it++ )
                delete *it;
        }

        void push_back( ThingPart* part ) { parts.push_back( part ); }

        ThingPartIt partsBegin() const { return parts.begin(); }
        ThingPartIt partsEnd()   const { return parts.end();   }

        const ThingPart* getRandomPart() const
        {
            const int index = (int)( (double)rand() / RAND_MAX * parts.size() );
            return parts[ index ];
        }

        const RGB& getColor()            const { return material.getColor(); }
        double     getRefractiveIndex()  const { return material.getRefractiveIndex(); }

        Material::Interaction interact() const { return material.interact(); } // Decides how the surface will behave for a particular hit by a particular Ray

        friend std::istream& operator>>( std::istream& is, Sphere& );
        friend std::istream& operator>>( std::istream& is, Plane& );
        friend std::istream& operator>>( std::istream& is, Triangle& );
        friend std::istream& operator>>( std::istream& is, Scene& );

    private:
        std::vector< ThingPart* > parts;

        Material material;
    };

    class LightPoint;
    class LightSphere;
    class LightPlane;
    class LightTriangle;
    // Complex light objects
    class Light : public Object {
    public:
        Light() : parts() { }

        void push_back( LightPart* part ) { parts.push_back( part ); }

        LightPartIt partsBegin() const { return parts.begin(); }
        LightPartIt partsEnd()   const { return parts.end();   }

        const LightPart* getRandomPart() const
        {
            const int index = rand() % parts.size();
            return parts[ index ];
        }

        const Triplet& getEmission() const { return emission; }

        friend std::istream& operator>>( std::istream& is, LightPoint& );
        friend std::istream& operator>>( std::istream& is, LightSphere& );
        friend std::istream& operator>>( std::istream& is, LightPlane& );
        friend std::istream& operator>>( std::istream& is, LightTriangle& );
        friend std::istream& operator>>( std::istream& is, Scene& );

    private:
        std::vector< LightPart* > parts;

        Triplet emission; // Light intensity emitted in all directions; not constrained between (0, 0, 0) and (1, 1, 1)
    };

    class IPoint : virtual public Surface {
    public:
        virtual double intersect( const Ray& )    const { return 0; } // Cannot be hit
        virtual Vector getRandomPoint()           const { return point; }
        virtual Vector getNormal( const Vector& ) const { return Vector::Zero; }

    protected:
        IPoint() { }
        IPoint( const Vector& point ) : point( point ) { }

        friend std::istream& operator>>( std::istream& is, Point& );
        friend std::istream& operator>>( std::istream& is, LightPoint& );

    protected:
        Vector point;
    };

    class LightPoint : public IPoint, public LightPart {
    public:
        LightPoint( const Light* parent ) : Surface( parent ) { }
    };

    class ISphere : virtual public Surface {
    public:
        virtual double intersect( const Ray& ray ) const;
        virtual Vector getNormal( const Vector& point ) const { return (point - center).normalize(); }
        virtual Vector getRandomPoint() const;

    protected:
        ISphere() { }
        ISphere( const Vector& center, double radius )
            : center( center )
            , radius( radius )
        { }

        friend std::istream& operator>>( std::istream& is, Sphere& );
        friend std::istream& operator>>( std::istream& is, LightSphere& );

    protected:
        Vector center;
        double radius;
    };

    class Sphere      : public ISphere, public ThingPart {
    public:
        Sphere     ( const Thing* parent ) : Surface( parent ) { }
    };

    class LightSphere : public ISphere, public LightPart {
    public:
        LightSphere( const Light* parent ) : Surface( parent ) { }
    };

    class IPlane : virtual public Surface {
    public:
        virtual double intersect( const Ray& ray ) const;
        virtual Vector getNormal( const Vector& ) const { return normal; }
        virtual Vector getRandomPoint() const;

    protected:
        IPlane() { }
        IPlane( const Vector& normal, double offset )
            : normal( normal )
            , offset( offset )
        {
            this->normal.normalize();
        }

        friend std::istream& operator>>( std::istream& is, Plane& );
        friend std::istream& operator>>( std::istream& is, LightPlane& );

    protected:
        Vector normal;
        double offset; // Signed distance from Origin: sign is `+' if the normal points AWAY from Origin, `-' if it points toward it
    };

    class Plane      : public IPlane, public ThingPart {
    public:
        Plane     ( const Thing* parent ) : Surface( parent ) { }
    };

    class LightPlane : public IPlane, public LightPart {
    public:
        LightPlane( const Light* parent ) : Surface( parent ) { }
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
        virtual Vector getRandomPoint() const;

        friend std::istream& operator>>( std::istream& is, Triangle& );
        friend std::istream& operator>>( std::istream& is, LightTriangle& );

    protected:
        ITriangle() { }
        ITriangle( const Vector& a, const Vector& b, const Vector& c )
        {
            points[0] = a;
            points[1] = b;
            points[2] = c;
        }

    private:
        Vector points[3];
    };

    class Triangle      : public ITriangle, public ThingPart {
    public:
        Triangle     ( const Thing* parent ) : Surface( parent ) { }
    };

    class LightTriangle : public ITriangle, public LightPart {
    public:
        LightTriangle( const Light* parent ) : Surface( parent ) { }

        virtual Triplet getEmission( const Vector& direction ) const;
    };

    struct Sky {
        RGB color; // Skies are not allowed to be emitters

        friend std::istream& operator>>( std::istream& is, Sky& );
    };

    class Scene {
    public:
        Scene() : lights(), things() { }
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

        Triplet getDirectLight( const Vector& point, const Vector& normal ) const; // Total direct illumination from all lightsources to point

        const Sky& getSky() const { return sky; }

        friend std::istream& operator>>( std::istream& is, Scene& );

    private:
        std::vector< Light* > lights;
        std::vector< Thing* > things;
        Sky sky;
    };

}

#endif // RETRA_SCENE

