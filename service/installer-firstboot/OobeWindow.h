#ifndef OOBEWINDOW_H
#define OOBEWINDOW_H

#include <QWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProcess>

class OobeWindow : public QWidget
{
    Q_OBJECT
public:
    explicit OobeWindow(QWidget *parent = nullptr);

private slots:
    void nextPage();
    void backPage();
    void applySettings();

private:
    void setupBackground();
    void fadeToPage(int index);
    QWidget* createCard();
    QWidget* createWelcomePage();
    QWidget* createUserPage();
    QWidget* createHostnamePage();
    QWidget* createConfirmPage();
    QWidget* createInstallPage();
    QWidget* createFinishedPage();
    void updateNavButtons();

    QStackedWidget *m_stack;
    QPushButton *m_backBtn;
    QPushButton *m_nextBtn;

    // User data
    QString m_username;
    QString m_password;
    QString m_hostname;

    // Pages
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_confirmPwdEdit;
    QLineEdit *m_hostnameEdit;

    QWidget *m_cardWidget;
    QLabel *m_progressLabel;
    QWidget *m_progressBar;
};

#endif
