#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <QWidget>

class QComboBox;
class QPushButton;
class QLabel;
class RippleButton;  // 添加前向声明
class QStackedWidget;  // 添加 QStackedWidget 的前向声明

class WelcomePage : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);
    bool getIsInstallSelected() const { return isInstallSelected; }

signals:
    void languageSelected();
    void installOptionSelected();
    void tryOptionSelected();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    enum Page { LanguagePage, InstallPage };
    Page currentPage;
    
    RippleButton *nextButton;  // 只用于语言选择页面
    QLabel *welcomeLabel;
    QLabel *installWelcomeLabel;
    QWidget *installOption;
    QWidget *tryOption;
    QComboBox *languageCombo;
    bool isInstallSelected;
    
    void setupUI();
    void setupLanguagePage();
    void setupInstallPage();
    void updateOptionStyles();
    void switchToInstallPage();
    
    QStackedWidget *stackedWidget;
    QWidget *languagePage;
    QWidget *installPage;

private slots:
    void updateText();
    void onNextClicked();
};

#endif // WELCOMEPAGE_H 