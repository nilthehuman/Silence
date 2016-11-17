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

#include "tree.h"

namespace Silence {

    template< class T >
    int Tree<T>::height() const
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

    template< class T >
    void Tree<T>::clearChildren()
    {
        for ( TreeIt child = children.begin(); child != children.end(); ++child )
            delete *child;
        children.clear();
    }

}

