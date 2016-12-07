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

// Shadow class methods
// Part of Silence, an experimental rendering engine

#include "shadow.h"

namespace Silence {

    double Shadow::occluded( const Vector& point ) const
    {
        if ( umbra.containsNew(point) )
            return 1;
        if ( penumbra.containsNew(point) )
        {
            double shade = 0.5;
            std::vector< Ray > innerEdges( umbra.getEdges() );
            std::vector< Ray > outerEdges( penumbra.getEdges() );
            std::vector< Ray > ipEdges;
            for ( std::vector< Ray >::const_iterator ie = innerEdges.begin(); ie != innerEdges.end(); ie++ )
                for ( std::vector< Ray >::const_iterator oe = outerEdges.begin(); oe != outerEdges.end(); oe++ )
                    if ( ie->getOrigin() == oe->getOrigin() )
                    {
                        const Vector middle = (ie->getDirection() + oe->getDirection()) * 0.5;
                        ipEdges.push_back( Ray(umbra.getScene(), ie->getOrigin(), middle) );
                        break;
                    }
            // Interpolate linearly (kind of)
            for ( int i = 0; i < PENUMBRAIP; ++i )
            {
                Beam interpolator( umbra.getScene(), umbra.getApex(), umbra.getSource(), NULL, umbra.getPivot(), ipEdges, RGB::Black, Beam::Zero );
                std::vector< Ray > newIpEdges;
                if ( interpolator.containsNew(point) )
                {
                    shade += pow( 0.5, i+2 );
                    for ( std::vector< Ray >::const_iterator ie = innerEdges.begin(); ie != innerEdges.end(); ie++ )
                        for ( std::vector< Ray >::const_iterator ipe = ipEdges.begin(); ipe != ipEdges.end(); ipe++ )
                            if ( ie->getOrigin() == ipe->getOrigin() )
                            {
                                const Vector middle = (ie->getDirection() + ipe->getDirection()) * 0.5;
                                newIpEdges.push_back( Ray(umbra.getScene(), ie->getOrigin(), middle) );
                                break;
                            }
                    outerEdges.clear();
                    for ( std::vector< Ray >::const_iterator ipe = ipEdges.begin(); ipe != ipEdges.end(); ipe++ )
                        outerEdges.push_back( *ipe );
                    ipEdges.clear();
                    for ( std::vector< Ray >::const_iterator nipe = newIpEdges.begin(); nipe != newIpEdges.end(); nipe++ )
                        ipEdges.push_back( *nipe );
                }
                else
                {
                    shade -= pow( 0.5, i+2 );
                    for ( std::vector< Ray >::const_iterator oe = outerEdges.begin(); oe != outerEdges.end(); oe++ )
                        for ( std::vector< Ray >::const_iterator ipe = ipEdges.begin(); ipe != ipEdges.end(); ipe++ )
                            if ( oe->getOrigin() == ipe->getOrigin() )
                            {
                                const Vector middle = (oe->getDirection() + ipe->getDirection()) * 0.5;
                                newIpEdges.push_back( Ray(umbra.getScene(), oe->getOrigin(), middle) );
                                break;
                            }
                    innerEdges.clear();
                    for ( std::vector< Ray >::const_iterator ipe = ipEdges.begin(); ipe != ipEdges.end(); ipe++ )
                        innerEdges.push_back( *ipe );
                    ipEdges.clear();
                    for ( std::vector< Ray >::const_iterator nipe = newIpEdges.begin(); nipe != newIpEdges.end(); nipe++ )
                        ipEdges.push_back( *nipe );
                }
            }
            return shade;
        }
        return 0;
    }

}

