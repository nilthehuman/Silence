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

    // Generic scene object class. This is what every lightsource and thing in the Scene needs to be able to do
    class Surface {
    public:
        Surface() { }
        Surface( bool background, bool backCulled )
            : background( background )
            , backCulled( backCulled )
        { }
        virtual ~Surface() { }

        virtual double intersect( const Ray& ray )      const = 0; // Returns distance from Ray origin; a return value of zero will mean a miss
        virtual Vector getRandomPoint()                 const = 0; // Yields a random point on the Object's surface
        virtual Vector getNormal( const Vector& point ) const = 0; // Returns the outward pointing surface normal

        bool isBackground() const { return background; }

    protected:
        bool background; // A background is a Surface that may only occlude other backgrounds from any direction
        bool backCulled; // Back-face culling makes the negative side of a Surface invisible
    };

    class Sphere;
    class Plane;
    class Triangle;

    // Non-light objects in the Scene
    class Thing : virtual public Surface {
    public:
        Thing() { }
        Thing( const Material& material ) : material( material ) { }
        virtual ~Thing() { }

        const RGB& getColor()            const { return material.getColor(); }
        double     getRefractiveIndex()  const { return material.getRefractiveIndex(); }

        Material::Interaction interact() const { return material.interact(); } // Decides how the surface will behave for a particular hit by a particular Ray

        friend std::istream& operator>>( std::istream& is, Sphere&   sphere );
        friend std::istream& operator>>( std::istream& is, Plane&    plane );
        friend std::istream& operator>>( std::istream& is, Triangle& triangle );

    protected:
        Material material;
    };

    class LightSphere;

    class ISphere : virtual public Surface {
    protected:
        ISphere() { }
        ISphere( bool background, bool backCulled, const Vector& center, double radius )
            : Surface( background, backCulled )
            , center( center )
            , radius( radius )
        { }

    public:
        virtual double intersect( const Ray& ray ) const;
        virtual Vector getNormal( const Vector& point ) const { return (point - center).normalize(); }
        virtual Vector getRandomPoint() const;

        friend std::istream& operator>>( std::istream& is, Sphere& sphere );
        friend std::istream& operator>>( std::istream& is, LightSphere& light );

    protected:
        Vector center;
        double radius;
    };

    class Sphere : public Thing, public ISphere {
    public:
        Sphere() { }
        Sphere( const Material& material, bool background, bool backCulled, const Vector& center, double radius )
            : Thing( material )
            , ISphere( background, backCulled, center, radius )
        { }
    };

    class IPlane : virtual public Surface {
    protected:
        IPlane() { }
        IPlane( bool background, bool backCulled, const Vector& normal, double offset )
            : Surface( background, backCulled )
            , normal( normal )
            , offset( offset )
        {
            this->normal.normalize();
        }

    public:
        virtual double intersect( const Ray& ray ) const;
        virtual Vector getNormal( const Vector& ) const { return normal; }
        virtual Vector getRandomPoint() const;

        friend std::istream& operator>>( std::istream& is, Plane& plane );

    protected:
        Vector normal;
        double offset; // Signed distance from Origin: sign is `+' if the normal points AWAY from Origin, `-' if it points toward it
    };

    class Plane : public Thing, public IPlane {
    public:
        Plane() { }
        Plane( const Material& material, bool background, bool backCulled, Vector normal, double offset )
            : Thing( material )
            , IPlane( background, backCulled, normal, offset )
        { }
    };

    class LightTriangle;

    class ITriangle : virtual public Surface {
    protected:
        ITriangle() { }
        ITriangle( bool background, bool backCulled, const Vector& a, const Vector& b, const Vector& c )
            : Surface( background, backCulled )
        {
            points[0] = a;
            points[1] = b;
            points[2] = c;
        }

    public:
        virtual double intersect( const Ray& ray ) const;
        virtual Vector getNormal( const Vector& ) const
        {
            Vector edge0 = this->points[1] - this->points[0];
            Vector edge1 = this->points[2] - this->points[0];
            return edge0.cross( edge1 ).normalize();
        }
        virtual Vector getRandomPoint() const;

        friend std::istream& operator>>( std::istream& is, Triangle& triangle );
        friend std::istream& operator>>( std::istream& is, LightTriangle& light );

    protected:
        Vector points[3];
    };

    class Triangle : public Thing, public ITriangle {
    public:
        Triangle() { }
        Triangle( const Material& material, bool background, bool backCulled, const Vector& a, const Vector& b, const Vector& c )
            : Thing( material )
            , ITriangle( background, backCulled, a, b, c )
        { }
    };

    class Light : virtual public Surface {
    protected:
        Light() { }
        Light( const Triplet& emission ) : emission( emission ) { }

    public:
        virtual ~Light() { }

        virtual Triplet getEmission( const Vector& ) const { return emission; }

    protected:
        Triplet emission; // Light intensity emitted in all directions; not constrained between (0, 0, 0) and (1, 1, 1)
    };

    class LightPoint : public Light {
    public:
        LightPoint() { }
        LightPoint( const RGB& color, const Vector& point )
            : Light( color )
            , point( point )
        { }

        virtual double intersect( const Ray& )    const { return 0; } // Cannot be hit
        virtual Vector getRandomPoint()           const { return point; }
        virtual Vector getNormal( const Vector& ) const { return Vector::Zero; }
        virtual bool   isBackground()             const { return true; } // Never occludes another object

        friend std::istream& operator>>( std::istream& is, LightPoint& light );

    protected:
        Vector point;
    };

    class LightSphere : public Light, public ISphere {
    public:
        LightSphere() { }
        LightSphere( const RGB& color, bool background, bool backCulled, const Vector& center, double radius )
            : Light( color )
            , ISphere( background, backCulled, center, radius )
        { }

        friend std::istream& operator>>( std::istream& is, LightSphere& light );
    };

    class LightTriangle : public Light, public ITriangle {
    public:
        LightTriangle() { }
        LightTriangle( const RGB& color, bool background, bool backCulled, const Vector& a, const Vector& b, const Vector& c )
            : Light( color )
            , ITriangle( background, backCulled, a, b, c )
        { }

        virtual Triplet getEmission( const Vector& direction ) const
        {
            double tilt = direction * getNormal( points[0] );
            if ( backCulled && tilt < 0 )
                return RGB::Black;
            return emission * abs( tilt );
        }

        friend std::istream& operator>>( std::istream& is, LightTriangle& light );
    };

    class Scene {
    public:
        Scene() : lights(), things() { }
        ~Scene()
        {
            for ( std::vector< Light* >::const_iterator it = lightsBegin(); it != lightsEnd(); it++ )
                delete *it;
            for ( std::vector< Thing* >::const_iterator it = thingsBegin(); it != thingsEnd(); it++ )
                delete *it;
        }

        std::vector< Light* >::const_iterator lightsBegin() const { return lights.begin(); }
        std::vector< Light* >::const_iterator lightsEnd()   const { return lights.end();   }
        std::vector< Thing* >::const_iterator thingsBegin() const { return things.begin(); }
        std::vector< Thing* >::const_iterator thingsEnd()   const { return things.end();   }

        Triplet getDirectLight( const Vector& point, const Vector& normal ) const; // Total direct illumination from all lightsources to point

        friend std::istream& operator>>( std::istream& is, Scene& scene );

    private:
        std::vector< Light* > lights;
        std::vector< Thing* > things;
    };

}

#endif // RETRA_SCENE

