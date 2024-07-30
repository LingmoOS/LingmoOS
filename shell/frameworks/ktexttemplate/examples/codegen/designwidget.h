/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include <QWidget>

#ifndef DESIGNWIDGET_H
#define DESIGNWIDGET_H

#include "ui_designwidget.h"

#include <KTextTemplate/Context>

class ArgsModel;
class MethodModel;

class DesignWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DesignWidget(QWidget *parent = nullptr);

    KTextTemplate::Context getContext();

    QString outputType() const;

Q_SIGNALS:
    void generateClicked(bool clicked);

private:
    void setInitialContent();
    void insertProperty(int row, const QString &type, const QString &name, bool readonly);
    void
    insertMethod(const QString &access, bool _virtual, const QString &type, const QString &name, bool _const, QList<QStringList> args = QList<QStringList>());

private Q_SLOTS:
    void setArgsRootIndex(const QModelIndex &index);

private:
    Ui::DesignWidget ui;

    MethodModel *m_methodModel;
    ArgsModel *m_argsModel;
};

#endif
