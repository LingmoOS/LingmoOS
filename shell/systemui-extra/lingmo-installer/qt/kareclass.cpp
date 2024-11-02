#include "kareclass.h"
#include "adaptscreeninfo.h"
kareClass::kareClass(QWidget *parent) : QWidget(parent)
{

    this->setFixedSize(300,300);
    isHaveKare();

    adaptScreenInfo adapt;
    int x = adapt.m_nScreen_x;
    int y = adapt.m_nScreen_y;
    int width = adapt.m_screenWidth;
    int height = adapt.m_screenHeight;
    //                MotifWmHints hints;
    //                hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    //                hints.functions = MWM_FUNC_ALL;
    //                hints.decorations = MWM_DECOR_BORDER3
    //                XAtomHelper::getInstance()->setWindowMotifHint(w->winId(), hints);

    kdk::LingmoUIStyleHelper::self()->removeHeader(this);

    this->move((width / 2 + x) - this->width() / 2 , (height / 2 + y )- this->height() / 2);


    // 创建一个垂直布局管理器，用于管理单选框和确定按钮的位置
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addSpacing(10);

    QLabel *label = new QLabel(this);
    label->setText(tr("If a KARE environment is detected on the system, select the location where the software is installed"));
    label->setWordWrap(true);
    layout->addWidget(label);

    // 创建3个单选框，并添加到布局中
    QRadioButton *radioButton = new QRadioButton(tr("Current system"), this);
    layout->addWidget(radioButton);

    foreach (QString str, kareImageList) {
        QRadioButton *radioButton = new QRadioButton(this);
        radioButton->setText("KARE " + str);
        layout->addWidget(radioButton);
    }
    // 创建确定按钮，并添加到布局中
    okButton = new QPushButton(tr("OK"), this);
    layout->addWidget(okButton);
//    this->show();


    connect(okButton, &QPushButton::clicked, this ,&kareClass::onOkBtnClicked);

}


bool kareClass::isHaveKare()
{

    QStringList testList;
    testList << "baseimage_u16" << "baseimage_u18" << "baseimage_u20" << "baseimage_u22" << "baseimage_ok1"\
             << "baseimage_ok2" << "baseimage_v4" << "baseimage_v10" << "baseimage_v10sp1";


    kareImageList.clear();
    QProcess process;
    process.start("pkexec --user root docker images");
    process.waitForFinished(-1);
    QByteArray arr = process.readAll();
    //    qDebug() << "===============" << arr;
    QString str = QString(arr);
    QStringList result;
    if (str.contains("\n")){
        result = str.split("\n");
    }
    QStringList imageList;

    foreach (QString tmpStr, result) {
        QStringList tmpList;
        tmpStr = tmpStr.replace(QRegExp("\\s+"), " ");
        if (tmpStr.contains(" ")){
            tmpList = tmpStr.split(" ");
        }
        if (tmpList.size() > 0){
            if (tmpList.at(0).left(9) == "baseimage"){
                imageList << tmpList.at(0);
                switch (testList.indexOf(tmpList.at(0))) {
                case 0:
                    kareImageList << "ubuntu16.04";
                    break;
                case 1:
                    kareImageList << "ubuntu18.04";
                    break;
                case 2:
                    kareImageList << "ubuntu20.04";
                    break;
                case 3:
                    kareImageList << "ubuntu22.04";
                    break;
                case 4:
                    kareImageList << "lingmo1.0";
                    break;
                case 5:
                    kareImageList << "lingmo2.0";
                    break;
                case 6:
                    kareImageList << "v4";
                    break;
                case 7:
                    kareImageList << "v10";
                    break;
                case 8:
                    kareImageList << "v10sp1";
                    break;
                default:
                    break;
                }
            }
        }
    }
    qDebug() << kareImageList.size() << kareImageList;
    if (kareImageList.size() == 0){
        return false;
    }else
        return true;

    return false;
}



void kareClass::onOkBtnClicked(bool check)
{
    kareImage = "Local System";
    QList<QRadioButton*> radioButtons = this->findChildren<QRadioButton*>();
    foreach (QRadioButton *radioButton, radioButtons) {
        if (radioButton->isChecked()) {
            // 单选按钮已被选中，执行相应的操作
            qDebug() << radioButton->text() << " is selected.";
            if (radioButton->text().contains("KARE")){
                kareImage = radioButton->text().remove("KARE ");
            }
            if (!radioButton->text().compare(tr("Current system"))){
                kareImage = "Local System";
            }
        }
    }
    this->close();
}
