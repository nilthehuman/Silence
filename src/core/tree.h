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

#include <cassert>
#include <cstddef>
#include <vector>

namespace Silence {

    template< class T >
    class Tree {
    public:
        typedef class std::vector< Tree<T>* >::iterator TreeIt;

    private:
        Tree();

    public:
        Tree( T& value )
            : value( value )
            , parent( NULL )
            , children()
            , leaves()
        {
            leaves.push_back( this );
        }
        Tree( T& value, Tree<T>* parent )
            : value( value )
            , parent( parent )
            , children()
            , leaves()
        {
            assert( parent );
            leaves.push_back( this );
        }

        ~Tree()
        {
            for ( TreeIt it = children.begin(); it != children.end(); it++ )
                delete *it;
        }

        const Tree<T>* getParent() const { return parent; }
        T&             getValue()        { return value; }
        T&             operator*()       { return value; }
        TreeIt         childrenBegin()   { return children.begin(); }
        TreeIt         childrenEnd()     { return children.end();   }
        TreeIt         leavesBegin()     { return leaves.begin();   }
        TreeIt         leavesEnd()       { return leaves.end();     }

        int height() const
        {
            int height = 0;
            for ( TreeIt child = children.begin(); child != children.end(); ++child )
            {
                const int childHeight = (*child)->height();
                if( height < childHeight + 1 )
                    height = childHeight + 1;
            }
            return height;
        }
        //int depth( const Tree<T>* node ) const;

        Tree<T>* addChild( T& val )
        {
            Tree<T>* child = new Tree<T>( val, this );
            child->setParent( this );
            if ( children.empty() )
                leaves.clear(); // Remove self from leaves
            children.push_back( child );
            leaves  .push_back( child );
            if ( parent )
                parent->updateLeaves();
            return this;
        }
        Tree<T>* addChild( Tree<T>* child )
        {
            child->setParent( this );
            if ( children.empty() )
                leaves.clear(); // Remove self from leaves
            children.push_back( child );
            leaves  .push_back( child );
            if ( parent )
                parent->updateLeaves();
            return this;
        }

        void clearChildren()
        {
            for ( TreeIt child = children.begin(); child != children.end(); ++child )
                delete *child;
            children.clear();
            leaves.clear();
            leaves.push_back( this );
            if ( parent )
                parent->updateLeaves();
        }

    private:
        void setParent( Tree<T>* newParent ) { parent = newParent; }

        void updateLeaves()
        {
            leaves.clear();
            if ( children.empty() )
                leaves.push_back( this );
            else
                for ( TreeIt child = childrenBegin(); child != childrenEnd(); ++child )
                    for ( TreeIt leaf = (*child)->leavesBegin(); leaf != (*child)->leavesEnd(); ++leaf )
                        leaves.push_back( *leaf );
            if ( parent )
                parent->updateLeaves();
        }

    private:
        T                       value;
        Tree<T>*                parent;
        std::vector< Tree<T>* > children;
        std::vector< Tree<T>* > leaves;
    };

}

#endif // SILENCE_TREE

