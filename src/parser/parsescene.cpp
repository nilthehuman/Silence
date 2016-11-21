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

// A half-assed parser for our JSON scene description format
// (This parser is non-strict and will not complain about certain types
// of formal errors in the scene description)
// Part of Silence, an experimental rendering engine

#include "parsescene.h"

#include <fstream>
#include <limits>
#include <sstream>

#include "../core/camera.h"
#include "../core/material.h"
#include "../core/scene.h"

namespace Silence {

    static const std::string tokenErrorMessage = "unrecognized token: ";

    static Light* light = NULL;
    static Thing* thing = NULL;

    // Read and parse scene description
    Camera* parseScene( std::istream& is )
    {
        Scene*  scene      = new Scene;
        Camera* camera     = new Camera( scene );
        bool sceneDefined  = false;
        bool cameraDefined = false;

        int jsonDepth = 0;
        while( !is.eof() )
        {
            std::string token;
            is >> std::ws >> token;
            if     ( token == "{"           )
            {
                if ( ++jsonDepth == 2 )
                    std::cerr << "parseScene: warning: Scene and Camera should be defined in the top level JSON object" << std::endl;
            }
            else if( token == "}"           )
            {
                if ( --jsonDepth < 0 )
                    std::cerr << "parseScene: warning: ignoring unexpected '}' character in scene file" << std::endl;
            }
            else if( token == "\"camera\":" )
            {
                if ( cameraDefined )
                    throw std::string( "the scene file has multiple Cameras defined; please specify a single Camera instead" );
                if ( modeFlags.verbose )
                    std::cerr << "parseScene: reading Camera from input scene description... ";
                is >> *camera;
                cameraDefined = true;
                if ( modeFlags.verbose )
                    std::cerr << "OK." << std::endl;
            }
            else if( token == "\"scene\":"  )
            {
                if ( sceneDefined )
                    throw std::string( "the scene file has multiple Scenes defined; please specify a single Scene instead" );
                is >> *scene;
                sceneDefined = true;
            }
            else if( token == ","           )
            {
            }
            else if( token.empty()          )
            {
                // The file had some trailing whitespace, but we've reached the end
                assert( is.eof() );
                break;
            }
            else
                throw std::string( "unrecognized token in scene file: '" + token + "'" );
        }
        if( !camera )
            throw std::string( "no Camera defined in scene file" );
        if( !scene )
            throw std::string( "no Scene defined in scene file" );
        return camera;
    }

    std::istream& operator>>( std::istream& is, Triplet& triplet )
    {
        is.ignore(std::numeric_limits< std::streamsize >::max(), '[') >> triplet.x;
        is.ignore(std::numeric_limits< std::streamsize >::max(), ',') >> triplet.y;
        is.ignore(std::numeric_limits< std::streamsize >::max(), ',') >> triplet.z;
        is.ignore(std::numeric_limits< std::streamsize >::max(), ']');
        return is;
    }

    // Read Camera parameters from character stream
    std::istream& operator>>( std::istream& is, Camera& camera )
    {
        if ( camera.pixels )
        {
            // Free pixel table
            for ( int i = 0; i < camera.screen.gridheight; ++i )
                delete[] camera.pixels[i];
            delete[] camera.pixels;
        }
        if ( camera.skyMask )
        {
            // Free skyMask array
            for ( int i = 0; i < camera.screen.gridheight; ++i )
                delete[] camera.skyMask[i];
            delete[] camera.skyMask;
        }
        bool viewpointDefined      = false;
        bool screenDefined         = false;
        bool gridResolutionDefined = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if      ( token == "\"viewpoint\":"      )
            {
                if ( viewpointDefined ) throw std::string("multiple definitions of \"viewpoint\"");
                viewpointDefined = true;
                is >> camera.viewpoint;
            }
            else if ( token == "\"screen\":"         )
            {
                if ( screenDefined ) throw std::string("multiple definitions of \"screen\"");
                screenDefined = true;
                is.ignore(std::numeric_limits< std::streamsize >::max(), '[') >> camera.screen.window[0];
                is.ignore(std::numeric_limits< std::streamsize >::max(), ',') >> camera.screen.window[1];
                is.ignore(std::numeric_limits< std::streamsize >::max(), ',') >> camera.screen.window[2];
                is.ignore(std::numeric_limits< std::streamsize >::max(), ',') >> camera.screen.window[3];
                is.ignore(std::numeric_limits< std::streamsize >::max(), ']');
            }
            else if ( token == "\"gridresolution\":" )
            {
                if ( gridResolutionDefined ) throw std::string("multiple definitions of \"gridresolution\"");
                gridResolutionDefined = true;
                is.ignore(std::numeric_limits< std::streamsize >::max(), '[') >> camera.screen.gridwidth;
                is.ignore(std::numeric_limits< std::streamsize >::max(), ',') >> camera.screen.gridheight;
                is.ignore(std::numeric_limits< std::streamsize >::max(), ']');
            }
            else if ( token == ","                   )
            {
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        // Allocate new pixel table
        camera.pixels = new RGB*[camera.screen.gridheight];
        for ( int i = 0; i < camera.screen.gridheight; ++i )
            camera.pixels[i] = new RGB[camera.screen.gridwidth];
        // Allocate new skyMask array
        camera.skyMask = new double*[camera.screen.gridheight];
        for ( int i = 0; i < camera.screen.gridheight; ++i )
            camera.skyMask[i] = new double[camera.screen.gridwidth];
        return is;
    }

    std::istream& operator>>( std::istream& is, Material::Character& character )
    {
        double diffuse;
        double metallic = 0;
        double reflecting;
        double refractive;
        bool   diffuseDefined    = false;
        bool   metallicDefined   = false;
        bool   reflectingDefined = false;
        bool   refractiveDefined = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if      ( token == "\"diffuse\":"    )
            {
                if ( diffuseDefined ) throw std::string("multiple definitions of \"diffuse\"");
                diffuseDefined = true;
                is >> diffuse;
            }
            else if ( token == "\"metallic\":" )
            {
                if ( reflectingDefined ) throw std::string("multiple definitions of \"metallic\"");
                metallicDefined = true;
                is >> metallic;
            }
            else if ( token == "\"reflecting\":" )
            {
                if ( reflectingDefined ) throw std::string("multiple definitions of \"reflecting\"");
                reflectingDefined = true;
                is >> reflecting;
            }
            else if ( token == "\"refractive\":" )
            {
                if ( refractiveDefined ) throw std::string("multiple definitions of \"refractive\"");
                refractiveDefined = true;
                is >> refractive;
            }
            else if ( token == ","               )
            {
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        if ( !diffuseDefined    ) throw std::string("\"diffuse\" undefined");
        if ( !metallicDefined   ) throw std::string("\"metallic\" undefined");
        if ( !reflectingDefined ) throw std::string("\"reflecting\" undefined");
        if ( !refractiveDefined ) throw std::string("\"refractive\" undefined");
        character = Material::Character( diffuse, metallic, reflecting, refractive );
        return is;
    }

    std::istream& operator>>( std::istream& is, Material& material )
    {
        bool characterDefined       = false;
        bool colorDefined           = false;
        bool refractiveIndexDefined = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if      ( token == "\"character\":"       )
            {
                if ( characterDefined ) throw std::string("multiple definitions of \"character\"");
                characterDefined = true;
                is >> material.character;
            }
            else if ( token == "\"color\":"           )
            {
                if ( colorDefined ) throw std::string("multiple definitions of \"color\"");
                colorDefined = true;
                is >> material.color;
            }
            else if ( token == "\"refractiveindex\":" )
            {
                if ( refractiveIndexDefined ) throw std::string("multiple definitions of \"refractiveindex\"");
                refractiveIndexDefined = true;
                is >> material.refractiveIndex;
            }
            else if ( token == ","                    )
            {
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        if ( !characterDefined ) throw std::string("\"diffuse\" undefined");
        if ( !colorDefined     ) throw std::string("\"reflecting\" undefined");
        return is;
    }

    std::istream& operator>>( std::istream& is, LightPoint& lightPoint )
    {
        bool emissionDefined = false;
        bool pointDefined    = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if      ( token == "\"emission\":" )
            {
                if ( emissionDefined ) throw std::string("multiple definitions of \"emission\"");
                emissionDefined = true;
                is >> light->emission;
            }
            else if ( token == "\"point\":"    )
            {
                if ( pointDefined ) throw std::string("multiple definitions of \"point\"");
                pointDefined = true;
                is >> lightPoint.point;
            }
            else if ( token == ","             )
            {
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        if ( !pointDefined ) throw std::string("\"point\" undefined");
        return is;
    }

    std::istream& operator>>( std::istream& is, LightSphere& lightSphere )
    {
        bool emissionDefined   = false;
        bool backgroundDefined = false;
        bool backCulledDefined = false;
        bool centerDefined     = false;
        bool radiusDefined     = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if      ( token == "\"emission\":"   )
            {
                if ( emissionDefined ) throw std::string("multiple definitions of \"emission\"");
                emissionDefined = true;
                is >> light->emission;
            }
            else if ( token == "\"background\":" )
            {
                if ( backgroundDefined ) throw std::string("multiple definitions of \"background\"");
                backgroundDefined = true;
                is >> light->background;
            }
            else if ( token == "\"backculled\":" )
            {
                if ( backCulledDefined ) throw std::string("multiple definitions of \"backculled\"");
                backCulledDefined = true;
                is >> light->backCulled;
            }
            else if ( token == "\"center\":"     )
            {
                if ( centerDefined ) throw std::string("multiple definitions of \"center\"");
                centerDefined = true;
                is >> lightSphere.center;
            }
            else if ( token == "\"radius\":"     )
            {
                if ( radiusDefined ) throw std::string("multiple definitions of \"radius\"");
                radiusDefined = true;
                is >> lightSphere.radius;
            }
            else if ( token == ","               )
            {
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        if ( !centerDefined ) throw std::string("\"center\" undefined");
        if ( !radiusDefined ) throw std::string("\"radius\" undefined");
        return is;
    }

    std::istream& operator>>( std::istream& is, LightPlane& lightPlane )
    {
        bool emissionDefined   = false;
        bool backgroundDefined = false;
        bool backCulledDefined = false;
        bool normalDefined     = false;
        bool offsetDefined     = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if      ( token == "\"emission\":"   )
            {
                if ( emissionDefined ) throw std::string("multiple definitions of \"emission\"");
                emissionDefined = true;
                is >> light->emission;
            }
            else if ( token == "\"background\":" )
            {
                if ( backgroundDefined ) throw std::string("multiple definitions of \"background\"");
                backgroundDefined = true;
                is >> light->background;
            }
            else if ( token == "\"backculled\":" )
            {
                if ( backCulledDefined ) throw std::string("multiple definitions of \"backculled\"");
                backCulledDefined = true;
                is >> light->backCulled;
            }
            else if ( token == "\"normal\":"     )
            {
                if ( normalDefined ) throw std::string("multiple definitions of \"normal\"");
                normalDefined = true;
                is >> lightPlane.normal;
            }
            else if ( token == "\"offset\":"     )
            {
                if ( offsetDefined ) throw std::string("multiple definitions of \"offset\"");
                offsetDefined = true;
                is >> lightPlane.offset;
            }
            else if ( token == ","               )
            {
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        if ( !normalDefined ) throw std::string("\"normal\" undefined");
        if ( !offsetDefined ) throw std::string("\"offset\" undefined");
        return is;
    }

    std::istream& operator>>( std::istream& is, LightTriangle& lightTriangle )
    {
        bool emissionDefined   = false;
        bool backgroundDefined = false;
        bool backCulledDefined = false;
        bool pointsDefined     = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if      ( token == "\"emission\":"   )
            {
                if ( emissionDefined ) throw std::string("multiple definitions of \"emission\"");
                emissionDefined = true;
                is >> light->emission;
            }
            else if ( token == "\"background\":" )
            {
                if ( backgroundDefined ) throw std::string("multiple definitions of \"background\"");
                backgroundDefined = true;
                is >> light->background;
            }
            else if ( token == "\"backculled\":" )
            {
                if ( backCulledDefined ) throw std::string("multiple definitions of \"backculled\"");
                backCulledDefined = true;
                is >> light->backCulled;
            }
            else if ( token == "\"points\":"     )
            {
                if ( pointsDefined ) throw std::string("multiple definitions of \"points\"");
                pointsDefined = true;
                is.ignore(std::numeric_limits< std::streamsize >::max(), '[') >> lightTriangle.points[0];
                is.ignore(std::numeric_limits< std::streamsize >::max(), ',') >> lightTriangle.points[1];
                is.ignore(std::numeric_limits< std::streamsize >::max(), ',') >> lightTriangle.points[2];
                is.ignore(std::numeric_limits< std::streamsize >::max(), ']');
            }
            else if ( token == ","               )
            {
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        if ( !pointsDefined ) throw std::string("\"points\" undefined");
        return is;
    }

    std::istream& operator>>( std::istream& is, Sphere& sphere )
    {
        bool materialDefined   = false;
        bool backgroundDefined = false;
        bool backCulledDefined = false;
        bool centerDefined     = false;
        bool radiusDefined     = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if      ( token == "\"material\":"   )
            {
                if ( materialDefined ) throw std::string("multiple definitions of \"material\"");
                materialDefined = true;
                is >> thing->material;
            }
            else if ( token == "\"background\":" )
            {
                if ( backgroundDefined ) throw std::string("multiple definitions of \"background\"");
                backgroundDefined = true;
                is >> thing->background;
            }
            else if ( token == "\"backculled\":" )
            {
                if ( backCulledDefined ) throw std::string("multiple definitions of \"backculled\"");
                backCulledDefined = true;
                is >> thing->backCulled;
            }
            else if ( token == "\"center\":"     )
            {
                if ( centerDefined ) throw std::string("multiple definitions of \"center\"");
                centerDefined = true;
                is >> sphere.center;
            }
            else if ( token == "\"radius\":"     )
            {
                if ( radiusDefined ) throw std::string("multiple definitions of \"radius\"");
                radiusDefined = true;
                is >> sphere.radius;
            }
            else if ( token == ","               )
            {
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        if ( !centerDefined ) throw std::string("\"center\" undefined");
        if ( !radiusDefined ) throw std::string("\"radius\" undefined");
        return is;
    }

    std::istream& operator>>( std::istream& is, Plane& plane )
    {
        bool materialDefined   = false;
        bool backgroundDefined = false;
        bool backCulledDefined = false;
        bool normalDefined     = false;
        bool offsetDefined     = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if      ( token == "\"material\":"   )
            {
                if ( materialDefined ) throw std::string("multiple definitions of \"material\"");
                materialDefined = true;
                is >> thing->material;
            }
            else if ( token == "\"background\":" )
            {
                if ( backgroundDefined ) throw std::string("multiple definitions of \"background\"");
                backgroundDefined = true;
                is >> thing->background;
            }
            else if ( token == "\"backculled\":" )
            {
                if ( backCulledDefined ) throw std::string("multiple definitions of \"backculled\"");
                backCulledDefined = true;
                is >> thing->backCulled;
            }
            else if ( token == "\"normal\":"     )
            {
                if ( normalDefined ) throw std::string("multiple definitions of \"normal\"");
                normalDefined = true;
                is >> plane.normal;
            }
            else if ( token == "\"offset\":"     )
            {
                if ( offsetDefined ) throw std::string("multiple definitions of \"offset\"");
                offsetDefined = true;
                is >> plane.offset;
            }
            else if ( token == ","               )
            {
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        if ( !normalDefined ) throw std::string("\"normal\" undefined");
        if ( !offsetDefined ) throw std::string("\"offset\" undefined");
        return is;
    }

    std::istream& operator>>( std::istream& is, Triangle& triangle )
    {
        bool materialDefined   = false;
        bool backgroundDefined = false;
        bool backCulledDefined = false;
        bool pointsDefined     = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if      ( token == "\"material\":"   )
            {
                if ( materialDefined ) throw std::string("multiple definitions of \"material\"");
                materialDefined = true;
                is >> thing->material;
            }
            else if ( token == "\"background\":" )
            {
                if ( backgroundDefined ) throw std::string("multiple definitions of \"background\"");
                backgroundDefined = true;
                is >> thing->background;
            }
            else if ( token == "\"backculled\":" )
            {
                if ( backCulledDefined ) throw std::string("multiple definitions of \"backculled\"");
                backCulledDefined = true;
                is >> thing->backCulled;
            }
            else if ( token == "\"points\":"     )
            {
                if ( pointsDefined ) throw std::string("multiple definitions of \"points\"");
                pointsDefined = true;
                is.ignore(std::numeric_limits< std::streamsize >::max(), '[') >> triangle.points[0];
                is.ignore(std::numeric_limits< std::streamsize >::max(), ',') >> triangle.points[1];
                is.ignore(std::numeric_limits< std::streamsize >::max(), ',') >> triangle.points[2];
                is.ignore(std::numeric_limits< std::streamsize >::max(), ']');
            }
            else if ( token == ","               )
            {
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        if ( !pointsDefined ) throw std::string("\"points\" undefined");
        return is;
    }

    std::istream& operator>>( std::istream& is, Sky& sky )
    {
        bool colorDefined = false;
        is.ignore(std::numeric_limits< std::streamsize >::max(), '{');
        std::string token;
        is >> token;
        while ( token[0] != '}' )
        {
            if ( token == "\"color\":"   )
            {
                if ( colorDefined ) throw std::string("multiple definitions of \"color\"");
                colorDefined = true;
                is >> sky.color;
            }
            else throw tokenErrorMessage + token;
            is >> token;
        }
        if ( !colorDefined ) throw std::string("\"color\" undefined");
        return is;
    }

    // Read Scene description from character stream
    std::istream& operator>>( std::istream& is, Scene& scene )
    {
        bool skyDefined = false;

        int objectNumber = 0;

        is.ignore(std::numeric_limits< std::streamsize >::max(), '[');
        is >> std::boolalpha;
        std::string token;
        while ( token[0] != ']' || light || thing )
        {
            is >> token;
            try
            {
                if ( token == "{" || token == "}" || token == "}," )
                {
                    continue;
                }
                else if ( token == "\"light\":" )
                {
                    if ( light )
                        throw std::string( "nested Lights are not allowed" );
                    if ( modeFlags.verbose )
                        std::cerr << "parseScene: reading Light from input scene description..." << std::endl;
                    light = new Light( &scene );
                    is.ignore(std::numeric_limits< std::streamsize >::max(), '[');
                }
                else if ( token == "\"thing\":" )
                {
                    if ( thing )
                        throw std::string( "nested Things are not allowed" );
                    if ( modeFlags.verbose )
                        std::cerr << "parseScene: reading Thing from input scene description..." << std::endl;
                    thing = new Thing( &scene );
                    is.ignore(std::numeric_limits< std::streamsize >::max(), '[');
                }
                else if ( token == "\"background\":" )
                {
                    if      ( light )
                        is >> light->background;
                    else if ( thing )
                        is >> thing->background;
                    else
                        throw std::string( "unexpected \"background\" token" );
                }
                else if ( token == "\"backculled\":" )
                {
                    if      ( light )
                        is >> light->backCulled;
                    else if ( thing )
                        is >> thing->backCulled;
                    else
                        throw std::string( "unexpected \"backculled\" token" );
                }
                else if ( token == "\"emission\":" )
                {
                    if      ( light )
                        is >> light->emission;
                    else if ( thing )
                        throw std::string( "unexpected \"emission\" token" );
                    else
                        throw std::string( "unexpected \"emission\" token" );
                }
                else if ( token == "\"material\":" )
                {
                    if      ( light )
                        throw std::string( "unexpected \"material\" token" );
                    else if ( thing )
                        is >> thing->material;
                    else
                        throw std::string( "unexpected \"material\" token" );
                }
                else if ( token == "\"lightpoint\":" )
                {
                    if ( thing )
                        throw std::string( "a Thing cannot contain light parts" );
                    if ( modeFlags.verbose )
                        std::cerr << "parseScene: reading LightPoint from input scene description... ";
                    bool single = !light;
                    if ( single )
                        light = new Light( &scene );
                    LightPoint* lightPoint = new LightPoint( light );
                    is >> *lightPoint;
                    is.ignore(std::numeric_limits< std::streamsize >::max(), '}');
                    light->push_back( lightPoint );
                    if ( single )
                    {
                        scene.lights.push_back( light );
                        light = NULL;
                    }
                    ++objectNumber;
                    if ( modeFlags.verbose )
                        std::cerr << "OK." << std::endl;
                }
                else if ( token == "\"lightsphere\":" )
                {
                    if ( thing )
                        throw std::string( "a Thing cannot contain light parts" );
                    if ( modeFlags.verbose )
                        std::cerr << "parseScene: reading LightSphere from input scene description... ";
                    bool single = !light;
                    if ( single )
                        light = new Light( &scene );
                    LightSphere* lightSphere = new LightSphere( light );
                    is >> *lightSphere;
                    is.ignore(std::numeric_limits< std::streamsize >::max(), '}');
                    light->push_back( lightSphere );
                    if ( single )
                    {
                        scene.lights.push_back( light );
                        light = NULL;
                    }
                    ++objectNumber;
                    if ( modeFlags.verbose )
                        std::cerr << "OK." << std::endl;
                }
                else if ( token == "\"lightplane\":" )
                {
                    if ( thing )
                        throw std::string( "a Thing cannot contain light parts" );
                    if ( modeFlags.verbose )
                        std::cerr << "parseScene: reading LightPlane from input scene description... ";
                    bool single = !light;
                    if ( single )
                        light = new Light( &scene );
                    LightPlane* lightPlane = new LightPlane( light );
                    is >> *lightPlane;
                    is.ignore(std::numeric_limits< std::streamsize >::max(), '}');
                    light->push_back( lightPlane );
                    if ( single )
                    {
                        scene.lights.push_back( light );
                        light = NULL;
                    }
                    ++objectNumber;
                    if ( modeFlags.verbose )
                        std::cerr << "OK." << std::endl;
                }
                else if ( token == "\"lighttriangle\":" )
                {
                    if ( thing )
                        throw std::string( "a Thing cannot contain light parts" );
                    if ( modeFlags.verbose )
                        std::cerr << "parseScene: reading LightTriangle from input scene description... ";
                    bool single = !light;
                    if ( single )
                        light = new Light( &scene );
                    light = new Light( &scene );
                    LightTriangle* lightTriangle = new LightTriangle( light );
                    is >> *lightTriangle;
                    is.ignore(std::numeric_limits< std::streamsize >::max(), '}');
                    light->push_back( lightTriangle );
                    if ( single )
                    {
                        scene.lights.push_back( light );
                        light = NULL;
                    }
                    ++objectNumber;
                    if ( modeFlags.verbose )
                        std::cerr << "OK." << std::endl;
                }
                else if ( token == "\"sphere\":" )
                {
                    if ( light )
                        throw std::string( "a Light cannot contain thing parts" );
                    if ( modeFlags.verbose )
                        std::cerr << "parseScene: reading Sphere from input scene description... ";
                    bool single = !thing;
                    if ( single )
                        thing = new Thing( &scene );
                    Sphere* sphere = new Sphere( thing );
                    is >> *sphere;
                    is.ignore(std::numeric_limits< std::streamsize >::max(), '}');
                    thing->push_back( sphere );
                    if ( single )
                    {
                        scene.things.push_back( thing );
                        thing = NULL;
                    }
                    ++objectNumber;
                    if ( modeFlags.verbose )
                        std::cerr << "OK." << std::endl;
                }
                else if ( token == "\"plane\":" )
                {
                    if ( light )
                        throw std::string( "a Light cannot contain thing parts" );
                    if ( modeFlags.verbose )
                        std::cerr << "parseScene: reading Plane from input scene description... ";
                    bool single = !thing;
                    if ( single )
                        thing = new Thing( &scene );
                    Plane* plane = new Plane( thing );
                    is >> *plane;
                    is.ignore(std::numeric_limits< std::streamsize >::max(), '}');
                    thing->push_back( plane );
                    if ( single )
                    {
                        scene.things.push_back( thing );
                        thing = NULL;
                    }
                    ++objectNumber;
                    if ( modeFlags.verbose )
                        std::cerr << "OK." << std::endl;
                }
                else if ( token == "\"triangle\":" )
                {
                    if ( light )
                        throw std::string( "a Light cannot contain thing parts" );
                    if ( modeFlags.verbose )
                        std::cerr << "parseScene: reading Triangle from input scene description... ";
                    bool single = !thing;
                    if ( single )
                        thing = new Thing( &scene );
                    Triangle* triangle = new Triangle( thing );
                    is >> *triangle;
                    is.ignore(std::numeric_limits< std::streamsize >::max(), '}');
                    thing->push_back( triangle );
                    if ( single )
                    {
                        scene.things.push_back( thing );
                        thing = NULL;
                    }
                    ++objectNumber;
                    if ( modeFlags.verbose )
                        std::cerr << "OK." << std::endl;
                }
                else if ( token == "\"sky\":" )
                {
                    if ( skyDefined )
                        throw std::string( "the scene file has multiple Skies defined; please specify at most one Sky instead" );
                    skyDefined = true;
                    if ( modeFlags.verbose )
                        std::cerr << "parseScene: reading Sky from input scene description... ";
                    is >> scene.sky;
                    is.ignore(std::numeric_limits< std::streamsize >::max(), '}');
                    if ( modeFlags.verbose )
                        std::cerr << "OK." << std::endl;
                }
                else if ( token == "]" )
                {
                    if      ( light )
                    {
                        scene.lights.push_back( light );
                        light = NULL;
                        ++objectNumber;
                        is.ignore(std::numeric_limits< std::streamsize >::max(), '}');
                        token.clear();
                        if ( modeFlags.verbose )
                            std::cerr << "parseScene: end of Light." << std::endl;
                    }
                    else if ( thing )
                    {
                        scene.things.push_back( thing );
                        thing = NULL;
                        ++objectNumber;
                        is.ignore(std::numeric_limits< std::streamsize >::max(), '}');
                        token.clear();
                        if ( modeFlags.verbose )
                            std::cerr << "parseScene: end of Thing." << std::endl;
                    }
                    else
                        throw std::string( "unexpected ']' token" );
                }
                else if ( token == "]," || token == "," )
                {
                }
                else
                    throw std::string( "unrecognized token in Scene description: " + token );
            }
            catch( std::string e )
            {
                // Propagate exception to main
                std::stringstream ss;
                ss << "in object #" << objectNumber << ": " << e;
                throw ss.str();
            }
        }
        return is;
    }

}

