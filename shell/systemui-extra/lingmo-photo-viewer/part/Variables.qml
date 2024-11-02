import QtQuick 2.12

Item {
    property string visibleItem: ""
    property point mousePos: Qt.point(-1, -1)
    property double curWidW: 0
    property double curWidH: 0
    property point mouseCurPos: Qt.point(-1, -1)
    property bool navigatorState: false
    //缩放比例常数
    property int scalingconstant: 100
    property int imageAniDuration: 300
    property int imageZoomSpeed: 20
    //4个角度
    property int angle90: 90
    property int angle180: 180
    property int angle270: 270
    property int angle360: 360
    property int operateImageWay:12
    property bool wheelChange: false
    property var signPaintList: []
    property string format: ""

    property int painterType : -1                //记录画笔类型
    property int painterThickness : 12            //记录画笔粗细
    property color painterColor : Qt.rgba(255,0,0)                 //记录画笔颜色
    property bool isBold : false;                   //加粗
    property bool isDeleteLine : false            //删除
    property bool isUnderLine :false             //下划线
    property bool isItalics : false                //斜体
    property string fontType : "Monospace"                 //字体
    property int fontSize : 12                      //字号

    property var objList:[]
    property int operateTime: 0

    property double mouseqX: 0
    property double mouseqY: 0
    property double acturalWidth: 0
    property double acturalHeight: 0
    property double acturalX: 0
    property double acturalY: 0
    property double acturalRotate: 0

    property bool painterRelease: false
    property bool coreNeedPaint: false
    property bool needExitPaint: false
    signal painterRectSignal(var rect)
    signal painterCircleSignal(var leftX, var leftY, var painterW, var painterH)
    signal painterLinePaint(var startPos, var endPos);
    signal painterArrowPaint(var startPos, var endPos, var arrowStartPos, var arrowEndPos);
    signal painterPencilPaint(var pointList)
    signal painterMarkPaint(var startPos, var endPos,var alpha)
    signal painterTextPaint(var textContent, var startPos, var type)
    signal painterBlurPaint(var blurRect)

    property bool createNewTextobj: false
    signal painterTextChanged(var mx,var my)
    property double textMouseX: -1000
    property double textMouseY: -1000

    //story 19807
    //记录历史信息，历史缩放比，历史位置的x，y值
    property double historyScale: 0
    property double historyX: 0
    property double historyY: 0

    //图片打开的初始信息，初始缩放比，初始位置的x，y值
    property double origScale: 1
    property double origX: 0
    property double origY: 0

    //图片上一次的path和当前的path
    property string historyPath: ""
    property string currentPath: ""

    //图片自适应
    property bool needRestoreOrig: true //是否需要回到自适应窗口的状态
    property bool restoreOrigToToolbar: true // //告诉c++前端需要使图片自适应窗口

    property bool doubleRestore: false //用来判断是否时双击触发的还原自适应
    property bool controlImageDoubleScale: false //用来判断是否时双击触发的2倍缩放
    property bool recordCurrentImageHasOperate: false //用来判断当前图片是否被操作过
    property point histortPos: Qt.point(-1,-1) //历史位置
    property point wheelVaule: Qt.point(-1,-1) //滚轮值-暂时没有用到
    property bool restoreToHistory: false//还原到历史状态
    property bool imageIsDrag: false //判断图片是否为拖拽状态
    property bool startAni: true //控制动画的播放
    //story 19807
    property int currentShowIndex: 0
    property string imageDefault: ""
    //    property var imagelist: ["file:///home/zou/图片/5.png"]
    //"jpg", "jpe", "jpeg", "pbm", "pgm", "ppm", "sr", "ras", "png", "tga", "svg", "gif", "apng", "ico", "xpm", "exr", "psd", "jfi", "jif", "j2k", "jp2", "jng", "wbmp", "xbm", "tiff", "tif", "webp", "pnm", "bmp", "dib"
    property var loadImageList:[]; //每次需要加载的图片
    property int imageTotalNum:0;         //图片总数量
    property int currentImageIndex:-1;     //当前图片的位置
    property string imageDelPath:"";      //需要删掉的图片路径
    property var showImageIndex:[]; //记录当前展示的5个item的所有index，用来加载图片用，与loadImageList是一一对应的
    property var imageTypeMovieOrNormal:[]; //记录图片是否是动静图
    property var imagelist: [] //所有图片的list
    property int addImageNum: 0//每次图片增加的数目
    property var loadedImageList:[]//记录已经加载过的图片
    property bool signMouseStart: false
    property string currentType: "normal"
    property var imageListEveryAddType: []
    property bool updateSuorce: false
    property var hasnotFinishedLoad: []
    property var imageHasChanged: false//图片是否操作过
}
