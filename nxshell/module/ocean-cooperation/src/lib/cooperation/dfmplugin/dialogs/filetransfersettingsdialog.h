// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILETRANSFERSETTINGSDIALOG_H
#define FILETRANSFERSETTINGSDIALOG_H

#include <DDialog>
#include <DSuggestButton>
#include <DComboBox>

#include <QLabel>
#include <QVBoxLayout>

namespace dfmplugin_cooperation {

class FileChooserEdit : public QWidget
{
    Q_OBJECT
public:
    explicit FileChooserEdit(QWidget *parent = nullptr);

    void setText(const QString &text);

Q_SIGNALS:
    void fileChoosed(const QString &fileName);

protected:
    void paintEvent(QPaintEvent *event) override;

private Q_SLOTS:
    void onButtonClicked();

private:
    void initUI();

    QLabel *pathLabel {nullptr};
    DTK_WIDGET_NAMESPACE::DSuggestButton *fileChooserBtn { nullptr };
};

class BackgroundWidget : public QFrame
{
    Q_OBJECT

public:
    enum RoundRole {
        NoRole,
        Top,
        Bottom
    };

    explicit BackgroundWidget(QWidget *parent = nullptr);

    void setRoundRole(RoundRole role);

protected:
    void paintEvent(QPaintEvent *event) override;

protected:
    RoundRole role = NoRole;
};

class FileTransferSettingsDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT

public:
    explicit FileTransferSettingsDialog(QWidget *parent = nullptr);

public Q_SLOTS:
    void onFileChoosered(const QString &fileName);
    void onComBoxValueChanged(int index);

protected:
    void showEvent(QShowEvent *e) override;

private:
    void initUI();
    void initConnect();
    void loadConfig();
    // indexPos: 0-first, 1-last, 2-mid
    void addItem(const QString &text, QWidget *widget, int indexPos);

private:
    FileChooserEdit *fileChooserEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DComboBox *comBox { nullptr };
    QVBoxLayout *mainLayout { nullptr };
};

}   // namespace dfmplugin_cooperation

#endif   // FILETRANSFERSETTINGSDIALOG_H
