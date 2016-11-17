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

// A tree data structure to represent parent-child relationships between zones
// Part of Silence, an experimental rendering engine

#ifndef SILENCE_TREE
#define SILENCE_TREE

#include <vector>

namespace Silence {

    template< class T >
    class Tree {

        typedef class std::vector< Tree<T>* >::const_iterator TreeIt;

    private:
        Tree();

    public:
        Tree( T& value )
            : value( value )
            , children()
        { }

        ~Tree()
        {
            for ( TreeIt it = children.begin(); it != children.end(); it++ )
                delete *it;
        }

        T&     getValue()            { return value; }
        T&     operator*()           { return value; }
        TreeIt childrenBegin() const { return children.begin(); }
        TreeIt childrenEnd()   const { return children.end();   }

        int height() const;
        //int depth( const Tree<T>* node ) const;

        void addChild( Tree<T>* node ) { children.push_back( node ); }
        void clearChildren();

    private:
        T value;
        std::vector< Tree<T>* > children;
    };

}

#endif // SILENCE_TREE

