// 语言资源
let translations = {};

// 支持的语言
const supportedLanguages = ['en', 'zh'];

// 获取浏览器语言
function getBrowserLanguage() {
    const lang = navigator.language || navigator.userLanguage;
    return lang.split('-')[0]; // 只取主语言代码（如 'en' 或 'zh'）
}

// 加载语言文件
async function loadLanguage(lang) {
    if (!supportedLanguages.includes(lang)) {
        lang = 'en'; // 默认英语
    }
    try {
        const response = await fetch(`lang/${lang}.json`);
        if (!response.ok) {
            throw new Error('语言文件加载失败');
        }
        translations = await response.json();
        updatePageContent(); // 更新页面内容
    } catch (error) {
        console.error('加载语言文件时出错:', error);
    }
}

// 更新页面内容
function updatePageContent() {
    // 更新标题
    document.getElementById('title').textContent = translations.title;

    // 更新语言选择器标签
    document.getElementById('language-label').textContent = translations.language;

    // 更新构建信息标签
    document.getElementById('build-version-label').textContent = translations.buildVersion;
    document.getElementById('build-target-label').textContent = translations.buildTarget;
    document.getElementById('build-arch-label').textContent = translations.buildArch;
    document.getElementById('start-time-label').textContent = translations.startTime;
    document.getElementById('end-time-label').textContent = translations.endTime;
    document.getElementById('duration-label').textContent = translations.duration;
    document.getElementById('next-build-time-label').textContent = translations.nextBuildTime;

    // 更新状态文本
    const pkgStatus = document.getElementById('lingmo-pkg-status').getAttribute('data-status');
    const isoStatus = document.getElementById('lingmo-iso-status').getAttribute('data-status');

    document.getElementById('pkg-status-text').textContent = translations.pkgStatus[pkgStatus] || translations.pkgStatus.unknown;
    document.getElementById('iso-status-text').textContent = translations.isoStatus[isoStatus] || translations.isoStatus.unknown;
}

// 读取文件内容
async function fetchStatusFile() {
    try {
        const response = await fetch('get_status.build.sa');
        if (!response.ok) {
            throw new Error('文件读取失败');
        }
        const text = await response.text();
        return text;
    } catch (error) {
        console.error('读取文件时出错:', error);
        return null;
    }
}

// 解析文件内容
function parseStatusFile(content) {
    const status = {};
    const lines = content.split('\n');
    lines.forEach(line => {
        const [key, value] = line.split('=');
        if (key && value !== undefined) {
            status[key.trim()] = value.trim();
        }
    });
    return status;
}

// 更新构建状态
function updateStatus(status) {
    const pkgStatusElement = document.getElementById('lingmo-pkg-status');
    const isoStatusElement = document.getElementById('lingmo-iso-status');

    // 更新软件包构建状态
    const pkgStatus = status['AUTO_BUILD_STATUS_LINGMO_PKG'];
    if (pkgStatus === '0') {
        pkgStatusElement.setAttribute('data-status', 'success');
        pkgStatusElement.className = 'status success';
    } else if (pkgStatus === '1') {
        pkgStatusElement.setAttribute('data-status', 'building');
        pkgStatusElement.className = 'status building';
    } else if (pkgStatus === '2') {
        pkgStatusElement.setAttribute('data-status', 'failed');
        pkgStatusElement.className = 'status failed';
    } else if (pkgStatus === '3') {
        pkgStatusElement.setAttribute('data-status', 'pending');
        pkgStatusElement.className = 'status pending';
    } else if (pkgStatus === '4') {
        pkgStatusElement.setAttribute('data-status', 'paused');
        pkgStatusElement.className = 'status paused';
    } else {
        pkgStatusElement.setAttribute('data-status', 'unknown');
        pkgStatusElement.className = 'status unknown';
    }

    // 更新ISO构建状态
    const isoStatus = status['AUTO_BUILD_STATUS_LINGMO_ISO'];
    if (isoStatus === '0') {
        isoStatusElement.setAttribute('data-status', 'success');
        isoStatusElement.className = 'status success';
    } else if (isoStatus === '1') {
        isoStatusElement.setAttribute('data-status', 'building');
        isoStatusElement.className = 'status building';
    } else if (isoStatus === '2') {
        isoStatusElement.setAttribute('data-status', 'failed');
        isoStatusElement.className = 'status failed';
    } else if (isoStatus === '3') {
        isoStatusElement.setAttribute('data-status', 'pending');
        isoStatusElement.className = 'status pending';
    } else if (isoStatus === '4') {
        isoStatusElement.setAttribute('data-status', 'paused');
        isoStatusElement.className = 'status paused';
    } else {
        isoStatusElement.setAttribute('data-status', 'unknown');
        isoStatusElement.className = 'status unknown';
    }

    // 更新详细信息
    document.getElementById('build-version').textContent = status['BUILD_VERSION'] || '-';
    document.getElementById('build-target').textContent = status['BUILD_TARGET'] || '-';
    document.getElementById('build-arch').textContent = status['BUILD_ARCH'] || '-';
    document.getElementById('start-time').textContent = status['BUILD_START_TIME'] || '-';
    document.getElementById('end-time').textContent = status['BUILD_END_TIME'] || '-';
    document.getElementById('duration').textContent = status['BUILD_DURATION'] || '-';
    document.getElementById('next-build-time').textContent = status['NEXT_BUILD_TIME'] || '-';

    // 更新语言文本
    updatePageContent();
}

// 初始化
async function init() {
    // 设置默认语言
    const browserLang = getBrowserLanguage();
    await loadLanguage(browserLang);

    // 设置语言选择器的默认值
    const languageSelect = document.getElementById('language-select');
    languageSelect.value = supportedLanguages.includes(browserLang) ? browserLang : 'en';

    // 监听语言选择器的变化
    languageSelect.addEventListener('change', async (event) => {
        await loadLanguage(event.target.value);
    });

    // 读取构建状态文件
    const content = await fetchStatusFile();
    if (content) {
        const status = parseStatusFile(content);
        updateStatus(status);
    }
}

// 页面加载时运行
init();
