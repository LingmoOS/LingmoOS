// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileclient.h"
#include "tokencache.h"

#include "filesystem/file.h"
#include "filesystem/path.h"
#include "filesystem/directory.h"

#include "server/http/https_client.h"

#include <iostream>

using CppServer::HTTP::HTTPRequest;
using CppServer::HTTP::HTTPResponse;

class HTTPFileClient : public CppServer::HTTP::HTTPSClientEx
{
public:
    using CppServer::HTTP::HTTPSClientEx::HTTPSClientEx;

    void setResponseHandler(ResponseHandler cb)
    {
        _handler = std::move(cb);
        // Must sleep for release something, or it will be blocked in request
        CppCommon::Thread::Yield();
        CppCommon::Thread::Sleep(1);
    }

protected:
    void onConnected() override
    {
        // must invoke supper fun, or cause canot connect server.
        HTTPSClientEx::onConnected();
    }

    void onDisconnected() override
    {
        HTTPSClientEx::onDisconnected();
    }

    void onReceivedResponseHeader(const HTTPResponse &response) override
    {
//        std::cout << "Response status: " << response.status() << std::endl;
//        std::cout << "Response header body len: " << response.body_length() << std::endl;
//        std::cout << "------------------" << std::endl;
//        std::cout << "Response header Content: \n" << response.string() << std::endl;
//        std::cout << "------------------" << std::endl;

        if (_handler) {
            // get body by stream, so mark response arrived.
            HTTPSClientEx::onReceivedResponse(response);

            if (_handler(response.status() == 200 ? RES_OKHEADER : RES_NOTFOUND, response.string().data(), response.body_length())) {
                // cancel
                DisconnectAsync();
            }
            _response.ClearCache();
        }
    }

    // it has been marked response arrived when geting its header. Mark all done and clear
    void onReceivedResponse(const HTTPResponse &response) override
    {
        //std::cout << "Response done! data len:" << response.body_length() << std::endl;
        if (_canceled)
            return;

        if (_handler) {
            std::string cache = response.cache();
            size_t size = cache.size();
            if (_handler(RES_FINISH, cache.data(), size)) {
                // cancel
                DisconnectAsync();
            }
            _response.Clear();
            // donot disconnect at here, this connection may be continue to downlad other.
        } else {
            HTTPSClientEx::onReceivedResponse(response);
        }
    }

    bool onReceivedResponseBody(const HTTPResponse &response) override
    {
       // std::cout << "Response BODY cache: " << response.cache().size() << std::endl;
        if (_handler) {
            std::string cache = response.cache();
            size_t size = cache.size();
            if (_handler(RES_BODY, cache.data(), size)) {
                _canceled = true;
                // cancel
                DisconnectAsync();
            }
            _response.ClearCache();
        }

        return true;
    }

    void onReceivedResponseError(const HTTPResponse &response, const std::string &error) override
    {
        std::cout << "Response error: " << error << std::endl;
        if (_handler) {
            _handler(RES_ERROR, nullptr, 0);
        }
    }

private:
    ResponseHandler _handler { nullptr };
    std::atomic<bool> _canceled { false };
};

FileClient::FileClient(const std::shared_ptr<CppServer::Asio::Service> &service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, const std::string &address, int port)
{
    if (_httpClient) {
        //discontect current one
        _httpClient->DisconnectAsync();
    }
    // Create HTTP client if the current one is empty
    _httpClient = std::make_shared<HTTPFileClient>(service, context, address, port);
}

FileClient::~FileClient()
{
    if (_httpClient) {
        _httpClient->Disconnect();
        _httpClient = nullptr;
    }
}

std::vector<std::string> FileClient::parseWeb(const std::string &token)
{
    return TokenCache::GetInstance().getWebfromToken(token);
}

void FileClient::setConfig(const std::string &token, const std::string &savedir)
{
    _token = token;
    _savedir = savedir;
}

void FileClient::stop()
{
    _stop.store(true);
    _httpClient->DisconnectAsync();
    // Note: can not join this thread beause of it callback to main thread.
    // _download_thread.join();
}

void FileClient::startFileDownload(const std::vector<std::string> &webnames)
{
    if (_token.empty() || _savedir.empty()) {
        std::cout << "Must setConfig first!" << std::endl;
        return;
    }

    _stop.store(false);

    _download_thread = CppCommon::Thread::Start([this, webnames]() { walkDownload(webnames); });
}

//-------------private-----------------

// request the dir/file info
// [HEAD]webstart>|webfinish>|webindex><name>
void FileClient::sendInfobyHeader(uint8_t mask, const std::string &name)
{
    if (mask >= INFO_WEB_MAX) {
        return;
    }
    _httpClient->setResponseHandler(nullptr);

    std::string url = s_headerInfos[mask] + ">" + name;
    _httpClient->SendHeadRequest(url, CppCommon::Timespan::seconds(3)).get();
}

InfoEntry FileClient::requestInfo(const std::string &name)
{
    InfoEntry fileInfo;
//    std::vector<std::pair<std::string, int64_t>> infos;
    if (_token.empty()) {
        std::cout << "Must set access token!" << std::endl;
        return fileInfo;
    }

    // request the dir/file info
    // [GET]info/<name>&token
    std::string url = "info/";
    // base64 the file name in order to keep original name, which may include '&'
    std::string ename = CppCommon::Encoding::Base64Encode(name);
    url.append(ename);
    url.append("&token=").append(_token);

    _httpClient->setResponseHandler(nullptr);

    auto response = _httpClient->SendGetRequest(url, CppCommon::Timespan::seconds(3)).get();
    if (response.status() == 404) {
        return fileInfo;
    }
    std::string body_str = response.body().data();

    picojson::value v;
    std::string err = picojson::parse(v, body_str);
    if (!err.empty()) {
        std::cout << "Failed to parse JSON data: " << err << std::endl;
        return fileInfo;
    }

    fileInfo.from_json(v);
    return fileInfo;
}

std::string FileClient::getHeadKey(const std::string &headstrs, const std::string &keyfind)
{
    std::unordered_map<std::string, std::string> headermap;
    std::stringstream ss(headstrs);

    std::string line;
    while (std::getline(ss, line, '\n')) {
        size_t delimiterPos = line.find(':');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            // 移除末尾的空格
            key.erase(key.find_last_not_of(" \n\r\t") + 1);
            value.erase(0, value.find_first_not_of(" \n\r\t"));

            headermap[key] = value;
        }
    }
    std::string value = headermap[keyfind];
    return value;
}

// download the file by name
// [GET]download/<name>&token
bool FileClient::downloadFile(const std::string &name, const std::string &rename)
{
    auto avaipath = createNextAvailableName(rename.empty() ? name : rename, true);
    if (avaipath.empty()) {
        //FS exception now
        std::cout << "createNextAvailableName exception now! " << name << std::endl;
        _callback->onWebChanged(WEB_IO_ERROR, "fs_exception");
        return false;
    }

    bool result = false;
    {
        const int EXIT_COUNT = 5000; // timeout if no data arrived
        std::atomic<int> timeout_done = 0;

        uint64_t current = 0, total = 0;
        uint64_t offset = 0;

        auto tempFile = CppCommon::File(avaipath);
        //    offset = tempFile.size();

        ResponseHandler cb([&](int status, const char *buffer, size_t size) -> bool {
            timeout_done.store(0); // reset when data arrived
            if (_stop.load()) {
                std::cout << "has been canceled from outside!" << std::endl;
                return true;
            }

            bool shouldExit = false;
            switch (status)
            {
            case RES_NOTFOUND: {
                std::cout << "File not Found!" << std::endl;

                // error：not found
                shouldExit = true;
                _callback->onWebChanged(WEB_NOT_FOUND, "not_found");
            }
            break;
            case RES_OKHEADER: {
                //std::string flag = this->getHeadKey(buffer, "Flag");
                //std::cout << "head flag: " << flag << std::endl;
                try {
                    CppCommon::Path file_path = tempFile.absolute().RemoveExtension();

                    if (!tempFile.IsFileWriteOpened()) {
                        size_t cur_off = 0;
                        if (tempFile.IsFileExists()) {
                            cur_off = tempFile.size();
                            //std::cout << "Exists seek=: " << offset  << " cur_off=" << cur_off << std::endl;
                        }
                        tempFile.OpenOrCreate(false, true, true);

                        // set offset and current size
                        tempFile.Seek(cur_off);
                    }

                    total = size;
                    _callback->onWebChanged(WEB_FILE_BEGIN, file_path.string(), total);
                } catch (const CppCommon::FileSystemException &ex) {
                    std::cout << "Header create throw FS exception: " << ex.message() << std::endl;
                    shouldExit = true;
                    _callback->onWebChanged(WEB_IO_ERROR, "io_error");
                }
            }
            break;
            case RES_BODY: {
                if (tempFile.IsFileWriteOpened() && buffer && size > 0) {
                    current += size;
                    try {
                        // 实现层已循环写全部
                        tempFile.Write(buffer, size);

                        shouldExit = _callback->onProgress(size);
                    } catch (const CppCommon::FileSystemException &ex) {
                        std::cout << "Write throw FS exception: " << ex.message() << std::endl;
                        shouldExit = true;
                        _callback->onWebChanged(WEB_IO_ERROR, "io_error");
                    }
                }
            }
            break;
            case RES_ERROR: {
                // error：break off
                shouldExit = true;

                _callback->onWebChanged(WEB_DISCONNECTED, "net_error");
            }
            break;
            case RES_FINISH: {
                if (tempFile.IsFileWriteOpened()) {
                    current += size;
                    try {
                        // 写入最后一块
                        tempFile.Write(buffer, size);
                    } catch (const CppCommon::FileSystemException &ex) {
                        std::cout << "Write&Close throw FS exception: " << ex.message() << std::endl;
                        _callback->onWebChanged(WEB_IO_ERROR, "io_error");
                    }
                }
                // std::cout << tempFile.string() << " RES_FINISH, current=" << current << " total:" << total << std::endl;

                shouldExit = true;
                _callback->onWebChanged(WEB_FILE_END, tempFile.string(), total);
            }
            break;

            default:
                std::cout << "error, unkonw status=" << status << std::endl;
                break;
            }

            if (shouldExit) {
                timeout_done.store(EXIT_COUNT);
            }

            return shouldExit;
        });

        _httpClient->setResponseHandler(std::move(cb));

        std::string url = "download/";
        // base64 the file name in order to keep original name, which may include '&'
        std::string ename = CppCommon::Encoding::Base64Encode(name);
        url.append(ename);
        url.append("&token=").append(_token);
        url.append("&offset=").append(std::to_string(offset));

        _httpClient->SendGetRequest(url).get(); // use get to sync download one by one

        // Wait for download finish or no data arrived more than many times
        while (!_stop.load()) {
            auto count = timeout_done.load();
            if (count >= EXIT_COUNT) {
                break;
            }
            timeout_done.fetch_add(1);
            CppCommon::Thread::Yield();
            CppCommon::Thread::Sleep(1);
        }

        // make sure the file has been closed
        if (tempFile.IsFileWriteOpened()) {
            try {
                // tempFile.Flush();
                tempFile.Close();
                tempFile.Clear();
            } catch (const CppCommon::FileSystemException &ex) {
                std::cout << "Close throw FS exception: " << ex.message() << std::endl;
            }
        }

        _httpClient->setResponseHandler(nullptr);
    }
    // std::cout << "$$$ file end: " << name << std::endl;

    return result;
}

void FileClient::downloadFolder(const std::string &foldername, const std::string &refoldername)
{
    // create a override folder
    if (refoldername.empty()) {
        CppCommon::Path path = CppCommon::Path(_savedir) / foldername;
        path = path.canonical(); // remove all '.' and '..' properly
        auto absapth = path.string();
        auto ok = createNotExistPath(absapth, false);
        if (!ok && absapth.empty()) {
            // FS exception hanppend
            _callback->onWebChanged(WEB_IO_ERROR, "fs_exception");
            return;
        }
    }

    // request get sub files's info
    auto info = requestInfo(foldername);

    // 循环处理文件夹内文件
    for (const auto &entry : info.datas) {
        if (_stop.load())
            break;

        std::string relName = foldername + "/" + entry.name;
        std::string repName = refoldername + "/" + entry.name;
        int64_t size = entry.size;
        if (size > 0) {
            // replace with the new folder if need
            downloadFile(relName, refoldername.empty() ? "" : repName);
        } else if (size < 0) {
            downloadFolder(relName, refoldername.empty() ? "" : repName);
        } else {
            // 链接文件或大小为0的空文件
            createNextAvailableName(refoldername.empty() ? relName : repName, true);
        }
    }
}

void FileClient::walkDownload(const std::vector<std::string> &webnames)
{
    sendInfobyHeader(INFO_WEB_START);
    _callback->onWebChanged(WEB_TRANS_START);

    for (const auto& name : webnames) {
        //std::cout << "start download web: " << name << std::endl;

        sendInfobyHeader(INFO_WEB_INDEX, name);
        _callback->onWebChanged(WEB_INDEX_BEGIN, name);

        // do not sure the file type: floder or file
        auto info = requestInfo(name);
        if (info.size == 0) {
            std::cout << name << " walkDownload requestInfo return NULL! " << std::endl;
            continue;
        }

        // file: size > 0; dir: size < 0; default size = 0
        if (info.size > 0) {
            downloadFile(name);
        } else {
            // check and create the first index dir, ex. a -> /xx/download/a(1)
            std::string replacePath = createNextAvailableName(name, false);
            if (replacePath.empty()) {
                // can not get a replace folder, skip.
                std::cout << name << "can not get a replace folder, skip!" << std::endl;
                continue;
            }
            std::string avainame = CppCommon::Path(replacePath).filename().string();
            std::string rename = (name == avainame) ? "" : avainame;
            // if need, change all sub files storage dir, e.x <save>/a : <save>/a(1)
            downloadFolder(name, rename);
        }

        if (_stop.load()) {
            std::cout << "User stop to download!" << std::endl;
            return;
        }
    }
    std::cout << "whole download finished!" << std::endl;

    sendInfobyHeader(INFO_WEB_FINISH);
    _callback->onWebChanged(WEB_TRANS_FINISH);
    std::cout << "whole download finished end thread!" << std::endl;
}

bool FileClient::createNotExistPath(std::string &abspath, bool isfile)
{
    CppCommon::Path path(abspath);
    try {
        if (!path.IsExists()) {
            // create its parent first.
            CppCommon::Directory::CreateTree(path.parent());
            if (isfile) {
                CppCommon::File::WriteEmpty(path);
            } else {
                CppCommon::Directory::Create(path);
            }

            return true;
        }
    } catch (const CppCommon::FileSystemException &ex) {
        // std::cout << "Create throw FS exception: " << abspath << std::endl;
#ifdef __linux__
        auto filename = path.filename().string();
        if (filename.length() >= NAME_MAX) {
            // 长文件名截取前后
            size_t catlen = NAME_MAX/3;
            std::string prefix = filename.substr(0, catlen);
            std::string suffix = filename.substr(filename.length() - catlen, catlen);

            filename = prefix + "..." + suffix;
            // std::cout << "rename to short: " << filename << std::endl;
            path = path.parent() / filename;
            std::string repath = path.string();
            auto check = createNotExistPath(repath, isfile);
            abspath = repath;
            return check;
        }
#endif
        // 捕获, 重置路径表明异常发生
        abspath = "";
        return false;
    }

    if (isfile && !path.IsDirectory()) {
        // empty file, use the exist one.
        CppCommon::File existFile(path);
        if (existFile.IsFileEmpty())
            return true;
    } else {
        // empty folder, use the exist one.
        CppCommon::Directory existDir(path);
        if (existDir.IsDirectoryEmpty())
            return true;
    }

    return false;
}

std::string FileClient::createNextAvailableName(const std::string &name, bool isfile)
{
    CppCommon::Path path = CppCommon::Path(_savedir) / name;
    path = path.canonical(); // remove all '.' and '..' properly

    // save file security check
    if (isfile) {
        try {
            // Redirect to Download folder if save into Home
            if (path.parent().IsEquivalent(path.home())) {
                std::cout << "Save dir is user Home, forbid! " << path.string() << std::endl;
                path = path.parent() / "Download" / path.filename();
            }
        } catch (const CppCommon::FileSystemException &ex) {
            // 捕获并处理异常
            // std::cout << "IsEquivalent throw FS exception: " << ex.message()<< std::endl;
        }
    }

    auto abspath = path.string();
    if (createNotExistPath(abspath, isfile)) {
        return abspath;
    }

    std::string filename = path.filename().string();
    size_t dotPos = filename.find_last_of(".");
    std::string prefix = (dotPos == std::string::npos) ? filename : filename.substr(0, dotPos);
    std::string suffix = (dotPos == std::string::npos) ? "" : filename.substr(dotPos);

    int i = 1;
    while (true) {
        std::string newfilename = prefix + "(" + std::to_string(i) + ")" + suffix;
        std::string newpath = CppCommon::Path(path.parent() / newfilename).string();
        if (createNotExistPath(newpath, isfile)) {
            return newpath;
        } else {
            if (newpath.empty()) {
                // FS exception hanppend
                return "";
            }
        }
        i++;
    }
}
