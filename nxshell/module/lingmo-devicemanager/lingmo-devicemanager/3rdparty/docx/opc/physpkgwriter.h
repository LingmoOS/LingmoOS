// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PHYSPKGWRITER_H
#define PHYSPKGWRITER_H

#include <QByteArray>
#include <QList>

class QZipWriter;
class QIODevice;

namespace Docx {
class PhysPkgWriter
{
public:
    PhysPkgWriter(const QString &filePath);
    PhysPkgWriter(QIODevice* device);
    void write(const QString &packUri, const QByteArray &blob);
    void close();

    ~PhysPkgWriter();

private:
    QList<QString> m_paths;
    QZipWriter *m_writer;
};
}
#endif // PHYSPKGWRITER_H
