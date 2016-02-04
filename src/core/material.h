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

// Material class for the surface qualities of Things
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_MATERIAL
#define SILENCE_MATERIAL

#include <cstdlib>
#include <cassert>

#include "triplet.h"

namespace Silence {

    class Material {
    public:
        enum Interaction { DIFFUSE, METALLIC, REFLECT, REFRACT };

    private:
        struct Character {
            Character()
                : diffuse   ( 1 )
                , metallic  ( 0 )
                , reflecting( 0 )
                , refractive( 0 )
            { }
            Character( double diffuse, double metallic, double reflecting, double refractive )
                : diffuse   ( diffuse )
                , metallic  ( metallic )
                , reflecting( reflecting )
                , refractive( refractive )
            {
                assert( 0 <= diffuse    && diffuse    <= 1 );
                assert( 0 <= metallic   && metallic   <= 1 );
                assert( 0 <= reflecting && reflecting <= 1 );
                assert( 0 <= refractive && refractive <= 1 );
                assert( equal( diffuse + metallic + reflecting + refractive, 1.0 ) );
            }

            // The BRDF of a Surface is modeled as a mixture of 4 ideal BRDF's
            double diffuse;
            double metallic;
            double reflecting;
            double refractive;
        };

    public:
        Material()
            : character()
            , color( RGB::Black )
            , refractiveIndex( 1 )
        { }
        Material( double diffuse, double metallic, double reflecting, double refractive, RGB color, double refractiveIndex = 1 )
            : character( diffuse, metallic, reflecting, refractive )
            , color( color )
            , refractiveIndex( refractiveIndex )
        {
            assert( 1 <= refractiveIndex );
        }
        Material( Character character, RGB color, double refractiveIndex = 1 )
            : character( character )
            , color( color )
            , refractiveIndex( refractiveIndex )
        {
            assert( 1 <= refractiveIndex );
        }

        const RGB& getColor()           const { return color; }
        double     getRefractiveIndex() const { return refractiveIndex; }

        double interact( Interaction interaction ) const
        {
            switch ( interaction )
            {
                case DIFFUSE:  return character.diffuse;
                case METALLIC: return character.metallic;
                case REFLECT:  return character.reflecting;
                case REFRACT:  return character.refractive;
                default:       assert( false );
            }
        }

        friend std::istream& operator>>( std::istream& is, Character& character );
        friend std::istream& operator>>( std::istream& is, Material& material );

    private:
        Character character;
        RGB       color;
        double    refractiveIndex; // Going FROM vacuum INTO the Material
    };

}

#endif // SILENCE_MATERIAL

