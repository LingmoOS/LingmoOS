/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _KCM_FCITX_OPTIONWIDGET_H_
#define _KCM_FCITX_OPTIONWIDGET_H_

#include <QWidget>
#include <fcitxqtdbustypes.h>

class QFormLayout;

namespace fcitx {
namespace kcm {

class OptionWidget : public QWidget {
    Q_OBJECT
public:
    OptionWidget(const QString &path, QWidget *parent)
        : QWidget(parent), path_(path) {}

    static OptionWidget *addWidget(QFormLayout *layout,
                                   const fcitx::FcitxQtConfigOption &option,
                                   const QString &path, QWidget *parent);

    static bool execOptionDialog(QWidget *parent,
                                 const fcitx::FcitxQtConfigOption &option,
                                 QVariant &result);

    virtual void readValueFrom(const QVariantMap &map) = 0;
    virtual void writeValueTo(QVariantMap &map) = 0;
    virtual void restoreToDefault() = 0;
    virtual bool isValid() const { return true; }

    QString prettify(const FcitxQtConfigOption &option, const QVariant &value);

    const QString &path() const { return path_; }
Q_SIGNALS:
    void valueChanged();

private:
    QString path_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_OPTIONWIDGET_H_
