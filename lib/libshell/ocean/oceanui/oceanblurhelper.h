#ifndef oceanblurhelper_h
#define oceanblurhelper_h

//////////////////////////////////////////////////////////////////////////////
// oceanblurhelper.h
// handle regions passed to kwin for blurring
// -------------------
//
// Copyright (C) 2018 Alex Nemeth <alex.nemeth329@gmail.com>
//
// Largely rewritten from Oxygen widget style
// Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "ocean.h"
#include "oceanhelper.h"

#include <QHash>
#include <QSet>
#include <QObject>

namespace Ocean
{
    class BlurHelper: public QObject
    {
        Q_OBJECT

        public:

        //! constructor
        BlurHelper( QObject* );

        //! register widget
        void registerWidget( QWidget*, const bool isDolphin );

        //! register widget
        void unregisterWidget( QWidget* );

        //! event filter
        bool eventFilter( QObject*, QEvent* ) override;
        
        //! force update
        void forceUpdate( QWidget* widget )
        { if( widget->isWindow() ) update( widget ); }
        
        void setTranslucentTitlebar( bool value )
        { _translucentTitlebar = value; }

        protected:

        //! install event filter to object, in a unique way
        void addEventFilter( QObject* object )
        {
            object->removeEventFilter( this );
            object->installEventFilter( this );
        }
        
        //! handle blur region
        QRegion blurRegion (QWidget* widget) const;

        //! update blur regions for given widget
        void update( QWidget* ) const;
        
        private:
        
        bool _isDolphin = false;
        bool _translucentTitlebar = false;
            

    };

}

#endif
