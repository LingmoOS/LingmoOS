#ifndef oceantileset_h
#define oceantileset_h

/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/


#include <QPixmap>
#include <QRect>
#include <QVector>

//* handles proper scaling of pixmap to match widget rect.
/**
tilesets are collections of stretchable pixmaps corresponding to a given widget corners, sides, and center.
corner pixmaps are never stretched. center pixmaps are
*/
namespace Ocean
{
    class TileSet
    {
        public:
        /**
        Create a TileSet from a pixmap. The size of the bottom/right chunks is
        whatever is left over from the other chunks, whose size is specified
        in the required parameters.

        @param w1 width of the left chunks
        @param h1 height of the top chunks
        @param w2 width of the not-left-or-right chunks
        @param h2 height of the not-top-or-bottom chunks
        */
        TileSet(const QPixmap&, int w1, int h1, int w2, int h2 );

        //* empty constructor
        TileSet();

        //* destructor
        virtual ~TileSet()
        {}

        /**
        Flags specifying what sides to draw in ::render. Corners are drawn when
        the sides forming that corner are drawn, e.g. Top|Left draws the
        top-center, center-left, and top-left chunks. The center-center chunk is
        only drawn when Center is requested.
        */
        enum Tile {
            Top = 0x1,
            Left = 0x2,
            Bottom = 0x4,
            Right = 0x8,
            Center = 0x10,
            TopLeft = Top|Left,
            TopRight = Top|Right,
            BottomLeft = Bottom|Left,
            BottomRight = Bottom|Right,
            Ring = Top|Left|Bottom|Right,
            Horizontal = Left|Right|Center,
            Vertical = Top|Bottom|Center,
            Full = Ring|Center
        };
        Q_DECLARE_FLAGS(Tiles, Tile)

        /**
        Fills the specified rect with tiled chunks. Corners are never tiled,
        edges are tiled in one direction, and the center chunk is tiled in both
        directions. Partial tiles are used as needed so that the entire rect is
        perfectly filled. Filling is performed as if all chunks are being drawn.
        */
        void render(const QRect&, QPainter*, Tiles = Ring) const;

        //* return size associated to this tileset
        QSize size() const
        { return QSize( _w1 + _w3, _h1 + _h3 ); }

        //* is valid
        bool isValid() const
        { return _pixmaps.size() == 9; }

        //* returns pixmap for given index
        QPixmap pixmap( int index ) const
        { return _pixmaps[index]; }

        protected:

        //* shortcut to pixmap list
        using PixmapList = QVector<QPixmap>;

        //* initialize pixmap
        void initPixmap( PixmapList&, const QPixmap&, int w, int h, const QRect& );

        private:

        //* pixmap arry
        PixmapList _pixmaps;

        // dimensions
        int _w1;
        int _h1;
        int _w3;
        int _h3;

    };

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Ocean::TileSet::Tiles)

#endif //TILESET_H
