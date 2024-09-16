// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_OPTIONS_H
#define _DMR_OPTIONS_H

#include <QtCore>

namespace dmr {
class CommandLineManager: public QCommandLineParser {
public:
    static CommandLineManager& get();
    bool verbose() const;
    bool debug() const;
    QString openglMode() const;
    QString overrideConfig() const;
    
    QString dvdDevice() const;

private:
    CommandLineManager();
};

}

#endif /* ifndef _DMR_OPTIONS_H */
