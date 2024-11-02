#include "beforeturnoffhintwidget.h"
#include "ui_beforeturnoffhintwidget.h"
#include "config/config.h"

BeforeTurnOffHintWidget::BeforeTurnOffHintWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::beforeTurnOffHintWidget)
{
    ui->setupUi(this);
    QIcon warningIcon = QIcon::fromTheme("lingmo-dialog-warning");
    if (warningIcon.isNull())
        warningIcon = QIcon::fromTheme("dialog-warning");
    ui->iconLabel->setPixmap(warningIcon.pixmap(22,22));
    ui->cancelButton->setChecked(true);

    if(QGSettings::isSchemaInstalled("org.lingmo.style")){
        styleGSettings = new QGSettings("org.lingmo.style");
        double size = styleGSettings->get("system-font-size").toDouble();
        if (size == 10)
            ui->iconLabel->move(26,16);
        connect(styleGSettings,&QGSettings::changed,this,[=](QString key) {
            if (key == "systemFontSize") {
                double size = styleGSettings->get("system-font-size").toDouble();
                if (size == 10)
                    ui->iconLabel->move(26,16);
                else
                    ui->iconLabel->move(26,19);
                this->repaint();
            }
        });
    }

    initConnect();
}

BeforeTurnOffHintWidget::~BeforeTurnOffHintWidget()
{
    delete ui;
}

void BeforeTurnOffHintWidget::initConnect() {
    connect(ui->closeButton, &QPushButton::pressed, this, [=]() {
        emit buttonPressed(false);
    });
    connect(ui->cancelButton, &QPushButton::pressed, this, [=]() {
        emit buttonPressed(true);
    });
}
