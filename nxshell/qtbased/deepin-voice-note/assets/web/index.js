//C++ 调用js接口
//信号绑定
// initData(const QString& jsonData); 初始化，参数为json字符串
// void setHtml(const QString& html); 初始化，设置html
// insertVoiceItem(const QString &jsonData);　插入语音，参数为json字符串

//callback回调
// const QString getHtml();获取整个html
// const QString getAllNote();获取所有语音列表的Json
//

// 注册右键点击事件
$('body').on('contextmenu', rightClick)
// 注册内容改变事件
$('#summernote').on('summernote.change', changeContent);

// 初始化渲染模板
var h5Tpl = `
    <div class="li voiceBox" contenteditable="false" jsonKey="{{jsonValue}}">
        <div class='voiceInfoBox'>
            <div class="demo" >              
                <div class="voicebtn play"></div>
                <div class="lf">
                    <div class="title">{{title}}</div>
                    <div class="minute padtop">{{createTime}}</div>
                </div>
                <div class="lr">
                    <div class="icon">
                        <div class="wifi-symbol">
                            <div class="wifi-circle"></div>
                        </div>
                    </div>
                    <div class="time padtop">{{transSize}}</div>
                </div>
            </div>
            <div class="translate">
            </div>
        </div>
    </div>
    {{#if text}}
    <p>{{text}}</p>
    {{/if}}
    `;
// 语音插入模板
var nodeTpl = `
        <div class='voiceInfoBox'>
            <div class="demo"  >
                <div class="voicebtn play"></div>
                <div class="lf">
                    <div class="title">{{title}}</div>
                    <div class="minute padtop">{{createTime}}</div>
                </div>
                <div class="lr">
                    <div class="icon">
                        <div class="wifi-symbol">
                            <div class="wifi-circle"></div>
                        </div>
                    </div>
                    <div class="time padtop">{{transSize}}</div>
                </div>
            </div>
            <div class="translate">
            </div>
        </div>`;

var formatHtml = ''
var webobj;    //js与qt通信对象
var activeVoice = null;  //当前正操作的语音对象
var activeTransVoice = null;  //执行语音转文字对象
var bTransVoiceIsReady = true;  //语音转文字是否准备好
var initFinish = false;
var voiceIntervalObj;    //语音播放动画定时器对象
var isVoicePaste = false
var isShowAir = true
var nowClickVoice = null
var global_activeColor = ''
var global_disableColor = ''
var global_theme = 1
var global_themeColor = 'transparent'  //主题色
var scrollHide = null  //滚动条隐藏定时器
var scrollHideFont = null  //字体滚动条定时
var isUlOrOl = false
const airPopoverHeight = 44  //悬浮工具栏高度
const airPopoverWidth = 385  //悬浮工具栏宽度
const airPopoverLeftMargin = 20 //悬浮工具栏距离编辑区左侧边距
const airPopoverRightMargin = 15 //悬浮工具栏距离编辑区右侧边距

// 翻译列表
var tooltipContent = {
    fontname: '字体',
    fontsize: '字号',
    forecolor: '字体颜色',
    backcolor: '背景色',
    bold: '加粗',
    italic: '斜体',
    underline: '下划线',
    strikethrough: '删除线',
    ul: '无序列表',
    ol: '有序列表',
    more: '更多颜色',
    recentlyUsed: '最近使用'
}
// 字体列表
var global_fontList = []

// 国际化
function changeLang(tooltipContent) {
    for (let item in tooltipContent) {
        if (item != 'recentlyUsed') {
            $(`.${item}But`).attr('data-original-title', tooltipContent[item])
        } else {
            $('.recentlyUsedBut').html(tooltipContent[item])
        }
    }
}


/**
 * 设置初始化字体
 * @date 2022-04-12
 * @param {string} initFontName 字体名
 * @returns {any}
 */
function setInitFont(initFontName) {
    $('body').css('font-family', initFontName)
}

// 建立通信
new QWebChannel(qt.webChannelTransport,
    function (channel) {
        webobj = channel.objects.webobj;
        //所有的c++ 调用js的接口都需要在此绑定格式，webobj.c++函数名（jscontent.cpp查看.connect(js处理函数)
        //例如 webobj.c++fun.connect(jsfun)
        webobj.callJsInitData.connect(initData);
        webobj.callJsInsertVoice.connect(insertVoiceItem);
        webobj.callJsSetPlayStatus.connect(toggleState);
        webobj.callJsSetHtml.connect(setHtml);
        webobj.callJsSetVoiceText.connect(setVoiceText);
        webobj.callJsInsertImages.connect(insertImg);
        webobj.callJsSetTheme.connect(changeColor);
        webobj.calllJsShowEditToolbar.connect(showRightMenu);
        webobj.callJsHideEditToolbar.connect(hideRightMenu);
        webobj.callJsClipboardDataChanged.connect(shearPlateChange);
        webobj.callJsSetVoicePlayBtnEnable.connect(playButColor);
        webobj.callJsSetFontList.connect(setFontList);
        webobj.callJsPasteHtml.connect(pasteHtmlFrom);
        //通知QT层完成通信绑定
        webobj.jsCallChannleFinish();
        // setFontList(global_fontList, "Unifont")
    }
)

// 获取字体列表并初始化
function setFontList(fontList, initFont) {
    global_fontList = fontList;
    setInitFont(initFont)
    initSummernote()
    // 通知QT，summernote初始化完成
    webobj.jsCallSummernoteInitFinish()
    // 获取翻译和字体列表后，再初始化summernote
    webobj.jsCallGetTranslation(function (res) {
        changeLang(JSON.parse(res))
    })
}

// 初始化summernote
function initSummernote() {
    $('#summernote').summernote({
        focus: true,
        disableDragAndDrop: true,
        lang: 'zh-CN',
        popover: {
            air: [
                ['fontname', ['fontname']],
                ['fontsize', ['fontsize']],
                ['forecolor', ['forecolor']],
                ['backcolor', ['backcolor']],
                ['bold', ['bold']],
                ['italic', ['italic']],
                ['underline', ['underline']],
                ['strikethrough', ['strikethrough']],
                ['line'],
                ['ul', ['ul']],
                ['ol', ['ol']],
            ],
        },
        fontNames: global_fontList,
        airMode: true,
    });
    // 注册滚动事件
    listenFontnameList()
    // 默认选中
    setSelectColorButton($('[data-value="#414D68"]'))
    setSelectColorButton($('[data-value="transparent"]'))
    $('.icon-backcolor .path5').addClass('transparentColor')

    // 监听窗口大小变化
    $(window).resize(function () {
        $('.note-editable').css('min-height', $(window).height())
    }).resize();
}




/**
 * 通知后台存储页面内容
 * @date 2021-08-19
 * @param {any} we
 * @param {any} contents
 * @param {any} $editable
 * @returns {any}
 */
function changeContent(we, contents, $editable) {
    if ($('.note-editable').html() == '') {
        $('.note-editable').html('<p><br></p>')
    }
    if (webobj && initFinish) {
        if (!$('.note-air-popover').is(':hidden')) {
            if (getSelectedRange().innerHTML == "") {
                $('#summernote').summernote('airPopover.hide')
            }
        }
        webobj.jsCallTxtChange();
    }
}

/**
 * 处理编辑页面大小改变事件
 * @date 2023-05-30
 * @param {any}
 * @returns {any}
 */
window.onresize = function(){
    updateAirPopoverPos()
}

/**
 * 处理air-popover位置更新
 * @date 2023-05-30
 * @param {any} 
 * @returns {any}
 */
function updateAirPopoverPos() {
    if (!$('.note-air-popover').is(':hidden')) {
        if (getSelectedRange().innerHTML != "") {
            $('#summernote').summernote('airPopover.update')
        }
    }
}

// 判断编辑区是否为空
function isNoteNull() {
    return $('.note-editable').html() === '<p><br></p>'
}

//点击选中录音
$('body').on('click', '.li', function (e) {
    e.stopPropagation();
    $('.li').removeClass('active');
    setSelectRange(this)
    isShowAir = false
    $(this).addClass('active');
})

$('body').on('click', '.translate', function (e) {
    // 阻止冒泡
    e.stopPropagation();
})

/**
 * 获取选区
 * @date 2021-09-08
 * @returns {any} div
 */
function getSelectedRange() {
    var selectionObj = window.getSelection();
    var rangeObj = selectionObj.getRangeAt(0);
    var docFragment = rangeObj.cloneContents();
    var testDiv = document.createElement("div");
    testDiv.appendChild(docFragment)
    return testDiv
}

/**
 * 设置光标位置
 * @date 2021-09-09
 * @param {any} element 元素
 * @param {any} pos 起始偏移量
 * @returns {any}
 */
function setFocus(element, pos) {
    var range, selection;
    range = document.createRange();
    range.selectNodeContents(element);
    if (element.innerHTML.length > 0) {
        range.setStart(element.childNodes[0], pos);
    }
    range.collapse(true);       //设置选中区域为一个点
    selection = window.getSelection();
    selection.removeAllRanges();
    selection.addRange(range);
}

/**
 * 设置选区
 * @date 2021-09-10
 * @param {any} dom
 * @returns {any}
 */
function setSelectRange(dom) {
    var sel = window.getSelection();
    sel.removeAllRanges();
    var range = document.createRange();
    range.selectNode(dom);
    // range.collapse(true);
    if (sel.anchorOffset == 0) {
        sel.addRange(range);
    };

}

/**
 * 移除选区
 * @date 2021-09-10
 * @param {any} dom
 * @returns {any}
 */
function removeSelectRange(dom) {
    var sel = window.getSelection();
    var range = document.createRange();
    range.selectNode(dom);
    sel.removeRange(range)
}

// 取消选中样式
$('body').on('click', function () {
    $('.li').removeClass('active');
})

// 语音复制
document.addEventListener('copy', copyVoice);
// 语音剪切
document.addEventListener('cut', copyVoice);




// 语音复制功能
function copyVoice(event) {
    isVoicePaste = false
    isUlOrOl = false
    var selectionObj = window.getSelection();
    var rangeObj = selectionObj.getRangeAt(0);
    var docFragment = rangeObj.cloneContents();
    var testDiv = document.createElement("div");

    // 判断是否语音复制
    if ($(docFragment).find('.voiceBox').length != 0) {
        $(docFragment).find('.translate').html('')
        $(docFragment).find('.voiceBox').removeClass('active')

        $(docFragment).find('.voicebtn').removeClass('pause').addClass('play');
        $(docFragment).find('.voicebtn').removeClass('now');
        $(docFragment).find('.wifi-circle').removeClass('first').removeClass('second').removeClass('third').removeClass('four').removeClass('fifth').removeClass('sixth').removeClass('seventh');

        isVoicePaste = true

    }
    testDiv.appendChild(docFragment);
    isUlOrOl = $(testDiv).children().toArray().every(item => {
        return item.tagName == 'LI'
    })
    // 有序无序列表复制
    if (isUlOrOl) {
        let tagName = rangeObj.commonAncestorContainer.tagName
        let box = document.createElement(tagName)
        docFragment = rangeObj.cloneContents();
        box.appendChild(docFragment)
        $(box).find('p').toArray().forEach(item => {
            if (item.innerHTML == '') {
                item.innerHTML = '<br />'
            }
        })
        testDiv.innerHTML = ''
        testDiv.appendChild(box)
    }
    formatHtml = testDiv.innerHTML;
    if ($(testDiv).children().length == 1 && $(testDiv).children()[0].tagName != 'UL' && $(testDiv).find('.voiceBox').length != 0) {
        formatHtml = '<p><br></p>' + formatHtml

    }
    webobj.jsCallSetClipData(selectionObj.toString().replace(/\n\n/g, '\n'), formatHtml)
    if (event.type == 'cut') {
        // 记录之前数据
        $('#summernote').summernote('editor.recordUndo')
        document.execCommand('delete', false);
        // 主动触发change事件
        changeContent()
    }
    event.preventDefault()
}

// 剪切板内容变化
function shearPlateChange() {
    isVoicePaste = false
}

// 复制标志
function returnCopyFlag() {
    return isVoicePaste
}

function pasteHtmlFrom(html) {
    if (html != "") {
        document.execCommand('insertHTML', false, html + "<p><br></p>");
        event.preventDefault()
    }

    setFocusScroll()
    removeNullP()
}

// 粘贴
document.addEventListener('paste', function (event) {
    if (formatHtml != "" && isVoicePaste) {
        document.execCommand('insertHTML', false, formatHtml + "<p><br></p>");
        event.preventDefault()
    }
    setFocusScroll()
    removeNullP()
});

// 页面滚动到光标位置
function setFocusScroll() {
    let focusY = getCursortPosition(document.querySelector('.note-editable'))
    let scrollTop = $(document).scrollTop()
    let viewY = $(window).height() + scrollTop
    if (focusY < scrollTop || focusY > viewY) {
        $(document).scrollTop(focusY - $(window).height())
    }
}

// 记录当前选中语音
function recordActiveVoice() {
    var selectionObj = window.getSelection();
    var rangeObj = selectionObj.getRangeAt(0);
    let div = document.createElement('div')
    rangeObj.insertNode(div)
    let $voice = $(div).next()
    $voice.closest('.li').addClass('active')
    if (bTransVoiceIsReady) {
        activeTransVoice = $voice.closest('.li');
    }
    $('#summernote').summernote('airPopover.hide')
    setSelectRange($voice.closest('.voiceBox')[0])
    $(div).remove()
}


/**
 * 判断选区类别
 * @date 2021-09-06
 * @returns {object} 0图片 1语音 2文本
 */
function isRangeVoice() {
    let selectedRange = {
        flag: null,
        info: ''
    }
    var testDiv = getSelectedRange();
    let childrenLength = $(testDiv).children().length
    let voiceLength = $(testDiv).find('.voiceBox').length

    if (voiceLength == childrenLength && childrenLength != 0) {
        selectedRange.flag = 1
        selectedRange.info = $(testDiv).find('.voiceBox:first').attr('jsonKey')
        recordActiveVoice()
    } else if ($(testDiv).find('img').length == childrenLength
        && childrenLength == 1
        && $(testDiv).find('img').parent()[0].childNodes.length == 1) {
        selectedRange.flag = 0
        selectedRange.info = $(testDiv).find('img').attr('src')
    } else {
        selectedRange.flag = 2
    }
    return selectedRange
}

//播放
$('body').on('click', '.voicebtn', function (e) {
    // e.stopPropagation();
    var curVoice = $(this).parents('.li:first');
    var jsonString = curVoice.attr('jsonKey');
    var bIsSame = $(this).hasClass('now');
    var curBtn = $(this);
    $('.voicebtn').removeClass('now');
    activeVoice = curBtn;
    activeVoice.addClass('now');

    webobj.jsCallPlayVoice(jsonString, bIsSame, function (state) {
        //TODO 录音错误处理
    });
})

//获取整个处理后Html串,去除所有标签中临时状态
function getHtml() {
    var $cloneCode = $('.note-editable').clone();
    $cloneCode.find('.li').removeClass('active');
    $cloneCode.find('.voicebtn').removeClass('pause').addClass('play');
    $cloneCode.find('.voicebtn').removeClass('now');
    $cloneCode.find('.wifi-circle').removeClass('first').removeClass('second').removeClass('third').removeClass('four').removeClass('fifth').removeClass('sixth').removeClass('seventh');
    $cloneCode.find('.translate').html("")
    return $cloneCode[0].innerHTML;
}

//获取当前所有的语音列表
function getAllNote() {
    var jsonObj = {};
    var jsonArray = [];
    var jsonString;
    $('.li').each(function () {
        jsonString = $(this).attr('jsonKey');
        jsonArray[jsonArray.length] = JSON.parse(jsonString);
    })
    jsonObj.noteDatas = jsonArray;
    var retJson = JSON.stringify(jsonObj);
    return retJson;
}

//获取当前选中录音json串
function getActiveNote() {
    var retJson = '';
    if ($('.active').length > 0) {
        retJson = $('.active').attr('jsonKey');
    }
    return retJson;
}



/**
 * 获取光标Y轴位置
 * @date 2021-09-08
 * @param {any} element 可编辑dom
 * @returns {number} Y轴位置 
 */
function getCursortPosition(element) {
    var caretOffset = 0;
    var doc = element.ownerDocument || element.document;
    var win = doc.defaultView || doc.parentWindow;
    var sel = win.getSelection();
    if (sel.rangeCount > 0) {
        var range = win.getSelection().getRangeAt(0);
        let span = document.createElement('span');
        $(span).addClass('focusAddress')
        range.insertNode(span)
        caretOffset = $(span).offset().top + ($(span).css('fontSize').slice(0, -2) - 0) + 5
        $(span).remove()
    }
    return caretOffset;
}

/**
 * 录音长度格式化
 * @date 2022-02-09
 * @param {any} millisecond  毫秒
 * @returns {any} "00:00"
 */
function formatMillisecond(millisecond) {
    if (millisecond < 1000) {
        return '00:01'
    } else if (millisecond < 3600000) {
        var minutes = parseInt(millisecond / (1000 * 60));
        var seconds = parseInt((millisecond % (1000 * 60)) / 1000);
        return (minutes < 10 ? '0' + minutes : minutes) + ":" + (seconds < 10 ? '0' + seconds : seconds);
    }
    return "60:00";
}

//初始化数据 
function initData(text) {
    initFinish = false;
    var arr = JSON.parse(text);
    var html = '';
    var voiceHtml;
    var txtHtml = '';
    arr.noteDatas.forEach((item, index) => {
        //false: txt
        if (item.type == 1) {
            txtHtml = ''
            if (item.text == '') {
                txtHtml = '<p><br></p>';
            }
            else {
                let textArr = item.text.split('\n')
                textArr.forEach(ite => {
                    txtHtml = txtHtml + '<p>' + ite + '</p>';
                })
            }
            html += txtHtml;
        }
        //true: voice
        else {
            if (!item.transSize) {
                item.transSize = formatMillisecond(item.voiceSize)
            }
            voiceHtml = transHtml(item, false);
            html += voiceHtml;
        }
    })

    $('#summernote').summernote('code', html);
    // 搜索功能
    webobj.jsCallSetDataFinsh();
    initFinish = true;
    $('#summernote').summernote('editor.resetRecord')
}

/**
 * 播放按钮置灰
 * @date 2021-09-03
 * @param {boolean} status false禁用 true启用
 * @returns {any}
 */
function playButColor(status) {
    if (!status) {
        setVoiceButColor(global_disableColor, global_disableColor)

    } else {
        setVoiceButColor(global_activeColor, global_disableColor)
    }
}

//录音插入数据
function insertVoiceItem(text) {
    //插入语音之前先设置焦点
    $('#summernote').summernote('editor.focus')
    // 记录插入前数据
    $('#summernote').summernote('editor.recordUndo')

    var arr = JSON.parse(text);
    var voiceHtml = transHtml(arr, true);
    var oA = document.createElement('div');
    oA.className = 'li voiceBox';
    oA.contentEditable = false;
    oA.setAttribute('jsonKey', text);
    oA.innerHTML = voiceHtml;

    var tmpNode = document.createElement("div");
    tmpNode.appendChild(oA.cloneNode(true));
    var str = '<p><br></p>' + tmpNode.innerHTML + '<p><br></p>';

    document.execCommand('insertHTML', false, str);
    // $('#summernote').summernote('editor.recordUndo')

    removeNullP()
    setFocusScroll()
}

/**
 * 移除无内容p标签
 * @date 2021-08-19
 * @returns {any}
 */
function removeNullP() {
    $('ul').children().each((index, item) => {
        if (item.tagName == 'P') {
            $(item).remove();
        }
    })
    $('ol').children().each((index, item) => {
        if (item.tagName == 'P') {
            $(item).remove();
        }
    })
    $('p').each((index, item) => {
        if (item.innerHTML === '') {
            $(item).remove();
        }
    })
}

/**
 * 切换播放状态
 * @date 2021-08-19
 * @param {string} state 0,播放中，1暂停中，2.结束播放
 * @returns {any}
 */
function toggleState(state) {
    if (state == '0') {
        $('.voicebtn').removeClass('pause').addClass('play');
        activeVoice.removeClass('play').addClass('pause');
        voicePlay(true);
    } else if (state == '1') {
        activeVoice.removeClass('pause').addClass('play');
        voicePlay(false);
    }
    else {
        $('.voicebtn').removeClass('pause').addClass('play');
        $('.voicebtn').removeClass('now');
        activeVoice = null;
        voicePlay(false);
    }
}

/**
 * 设置整个html内容
 * @date 2021-08-19
 * @param {string} html
 * @returns {any}
 */
function setHtml(html) {
    if (html == '<p></p>') {
        html = '<p><br></p>'
    }
    initFinish = false;
    $('#summernote').summernote('code', html);
    initFinish = true;
    // 搜索功能
    webobj.jsCallSetDataFinsh();
    resetScroll()
    $('#summernote').summernote('editor.resetRecord')
}

//设置录音转文字内容 flag: 0: 转换过程中 提示性文本（＂正在转文字中＂)１:结果 文本,空代表转失败了
function setVoiceText(text, flag) {
    if (activeTransVoice) {
        if (flag) {
            $('.translate').html('')
            if (text) {
                text = text.trim()
                activeTransVoice.after('<p>' + text + '</p>');
                webobj.jsCallTxtChange();
            }
            //将转文字文本写到json属性里
            var jsonValue = activeTransVoice.attr('jsonKey');
            var jsonObj = JSON.parse(jsonValue);
            jsonObj.text = text;
            activeTransVoice.attr('jsonKey', JSON.stringify(jsonObj));

            webobj.jsCallTxtChange();
            activeTransVoice = null;
            bTransVoiceIsReady = true;

            // 移除选区
            var sel = window.getSelection();
            var range = sel.getRangeAt(0);
            sel.removeRange(range)
            $('.li').removeClass('active');
        }
        else {
            activeTransVoice.find('.translate').html('<div class="noselect">' + text + '</div><div class="showTextBox">' + text + '</div>');
            bTransVoiceIsReady = false;
        }
    }
}

//json串拼接成对应html串 flag==》》 false: h5串  true：node串
function transHtml(json, flag) {
    let createTime = json.createTime.slice(0, 16)
    createTime = createTime.replace(/-/g, `/`)
    json.createTime = createTime
    //将json内容当其属性与标签绑定
    var strJson = JSON.stringify(json);
    json.jsonValue = strJson;
    var template;
    if (flag) {
        template = Handlebars.compile(nodeTpl);
    }
    else {
        template = Handlebars.compile(h5Tpl);
    }
    var retHtml = template(json);
    return retHtml;
}

//设置summerNote编辑状态 
function enableSummerNote() {
    if (activeVoice || (activeTransVoice && !bTransVoiceIsReady)) {
        $('#summernote').summernote('disable');
    }
    else {
        $('#summernote').summernote('enable');
    }
}

// 录音播放控制， bIsPaly=ture 表示播放。
function voicePlay(bIsPaly) {
    clearInterval(voiceIntervalObj);
    $('.wifi-circle').removeClass('first').removeClass('second').removeClass('third').removeClass('four').removeClass('fifth').removeClass('sixth').removeClass('seventh');

    if (bIsPaly) {
        var index = 0;
        voiceIntervalObj = setInterval(function () {
            if (activeVoice && activeVoice.hasClass('pause')) {
                var voiceObj = activeVoice.parent().find('.wifi-circle');
                index++;
                switch (index) {
                    case 1:
                        voiceObj.removeClass('seventh').addClass('first');
                        break;
                    case 2:
                        voiceObj.removeClass('first').addClass('second');
                        break;
                    case 3:
                        voiceObj.removeClass('second').addClass('third');
                        break;
                    case 4:
                        voiceObj.removeClass('third').addClass('four');
                        break;
                    case 5:
                        voiceObj.removeClass('four').addClass('fifth');
                        break;
                    case 6:
                        voiceObj.removeClass('fifth').addClass('sixth');
                        break;
                    case 7:
                        voiceObj.removeClass('sixth').addClass('seventh');
                        index = 0;
                        break;
                }
            }
        }, 200);
    }
}

/**
 * 右键功能
 * @date 2021-08-19
 * @param {any} e
 * @returns {any} 
 */
function rightClick(e) {
    // isShowAir = false;
    var testDiv = getSelectedRange();
    let childrenLength = $(testDiv).children().length
    let voiceLength = $(testDiv).find('.voiceBox').length
    if (e.which == 3) {
        $('.li').removeClass('active');
    }
    if (e.target.tagName == 'IMG') {
        let img = e.target
        $('.note-editable ').blur()
        setSelectRange(img)
    } else if ($(e.target).hasClass('demo') || $(e.target).parents('.demo').length != 0) {
        $(e.target).parents('.li').addClass('active');
        // 当前没有语音在转文字时， 才可以转文字
        if (bTransVoiceIsReady) {
            activeTransVoice = $(e.target).parents('.li:first');
        }
        $('#summernote').summernote('airPopover.hide')
        setSelectRange($(e.target).parents('.voiceBox')[0])
    } else if (voiceLength == childrenLength && childrenLength != 0) {
        recordActiveVoice()
    }
    let info = isRangeVoice()
    webobj.jsCallPopupMenu(info.flag, info.info);
    // 阻止默认右键事件
    // e.preventDefault()
}


/**
 * 设置活动色
 * @date 2021-09-06
 * @param {any} color
 * @returns {any}
 */
function setVoiceButColor(color, shdow) {
    $("#style").html(`
    .voiceBox .voicebtn {
        background-color:${color};
        box-shadow: 0px 4px 6px 0px ${shdow}80; 
    } 
    ::selection {
        background: ${color}!important;
    }
    .btn-default.active i {
        color:${color}!important
    }
    .btn-default:active {
        color:${color}!important
    }
    .btn-default i:active {
        color:${color}!important
    }
    .selectColor {
        box-shadow: 0 0 0 1.5px ${color};
    }
    .active {
        background: ${color}80!important;
    }
    `)
}

/**
 * 深色浅色变换
 * @date 2021-08-19
 * @param {any} flag 1浅色 2深色
 * @returns {any}
 */
function changeColor(flag, activeColor, disableColor, backgroundColor) {
    global_theme = flag
    global_activeColor = activeColor
    global_disableColor = disableColor
    global_themeColor = backgroundColor
    setVoiceButColor(global_activeColor, global_disableColor)

    $('.dropdown-fontsize>li>a').hover(function (e) {
        $(this).css('background-color', activeColor);
    }, function () {
        $('.dropdown-fontsize>li>a').css('background-color', 'transparent');
        if (flag == 1) {
            $('.dropdown-fontsize>li>a').css('color', "black");
        } else {
            $('.dropdown-fontsize>li>a').css('color', "rgba(197,207,224,1)");
        }
    })
    $('.dropdown-fontname>li>a').hover(function (e) {
        $(this).css('background-color', activeColor);
    }, function () {
        $('.dropdown-fontname>li>a').css('background-color', 'transparent');
        if (flag == 1) {
            $('.dropdown-fontname>li>a').css('color', "black");
        } else {
            $('.dropdown-fontname>li>a').css('color', "rgba(197,207,224,1)");
        }
    })
    $('body').css('background-color', global_themeColor)
    if (flag == 1) {
        $('#dark').remove()
        $('.dropdown-fontsize>li>a').css('color', "black");
        $('.dropdown-fontname>li>a').css('color', "black");
    } else if (flag == 2 && !$('#dark').length) {
        $("head").append("<link>");
        var css = $("head").children(":last");
        css.attr({
            id: 'dark',
            rel: "stylesheet",
            type: "text/css",
            href: "./css/dark.css"
        });
    }
}

/**
 * 插入图片
 * @date 2021-08-19
 * @param {any} urlStr 图片地址list
 * @returns {any}
 */
async function insertImg(urlStr) {
    urlStr.forEach((item, index) => {
        $("#summernote").summernote('insertImage', item, 'img');
    })
}

//  
document.onkeydown = function (event) {
    if (window.event.keyCode == 13) {
        // 回车
        setFocusScroll()
    } else if (event.ctrlKey && window.event.keyCode == 86) {
        // ctrl+v
        webobj.jsCallPaste(returnCopyFlag())
        return false;
    } else if (window.event.keyCode == 8) {
        // backspace
        if (getSelectedRange().innerHTML == document.querySelector('.note-editable').innerHTML) {
            $('.note-editable').html('<p><br></p>')
        }

        // 移除行内样式的p元素
        let styleStr = $('p[style="display: inline !important;"]')
        styleStr.after(styleStr.html()).remove();

        var selectionObj = window.getSelection();
        let focusDom = selectionObj.extentNode
        // 判断是选区还是光标
        if (selectionObj.type != 'Caret') {
            return
        }
        if ($(focusDom).closest('span').length
            && $(focusDom).closest('span').html() !== '<br>'
            && selectionObj.extentOffset == 0) {
            if ($(focusDom).parents('li').length
                && $(focusDom).parents('li').prev().length
                && $(focusDom).parents('li').prev()[0].tagName == 'LI'
                && !$(focusDom).parents('li').find('.voiceBox').length) {
                // 父元素为LI
                setBackspace(focusDom, "li")
                return false
            }
            else if ($(focusDom).parents('p').length
                && $(focusDom).parents('p').prev().length
                && $(focusDom).parents('p').prev()[0].tagName == 'UL') {
                // 父元素为P 上个元素为UL
                setBackspace(focusDom, "p")
                return false
            }
            else if ($(focusDom).parents('p').length
                && $(focusDom).parents('p').prev().length
                && $(focusDom).parents('p').prev()[0].tagName == 'OL') {
                // 父元素为P 上个元素为OL
                setBackspace(focusDom, "p")
                return false
            }
        }
    } else if (window.event.keyCode == 46) {
        // delete
        var selectionObj = window.getSelection();
        let focusDom = selectionObj.extentNode
        let domList = $(focusDom).parents()
        let isHas = true
        // 判断是选区还是光标
        if (selectionObj.type != 'Caret') {
            return
        }
        // 处理LI以内子元素
        for (let i = 0; i < domList.length; i++) {
            if (domList[i].tagName == 'LI') {
                let nowItem = domList[i]
                domList = domList.slice(0, i);
                if (domList.length == 0) {
                    domList.push(nowItem.firstChild)
                }
                break;
            }
        }
        domList = domList.toArray()
        // 判断是否有除br的以外的兄弟元素
        for (let i = 0; i < domList.length; i++) {
            if (domList[i].nextSibling) {
                if (domList[i].nextSibling.nodeType == 3 || (domList[i].nextSibling.nodeType != 3 && !domList[i].nextSibling.innerHTML.match(/<br>/g))) {
                    isHas = false;
                    break;
                } else if (domList[i].nextSibling.innerHTML.match(/<br>/g)) {
                    $(domList[i].nextSibling).remove()
                    isHas = true;
                }
            } else {
                isHas = true;
            }
        }

        if (selectionObj.extentOffset == focusDom.nodeValue.length && isHas) {
            if ($(focusDom).parents('li').length
                && $(focusDom).parents('li').next().length
                && $(focusDom).parents('li').next()[0].tagName == 'LI'
                && !$(focusDom).parents('li').find('.voiceBox').length
                && $(focusDom).parents('li').next().children('span').length) {
                // 下个元素为LI
                setDelete(focusDom, "li")
                return false
            }
            else if (!$(focusDom).parents('li').next().length
                && $(focusDom).parents('ul').next().length
                && $(focusDom).parents('ul').next()[0].tagName == 'P'
                && $(focusDom).parents('ul').next().children('span').length) {
                // 下个元素为P
                setDelete(focusDom, "ul")
                return false
            }
            else if (!$(focusDom).parents('li').next().length
                && $(focusDom).parents('ol').next().length
                && $(focusDom).parents('ol').next()[0].tagName == 'P'
                && $(focusDom).parents('ol').next().children('span').length) {
                // 下个元素为P 
                setDelete(focusDom, "ol")
                return false
            }
        }
    } else if (event.ctrlKey && window.event.keyCode == 65) {
        // ctrl+a
        // 去除语音选中样式
        $('.li').removeClass('active');
    } else if (window.event.keyCode == 46) {
        // delete
        if (getSelectedRange().innerHTML == document.querySelector('.note-editable').innerHTML) {
            $('.note-editable').html('<p><br></p>')
        }
    }

}

/**
 * delete 改变元素位置
 * @date 2021-11-25
 * @param {dom} focusDom 焦点所在元素
 * @param {string} name 父元素名
 * @returns {any}
 */
function setDelete(focusDom, name) {
    let nextDom = $(focusDom).closest(name).next()
    if (name == 'li') {
        $(focusDom).parents(name).append($(focusDom).parents('li').next())
    } else {
        $(focusDom).closest('li').append(nextDom)
    }
    nextDom.children(":first").unwrap()
}

/**
 * backspace 改变元素位置
 * @date 2021-11-25
 * @param {dom} focusDom 焦点所在元素
 * @param {string} name 父元素名
 * @returns {any}
 */
function setBackspace(focusDom, name) {
    if (name == 'li') {
        $(focusDom).parents(name).prev().append($(focusDom).parents('li'))
    } else {
        $(focusDom).parents(name).prev().children(':last').append($(focusDom).parents(name))
    }
    let prevDom = $(focusDom).closest(name).prev()
    if (prevDom.length && prevDom[0].tagName == "SPAN" && prevDom.html().match(/<br>/g)) {
        prevDom.remove()
    }
    $(focusDom).parents(name).children(":first").unwrap()
    setFocus($(focusDom).parent()[0], 0)
}

// ctrl+b 添加记事本
document.onkeyup = function (event) {
    if (event.ctrlKey && window.event.keyCode == 66) {
        webobj.jsCallCreateNote()
    }
}

// 图片双击预览
$('body').on('dblclick', 'img', function (e) {
    e.stopPropagation()
    e.preventDefault()
    let imgUrl = $(e.target).attr('src')
    webobj.jsCallViewPicture(imgUrl)
})

// 图片单击选中
$('body').on('click', 'img', function (e) {
    let img = e.target
    setSelectRange(img)
})

/**
 * 右键显示悬浮工具栏
 * @date 2021-09-03
 * @param {any} x 坐标
 * @param {any} y 坐标
 * @returns {any}
 */
function showRightMenu(x, y) {
    $('#summernote').summernote('airPopover.rightUpdate', x, y)
}

// 右键隐藏悬浮工具栏
function hideRightMenu() {
    $('#summernote').summernote('airPopover.hide')
}

// 重置滚动条
function resetScroll() {
    if ($(document).scrollTop() != 0) {
        $(document).scrollTop(0)
    }
}

// 监听滚动事件
$(document).scroll(function () {
    if (scrollHide) {
        clearTimeout(scrollHide)
        $('#scrollStyle').html(`
        body::-webkit-scrollbar-thumb {
        background-color:${global_theme == 1 ? "rgba(0, 0, 0, 0.30)" : "rgba(255, 255, 255, 0.20)"} ;
        }
        /* 适配申威，触发重绘 */
        html {
            background-color: ${global_themeColor}fe;
        }
        `
        )
    }

    scrollHide = setTimeout(() => {
        $('#scrollStyle').html('')
    }, 1500);
});

// 字体滚动条
function listenFontnameList() {
    $('.dropdown-fontname ').scroll(function () {
        // 阻止内部滚动条到底后自动触发外部滚动
        const scroll = document.querySelector('.dropdown-fontname')
        // 当前滚动条距离底部还剩2px时
        if (scroll.scrollHeight - (scroll.scrollTop + scroll.clientHeight) <= 2) {
            // 定位到距离底部2px的位置
            scroll.scrollTop = (scroll.scrollHeight - scroll.clientHeight - 2)
        }

        if (scrollHideFont) {
            clearTimeout(scrollHideFont)
            $('#scrollStyleFont').html(`
        .dropdown-fontname::-webkit-scrollbar-thumb {
        background-color:${global_theme == 1 ? "rgba(0, 0, 0, 0.30)" : "rgba(255, 255, 255, 0.20)"} ;
        }
        /* 适配申威，触发重绘 */
        html {
            background-color: ${global_themeColor}fe;
        }
        `
            )
        }

        scrollHideFont = setTimeout(() => {
            $('#scrollStyleFont').html('')
        }, 1500);
    });
}


/**
 * 改变光标颜色
 * @date 2021-09-09
 * @param {int} flag 1 透明 2 自动
 * @returns {any}
 */
function setFocusColor(flag) {
    $('.note-editable').css('caret-color', flag == 1 ? 'transparent' : 'auto')
}

// 复制标志
function returnCopyFlag() {
    return isVoicePaste
}

// 当前选中颜色active
function setSelectColorButton($dom) {
    $dom.parents('.note-color-palette').find('.note-color-btn').removeClass('selectColor')
    $dom.addClass('selectColor')
}


