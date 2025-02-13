/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCREENSHOTVIEW_H
#define SCREENSHOTVIEW_H

#include <QQuickView>

class ScreenshotView : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(bool ocrEnabled READ ocrEnabled NOTIFY ocrEnabledChanged)

public:
    explicit ScreenshotView(QQuickView *parent = nullptr);
    bool ocrEnabled() const;
    void start();
    void delay(int value);

    Q_INVOKABLE void quit();
    Q_INVOKABLE void saveFile(QRect rect);
    Q_INVOKABLE void copyToClipboard(QRect rect);
    Q_INVOKABLE void ocr(QRect rect);

    void removeTmpFile();

private:
    bool m_ocrEnabled;

signals:
    void refresh();
    void ocrEnabledChanged();
};

#endif // SCREENSHOTVIEW_H
