// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "application.h"

void Application::handleQuitAction() {
    Q_EMIT appQuit();
}

Application* Application::m_instance = nullptr;

Application* Application::getInstance(int &argc, char **argv) {
    return m_instance = (m_instance == nullptr ? new Application(argc, argv) : m_instance);
}
