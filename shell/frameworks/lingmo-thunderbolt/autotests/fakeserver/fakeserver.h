/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef FAKESERVER_H
#define FAKESERVER_H

#include <QString>

#include <memory>

class FakeServerException : public std::runtime_error
{
public:
    FakeServerException(const char *what)
        : std::runtime_error(what)
    {
    }
    FakeServerException(const QString &what)
        : std::runtime_error(what.toStdString())
    {
    }
};

class FakeManager;
class FakeServer
{
public:
    explicit FakeServer(const QString &file);
    explicit FakeServer();
    ~FakeServer();

    static void enableFakeEnv();

    FakeManager *manager() const;

private:
    std::unique_ptr<FakeManager> mManager;
};

#endif
