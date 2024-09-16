// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTENLABEL_H
#define SHORTENLABEL_H
#include <QLabel>
namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxShortenLabel : public QLabel
{
    Q_OBJECT
public:
    FcitxShortenLabel(const QString &text = "", QWidget *parent = nullptr);
    void setShortenText(const QString &text);
    QString text() const;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateTitleSize();

private:
    QString m_text;
};
} // namespace widgets
} // namespace dcc_fcitx_configtool
#endif // SHORTENLABEL_H
