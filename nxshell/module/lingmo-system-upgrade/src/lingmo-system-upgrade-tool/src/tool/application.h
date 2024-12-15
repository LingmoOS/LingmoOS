// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DApplication>
#include <DApplicationSettings>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

static int kDefaultArgc = 0;

class Application : public DApplication
{
    Q_OBJECT
public:
    static Application* getInstance(int &argc = kDefaultArgc, char **argv = nullptr);

Q_SIGNALS:
    void appQuit();

protected:
    virtual void handleQuitAction();
    static Application *m_instance;

private:
    Application(int &argc, char **argv = nullptr) : DApplication(argc, argv) {}
};
