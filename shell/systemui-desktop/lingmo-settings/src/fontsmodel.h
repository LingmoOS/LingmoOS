/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#ifndef FONTSMODEL_H
#define FONTSMODEL_H

#include <QThread>

class FontsModel : public QThread
{
    Q_OBJECT
    Q_PROPERTY(QStringList generalFonts READ generalFonts NOTIFY loadFinished)
    Q_PROPERTY(QStringList fixedFonts READ fixedFonts NOTIFY loadFinished)
    Q_PROPERTY(QString systemGeneralFont READ systemGeneralFont NOTIFY loadFinished)
    Q_PROPERTY(QString systemFixedFont READ systemFixedFont NOTIFY loadFinished)

public:
    explicit FontsModel(QObject *parent = nullptr);

    void run() override;

    QStringList generalFonts() const;
    QStringList fixedFonts() const;

    QString systemGeneralFont() const;
    QString systemFixedFont() const;

signals:
    void loadFinished();

private:
    QStringList m_generalFonts;
    QStringList m_fixedFonts;
    QString m_lanCode;
};

#endif // FONTSMODEL_H
