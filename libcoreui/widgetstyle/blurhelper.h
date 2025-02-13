//////////////////////////////////////////////////////////////////////////////
// breezeblurhelper.h
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

#ifndef BLURHELPER_H
#define BLURHELPER_H

#include <QObject>

class BlurHelper : public QObject
{
    Q_OBJECT

public:
    explicit BlurHelper(QObject *parent = nullptr);

    void registerWidget(QWidget *);
    void unregisterWidget(QWidget *);
    bool eventFilter(QObject *, QEvent *) override;

    void update(QWidget *) const;

    void enableBlurBehind(QWidget *widget, bool enable = true, qreal windowRadius = 0.0);

protected:
    void addEventFilter(QObject *object) {
        object->removeEventFilter(this);
        object->installEventFilter(this);
    }

};

#endif // BLURHELPER_H
