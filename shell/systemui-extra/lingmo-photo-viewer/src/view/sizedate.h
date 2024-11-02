#ifndef SIZEDATE_H
#define SIZEDATE_H
#include <QString>
#include <QSize>
#include <QColor>
#include <QImage>
/*struct MarkPainterSet
{
    QPoint recordStartPos; //记录开始位置--标注
    QPoint recoedMovePos;  //记录移动位置--标注
    int type = -1;         //记录画笔类型--标注：0，1，2，3，4，5，6，7
    int thickness = -1;    //记录画笔粗细--标注：
    int painterColor = -1; //记录画笔颜色--标注
    //文字设置保存
    struct TextSet
    {
        bool isBold = false;
        bool isDeleteLine = false;
        bool isUnderLine = false;
        bool isItalics = false;
        QString fontType = "";
        int fontSize = 6;
    } m_textSet;
};
Q_DECLARE_METATYPE(MarkPainterSet) */                            // QVarant 注册自定义类型
static const int DEFAULT_WIDTH = 1080;                         //主界面默认宽度
static const int DEFAULT_HEIGHT = 720;                         //主界面默认高度
static const int DEFAULT_HORIZONTAL_WIDTH = 1920;              //主界面横屏默认宽度
static const int DEFAULT_HORIZONTAL_HEIGHT = 1080;             //主界面横屏默认高度
static const int DEFAULT_VERTICAL_WIDTH = 1080;                //主界面竖屏默认宽度
static const int DEFAULT_VERTICAL_HEIGHT = 1920;               //主界面竖屏默认高度
static const QSize MINI_SIZE = QSize(726, 600);                //主界面最小尺寸
static const QSize MINI_SIZE_INTEL = QSize(866, 678);          //主界面最小尺寸
static const QSize LOGO_BTN = QSize(24, 24);                   //标题栏logo按钮尺寸
static const QSize ICON_SIZE = QSize(30, 30);                  //图标大小
static const QSize OPEN_IMAGESIZE = QSize(350, 350 + 50 + 10); //打开图片界面尺寸
static const QSize OPENINCENTER_SIZE = QSize(128, 128); //打开图片界面--中间打开圆形图标按钮尺寸
static const QSize ADD_FILESIZE = QSize(40, 40);        //打开图片界面--中间打开加号图标按钮尺寸
static const int OPEN_IMAGEFIX = 350;                   //打开图片界面的固定值
static const QSize NAVI_SIZE = QSize(200, 40);          //导航栏尺寸
static const QSize INFOR_SIZE = QSize(207 + 6, 197 + 18 + 20 + 8); //信息栏尺寸

static const QSize MARK_SIZE = QSize(558, 48);     //标注栏尺寸
static const QSize MARK_BUTTON = QSize(24, 24);    //标注栏尺寸
static const QSize MARK_SIZE_TM = QSize(760, 64);  //标注栏尺寸
static const QSize MARK_BUTTON_TM = QSize(48, 48); //标注栏尺寸
//标注-界面-pc
static const QSize BRUSHSETTING_SIZE = QSize(308 * 48 / 37 + 4, 48);     //画笔设置弹窗尺寸
static const QSize BRUSHSETTINGTEXT_SIZE = QSize(410 * 48 / 37 + 4, 48); //画笔设置弹窗--字体设置窗口尺寸
static const int TRIANGLE_WIDTH = 15;                                    // 画笔设置弹窗小三角的宽度;
static const int TRIANGLE_HEIGHT = 7;                                    // 画笔设置弹窗小三角的高度;
static const int TRIANGLE_POINTY = 41;
static const int BORDER_RADIUS = 6;                                             // 画笔设置弹窗边角的弧度;
static const int TRIANGLE_STARTX = 95;                                          // 弹窗小三角起始位置
static const QRect BRUSHSETTING_PAINT = QRect(0, 0, 308 * 48 / 37 + 4, 41);     //画笔设置弹窗rect
static const QRect BRUSHSETTINGTEXT_PAINT = QRect(0, 0, 410 * 48 / 37 + 4, 41); //画笔设置弹窗--字体设置rect
static const int CONTROL_INTERVAL = 4;                                          //画笔设置弹窗控件间隔
static const int POSITION_COEFFICIENT = 0;                                      //起始位置系数

//标注-界面-TM
static const QSize BRUSHSETTING_SIZE_TM = QSize(308 * 65 / 37 + 4, 65);     //画笔设置弹窗尺寸
static const QSize BRUSHSETTINGTEXT_SIZE_TM = QSize(405 * 65 / 37 + 4, 65); //画笔设置弹窗--字体设置窗口尺寸
static const int TRIANGLE_WIDTH_TM = 25;                                    // 画笔设置弹窗小三角的宽度;
static const int TRIANGLE_HEIGHT_TM = 10;                                   // 画笔设置弹窗小三角的高度;
static const int TRIANGLE_POINTY_TM = 55;                                   // 画笔设置弹窗边角的弧度;
static const int BORDER_RADIUS_TM = 6;                                      // 画笔设置弹窗边角的弧度;
static const int TRIANGLE_STARTX_TM = 100;                                  // 弹窗小三角起始位置
static const QRect BRUSHSETTING_PAINT_TM = QRect(0, 0, 308 * 65 / 37 + 4, 55);     //画笔设置弹窗rect
static const QRect BRUSHSETTINGTEXT_PAINT_TM = QRect(0, 0, 405 * 65 / 37 + 4, 55); //画笔设置弹窗--字体设置rect
static const int CONTROL_INTERVAL_TM = 4;                                          //画笔设置弹窗控件间隔
static const int POSITION_COEFFICIENT_TM = 9;                                      //起始位置系数

static const QSize TOOLBAR_SIZE_INTEL = QSize(700 + 4 + 40 + 16 + 40 + 40 + 25, 64); //工具栏尺寸-intel
static const QSize TOOLBAR_SIZE = QSize(554 + 4 + 40 + 16 + 40 + 16 + 40 + 16, 40 + 4);   //工具栏尺寸
static const QSize TOOL_BUTTON_INTEL = QSize(48, 48);                                //工具栏按钮尺寸-intel
static const QSize TOOL_BUTTON = QSize(24, 24);                                      //工具栏按钮尺寸
static const QSize COLOR_BUTTON = QSize(26, 26);                                     //颜色按钮尺寸
static const QSize PICTURE_BUTTON = QSize(64, 64);                                   //图片按钮尺寸
static const QSize TOOLZOOM_SIZE_INTEL = QSize(175, 48); //工具栏缩放widget尺寸 (123,24) -intel
static const QSize TOOLZOOM_SIZE = QSize(133, 24);       //工具栏缩放widget尺寸
static const int TOOLBAR_HEIGHT_INTEL = 72;              // intel
static const int BAR_HEIGHT_INTEL = 48;                  //标题栏和工具栏的高度-intel
static const int BAR_HEIGHT = 40;                        //标题栏和工具栏的高度
static const QSize TITLE_BTN_INTEL = QSize(48, 48);      //标题栏按钮尺寸-intel
static const QSize TITLE_BTN = QSize(32, 32);            //标题栏按钮尺寸
static const QSize MBTN_SIZE_INTEL = QSize(48, 48);      //菜单栏按钮尺寸--intel
static const QSize MBTN_SIZE = QSize(32, 32);            //菜单栏按钮尺寸

static const QSize TOOL_PER = QSize(45 + 29, 22);          //工具栏缩放率尺寸
static const QSize TOOL_LINE = QSize(1, 22);               //工具栏两条线的尺寸
static const QColor TOOL_COLOR = QColor(0, 0, 0, 80);      //工具栏阴影颜色(54,54,54)
static const int BLUR_RADIUS = 10;                         //工具栏模糊度
static const QColor ALBUM_COLOR = QColor(1, 1, 1, 32);  //相册阴影颜色
static const int ALBUM_RADIUS = 10;                        //相册阴影模糊度
static const QSize IMAGE_BUTTON = QSize(56, 56);           //图片界面按钮尺寸
static const QSize IMAGE_ICON = QSize(56, 56);             //图片界面icon尺寸
static const int LEFT_POS = 43;                            //图片界面按钮距边距尺寸
static const QSize MICON_SIZES = QSize(16, 16);            //菜单栏图标尺寸-小
static const QSize MICON_SIZEM = QSize(24, 24);            //菜单栏图标尺寸-中
static const QSize MICON_SIZEB = QSize(96, 96);            //菜单栏图标尺寸-大
static const QSize M_ABOUT = QSize(420, 390);              //菜单-关于界面尺寸
static const int NAME_HEIGHT = 28;                         //菜单-关于界面appname高度
static const int VERSION_HEI = 30;                         //菜单-关于界面AppVersion高度
static const int DESCRIBE_HEI = 70;                        //菜单-关于界面app描述高度
static const QString VERSION_NEM = "1.2.0";                //菜单-关于界面版本号
static const QString USER_GUIDE = "pictures";              //菜单-关于界面用户手册
static const QSize SIDEBAR_SIZE = QSize(126, 590);         //相册尺寸
static const QSize SIDEITEM_SIZE = QSize(94 + 12, 56 + 8); //相册item尺寸
static const QSize SIDEBAR_TABLET_SIZE = QSize(1080, 128); //相册尺寸
static const QSize SIDEITEM_TABLET_SIZE = QSize(96, 96);   //平板侧栏item的尺寸

static const double DEFAULT_FONT_SIZE = 11; //字体默认字号
static const double ABOUT_FONT_SIZE = 14;   //字体默认字号
static const int DEFAULT_FONT = 11;         //字体默认字号
static const int CUT_DISTANCE = 8;          //字体默认字号
static const QStringList SPECIALCHAR = {"?", "/", "\"", "*", ">", "<", "|", "\\", ":"};
static const int WIDGETSIZE_MAX = 16777215; //删除固定窗口约束，需设置窗口size为此值
static const int TITLE_TRANSPARENT = 178;   //标题栏透明度
static QImage tempImage = QImage();
static QColor tempColor = QColor(255, 255, 255, 0);
static QStringList platForm = {"pc"};                            //当前模式 /*"V10SP1-edu"*/"normal"
static const QStringList EXTERNAL_START_APP = {"lingmo-gallery"}; //外部启动的应用名
static const QString SIGN_APP_NAME = "kolourpaint";
static int recordCurrentProportion = 100; //记录当前图片缩放比例，默认为100
static const QStringList SIGN_APP_NOT_SUPPORT = {"ras", "tga", "sr",  "jng",
                                                 "jp2", "psd", "j2k", "apng"}; //画图软件不支持的格式
static const QStringList MOVIE_IMAGE_FORMAT = {"apng", "gif"};                 //动图
static const QStringList OCR_SUPPORT = {"png", "jpg", "jpeg"};                 // ocr支持的格式

// qml方法和属性名
static const QString IMAGE_SOURCE = "source";
static const QString IMAGE_X = "x";
static const QString IMAGE_Y = "y";
static const QString IMAGE_WIDTH = "width";
static const QString IMAGE_HEIGHT = "height";
static const QString IMAGE_TYPE = "imageType";
static const QString IMAGE_NEW_LOADED = "newImageLoaded";
static const QString IMAGE_COLOR = "color";

static const QString IMAGE_FUNC_OPERATE = "operateImage";
static const QString IMAGE_FUNC_CHANGESCALE = "changeScale";
static const QString IMAGE_FUNC_POSRESET = "imagePostionReset";
static const QString IMAGE_FUNC_SENDSCALE = "sendScaleFirstTime";
static const QString IMAGE_FUNC_SETSOURCE = "setSource";
static const QString IMAGE_FUNC_GETLEFTUPPOSACCORDWID_X = "getLeftUpPosAccordingWidX";
static const QString IMAGE_FUNC_GETLEFTUPPOSACCORDWID_Y = "getLeftUpPosAccordingWidY";
static const QString IMAGE_FUNC_GETVISIBLESIZEACCORDWID_W = "getVisibleSizeAccordingWidW";
static const QString IMAGE_FUNC_GETVISIBLESIZEACCORDWID_H = "getVisibleSizeAccordingWidH";

static const QString IMAGE_FUNC_GETLEFTUPPOSACCORDIMAGE_X = "getLeftUpPosAccordingImageX";
static const QString IMAGE_FUNC_GETLEFTUPPOSACCORDIMAGE_Y = "getLeftUpPosAccordingImageY";

static const QString IMAGE_FUNC_NAVICHANGEIMAGEPOS = "operateNavigatorChangeImagePos";

static const QString IMAGE_FUNC_ADJUSTIMAGEPOS = "adjustImagePostion";
static const QString IMAGE_FUNC_RELOADIMAGE = "reloadImage";

static const QString IMAGE_FUNC_UPDATE_POS_ACCORD_WIDSIZE_CHANGE = "updatePosAccordingToWidSizeChange";

//标注
static const QString IMAGE_FUNC_SIGN_GETOPERATETIME = "getOperateTime";
static const QString IMAGE_FUNC_SIGN_EXITSIGN = "exitOperate";
// static const QStringList SIGN_NOT_SUPPORT = {"apng", "gif", "wbmp", "tiff", "tif", "sr",
//                                             "exr",  "pgm", "ras",  "pbm",  "svg", "xbm"};
static const QStringList SIGN_NOT_SUPPORT = {"apng", "gif", "tiff", "tif", "svg", "xbm"};
static const QString IMAGE_FUNC_MULTITIFF_NAVI_STATE = "returnNavigatorNeedShow";
static const int TOUCH_ONE_POINT = 1; // 单指触控

//抠图qml方法和属性名
static QImage mattingTempImage = QImage();
static int recordMattingCurrentProportion = 100;            //记录当前抠图图片缩放比例，默认为100
static const QString MATTINGIMAGE_FUNC_SENDSCALE = "sendMattingImageScale";
static const QString MATTINGIMAGE_FUNC_POSRESET = "mattingImagePostionReset";
static const QString MATTINGIMAGE_FUNC_CHANGESCALE = "changeMattingScale";
static const QString MATTINGIMAGE_FUNC_OPERATEIMAGE = "mattingOperateImage";
static const QString MATTINGIMAGE_FUNC_GETOPERATEWAY = "getOperateImageWay";
static const QString MATTINGIMAGE_FUNC_RECORDCURRENTFLIP = "comparePressedShowImage";
static const QString MATTINGIMAGE_FUNC_SETCURRENTFLIP = "compareReleasedShowImage";
static const QString MATTINGIMAGE_FUNC_ADJUSTIMAGEPOS = "adjustMattingImagePostion";
static const QString MATTINGIMAGE_FUNC_UPDATEPOSACCORDWIDSIZECHANGE = "updateMattingPosAccordingToWidSizeChange";
static const QString MATTINGIMAGE_FUNC_CLEAR = "mattingClear";
static const QString MATTINGIMAGE_FUNC_STARTCUT = "startCut";
static const QString MATTINGIMAGE_FUNC_QUITCUT = "quitCut";
static const QString MATTINGIMAGE_FUNC_SETCUTSIZE = "setCutSize";
static const QString MATTINGIMAGE_FUNC_SETCUTPOS = "setCutPos";
static const QString MATTINGIMAGE_FUNC_SETCUTSCALE = "setCutScale";
static const QString MATTINGIMAGE_FUNC_GETCUTINFO_X = "getCutInfoX";
static const QString MATTINGIMAGE_FUNC_GETCUTINFO_Y = "getCutInfoY";
static const QString MATTINGIMAGE_FUNC_GETCUTINFO_W = "getCutInfoW";
static const QString MATTINGIMAGE_FUNC_GETCUTINFO_H = "getCutInfoH";
static const QString MATTINGIMAGE_FUNC_CHECKCUTSIZE = "checkCutSize";
static const QString MATTINGIMAGE_FUNC_GETROTATETO = "getRotateTo";

//扫描 - qml方法和属性名
static QImage scannerTempImage = QImage();
static int recordScannerCurrentProportion = 100;            //记录当前扫描图片缩放比例，默认为100
static const QString SCANNERIMAGE_FUNC_SENDSCALE = "sendScannerImageScale";
static const QString SCANNERIMAGE_FUNC_POSRESET = "scannerImagePostionReset";
static const QString SCANNERIMAGE_FUNC_CHANGESCALE = "changeScannerScale";
static const QString SCANNERIMAGE_FUNC_OPERATEIMAGE = "scannerOperateImage";
static const QString SCANNERIMAGE_FUNC_GETLEFTUPPOSACCORDIMAGE_X = "getScannerLeftUpPosAccordingImageX";
static const QString SCANNERIMAGE_FUNC_GETLEFTUPPOSACCORDIMAGE_Y = "getScannerLeftUpPosAccordingImageY";
static const QString SCANNERIMAGE_FUNC_GETOPERATEWAY = "getOperateImageWay";
static const QString SCANNERIMAGE_FUNC_RECORDCURRENTFLIP = "comparePressedShowImage";
static const QString SCANNERIMAGE_FUNC_SETCURRENTFLIP = "compareReleasedShowImage";
static const QString SCANNERIMAGE_FUNC_ADJUSTIMAGEPOS = "adjustScannerImagePostion";
static const QString SCANNERIMAGE_FUNC_UPDATEPOSACCORDWIDSIZECHANGE = "updateScannerPosAccordingToWidSizeChange";
static const QString SCANNERIMAGE_FUNC_STARTCUT = "startCut";
static const QString SCANNERIMAGE_FUNC_QUITCUT = "quitCut";
static const QString SCANNERIMAGE_FUNC_GETCUTINFO_X = "getCutInfoX";
static const QString SCANNERIMAGE_FUNC_GETCUTINFO_Y = "getCutInfoY";
static const QString SCANNERIMAGE_FUNC_GETCUTINFO_W = "getCutInfoW";
static const QString SCANNERIMAGE_FUNC_GETCUTINFO_H = "getCutInfoH";
static const QString SCANNERIMAGE_FUNC_CHECKCUTSIZE = "checkCutSize";
static const QString SCANNERIMAGE_FUNC_SETCUTSIZE = "setCutSize";
static const QString SCANNERIMAGE_FUNC_GETROTATETO = "getRotateTo";
#endif // SIZEDATE_H
