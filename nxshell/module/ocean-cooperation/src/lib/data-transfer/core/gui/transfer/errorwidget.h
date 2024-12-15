#ifndef ERRORWIDGET_H
#define ERRORWIDGET_H

#include <QFrame>
#include <QLabel>

enum ErrorType { networkError = 0, outOfStorageError };

class ErrorWidget : public QFrame
{
    Q_OBJECT
public:
    ErrorWidget(QWidget *parent = nullptr);
    ~ErrorWidget();

public slots:
    void backPage();
    void retryPage();
    void themeChanged(int theme);
    void setErrorType(ErrorType type, int size = 0);

private:
    void initUI();

private:
    QLabel *titleLabel = nullptr;
    QLabel *promptLabel = nullptr;

    QString internetError{ tr("Network Error") };
    QString transferError{ tr("Transfer interrupted") };
    QString internetErrorPrompt{
        tr("The network disconnected, transfer failed, please connect the network and try again")
    };
    QString transferErrorPromptUOS{ tr(
            "Insufficient space in UOS, please clear at least %1 GB and try again") };
    QString transferErrorPromptWin{ tr("Insufficient space in UOS, Please reserve enough space") };
};
#endif // ERRORWIDGET_H
