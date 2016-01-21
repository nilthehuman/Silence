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

// A parser for importing JSON motion descriptions.
// The motion description determines how Objects move around in the Scene
// Part of Silence, an experimental rendering engine

#include <limits>

#include "../core/scene.h"
#include "../core/triplet.h"

#include "../gui/motion.h"

namespace Silence {

    // Read a single Motion from character stream
    bool readMotion( std::istream& is, std::vector< Motion* >& motions, const Scene* scene )
    {
        Object*   object = NULL;
        int       type   = -1;
        double    scale;
        Vector    delta;
        double    stop   = -1;
        WorldAxis axis   = INVALID;
        double    period;
        Vector    begin;
        Vector    end;

        std::string token;
        while( !is.eof() )
        {
            is >> std::ws >> token;
            if     ( token[0] == '{' )
            {
                continue;
            }
            else if( token[0] == '}' )
            {
                break;
            }
            else if( token[0] == ']' )
            {
                return false;
            }
            else if( token == "\"light\":" )
            {
                int lightNumber;
                is >> lightNumber;
                object = *(scene->lightsBegin() + lightNumber);
            }
            else if( token == "\"thing\":" )
            {
                int thingNumber;
                is >> thingNumber;
                object = *(scene->thingsBegin() + thingNumber);
            }
            else if( token == "\"type\":" )
            {
                is >> token;
                if     ( token.substr(0, 10) == "\"brownian\""    ) type = 0;
                else if( token.substr(0,  8) == "\"linear\""      ) type = 1;
                else if( token.substr(0, 10) == "\"orbiting\""    ) type = 2;
                else if( token.substr(0, 13) == "\"oscillating\"" ) type = 3;
                else throw std::string( "unknown Motion type: " + token );
            }
            else if( token == "\"scale\":" )
            {
                is >> scale;
            }
            else if( token == "\"delta\":" )
            {
                is >> delta;
            }
            else if( token == "\"stop\":" )
            {
                is >> stop;
            }
            else if( token == "\"axis\":" )
            {
                is >> token;
                if     ( token.substr(0, 3) == "\"x\"" || token.substr(0, 3) == "\"X\"" ) axis = AXIS_X;
                else if( token.substr(0, 3) == "\"y\"" || token.substr(0, 3) == "\"Y\"" ) axis = AXIS_Y;
                else if( token.substr(0, 3) == "\"z\"" || token.substr(0, 3) == "\"Z\"" ) axis = AXIS_Z;
                else throw std::string( "invalid axis: " + token );
            }
            else if( token == "\"period\":" )
            {
                is >> period;
            }
            else if( token == "\"begin\":" )
            {
                is >> begin;
            }
            else if( token == "\"end\":" )
            {
                is >> end;
            }
            else if( token == "," )
            {
            }
            else
                throw std::string( "unrecognized token in motion: '" + token + "'" );
        }

        if( -1 == type )
            throw std::string( "each Motion must have a type" );

        switch( type )
        {
            case 0:
            {
                BrownianMotion* motion = new BrownianMotion( object, scale );
                motions.push_back( motion );
                break;
            }
            case 1:
            {
                LinearMotion* motion = new LinearMotion( object, delta, stop );
                motions.push_back( motion );
                break;
            }
            case 2:
            {
                OrbitingMotion* motion = new OrbitingMotion( object, axis, period );
                motions.push_back( motion );
                break;
            }
            case 3:
            {
                OscillatingMotion* motion = new OscillatingMotion( object, begin, end, period );
                motions.push_back( motion );
                break;
            }
            default:
                assert( false );
        }
        return true;
    }

    // Read and parse a list of Motion descriptions
    std::vector< Motion* > parseMotions( std::istream& is, const Scene* scene )
    {
        std::vector< Motion* > motions;

        int jsonDepth = 0;
        while( !is.eof() )
        {
            std::string token;
            is >> std::ws >> token;
            if     ( token == "{" )
            {
                if ( ++jsonDepth == 2 )
                    std::cerr << "parseMotions: warning: Motions should be defined in the top level JSON object" << std::endl;
            }
            else if( token == "}" )
            {
                if ( --jsonDepth < 0 )
                    std::cerr << "parseMotions: warning: ignoring unexpected '}' character in motions file" << std::endl;
            }
            else if( token == "\"motions\":" )
            {
                is.ignore(std::numeric_limits< std::streamsize >::max(), '[');
                while( readMotion(is, motions, scene) );
            }
            else if( token.empty() )
            {
                // The file had some trailing whitespace, but we've reached the end
                assert( is.eof() );
                break;
            }
            else
                throw std::string( "unrecognized token in motions description: '" + token + "'" );
        }
        return motions;
    }

}

