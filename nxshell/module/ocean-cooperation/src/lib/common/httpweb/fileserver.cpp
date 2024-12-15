// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileserver.h"
#include "tokencache.h"
#include "webbinder.h"

#include "server/http/https_session.h"
#include "server/http/http_response.h"

#include "webproto.h"

class HTTPFileSession : public CppServer::HTTP::HTTPSSession
{
public:
    using CppServer::HTTP::HTTPSSession::HTTPSSession;

    void setResponseHandler(ResponseHandler cb)
    {
        _handler = std::move(cb);
    }

protected:
    InfoEntry putFileInfo(const CppCommon::Path &entry)
    {
        InfoEntry info;
        auto name = entry.filename().string();
        info.name = name;
        if (entry.IsDirectory()) {
            info.size = -1; // mask as folder flag.
        } else if (entry.IsRegularFile()) {
            CppCommon::File temp(entry);
            info.size = temp.size(); // mask as file flag.
        } else {
            std::cout << "this is link file: " << entry.string() << std::endl;
        }

        return info;
    }

    void serveInfo(const CppCommon::Path &path)
    {
        CppCommon::File info(path);
        if (info.IsExists()) {
            InfoEntry fileInfo = putFileInfo(info);
            if (info.IsDirectory()) {
                for (const auto &item : CppCommon::Directory(path)) {
                    const CppCommon::Path entry = item.IsSymlink() ? CppCommon::Symlink(item).target() : item;

                    InfoEntry subInfo = putFileInfo(entry);
                    fileInfo.datas.push_back(subInfo);
                }
            }

            std::string json_str = fileInfo.as_json().serialize();
            //std::cout << "serveInfo >>: " << json_str << std::endl;

            // Response with all dir/file values
            SendResponseAsync(response().MakeGetResponse(json_str, "application/json; charset=UTF-8"));
        } else {
            SendResponseAsync(response().MakeErrorResponse(404, "Not found."));
        }
    }

    void serveContent(const CppCommon::Path &path, size_t offset)
    {
        CppCommon::File info(path);
        if (info.IsExists()) {
            response().Clear();
            response().SetBegin(200);

            if (info.IsDirectory()) {
                response().SetBodyLength(0);
                response().SetBody("");

                SendResponseAsync(response());
            } else if (info.IsRegularFile()){
                info.Open(true, false);
                uint64_t total = 0;

                size_t sz = info.size();
                if (offset > 0 && offset < sz) {
                    //std::cout << "breakpoint continue transfer from:" << offset << std::endl;
                    info.Seek(offset); // seek to offset for breakpoint continue
                }

                response().SetContentType(info.extension().string());
                response().SetBodyLength(sz - offset); // set the remaining size as body lenght

                total = response().body_length();

                // send headers first
                SendResponse(response());

                _handler(RES_OKHEADER, info.string().data(), total);

                bool cancel = false;
                size_t read_sz = 0;
                char buff[BLOCK_SIZE];
                while (!cancel) {
                    memset(buff, 0, BLOCK_SIZE);
                    read_sz = info.Read(buff, sizeof(buff));
                    if (read_sz <= 0) {
                        break;
                    }
                    SendResponseBody(buff, read_sz);
                    // notify progress：size total
                    // return true to cancel download from outside.
                    cancel = _handler(RES_BODY, nullptr, read_sz);
                };

                info.Close();

                _handler(RES_FINISH, info.string().data(), total);
            } else {
                std::cout << "this is link file: " << path.absolute() << std::endl;
            }
        } else {
            SendResponseAsync(response().MakeErrorResponse(404, "Not found."));
            _handler(RES_NOTFOUND, info.string().data(), 0);
        }

        //std::cout << "response body end:" << total << std::endl;
    }

    // 解析URL中的query参数
    std::unordered_map<std::string, std::string> parseQueryParams(const std::string &query)
    {
        std::unordered_map<std::string, std::string> params;

        size_t start = 0;
        size_t end = 0;

        while ((end = query.find('&', start)) != std::string::npos) {
            std::string param = query.substr(start, end - start);
            size_t eqPos = param.find('=');
            if (eqPos != std::string::npos) {
                params[param.substr(0, eqPos)] = param.substr(eqPos + 1);
            }
            start = end + 1;
        }

        std::string lastParam = query.substr(start);
        size_t eqPos = lastParam.find('=');
        if (eqPos != std::string::npos) {
            params[lastParam.substr(0, eqPos)] = lastParam.substr(eqPos + 1);
        }

        return params;
    }

    void onReceivedRequest(const CppServer::HTTP::HTTPRequest &request) override
    {
        // Show HTTP request content
        //std::cout << std::endl << request;

        // Process HTTP request methods
        if (request.method() == "HEAD") {
            // [HEAD]webstart>|webfinish>|webindex><name>
            std::string key = std::string(request.url());
            // std::cout << "------------HEADER:" << key << std::endl;

            size_t dePos = key.find('>');
            if (dePos == std::string::npos || dePos + 1 > key.size()) {
                // std::cout << "------------HEADER:" << key << " size:" << dePos + 1 << std::endl;
                // error format request!
                SendResponseAsync(response().MakeHeadResponse());
                return;
            }

            std::string method = key.substr(0, dePos);
            if (method == s_headerInfos[INFO_WEB_START]) {
                _handler(RES_WEB_START, nullptr, 0);
            } else if (method == s_headerInfos[INFO_WEB_FINISH]) {
                _handler(RES_WEB_FINISH, nullptr, 0);
            } else if (method == s_headerInfos[INFO_WEB_INDEX]) {
                std::string name = key.substr(dePos + 1);
                CppCommon::Path diskpath = WebBinder::GetInstance().getPath(name);
                if (diskpath.empty()) {
                    std::cout << "webindex : " << name << " > " << diskpath.string() << std::endl;
                    SendResponseAsync(response().MakeErrorResponse(404, "Not found."));
                    return;
                }

                bool found = WebBinder::GetInstance().containWeb(key);
                if (found) {
                    // this web index changed
                    _handler(RES_INDEX_CHANGE, diskpath.string().data(), 0);
                }
            }

            SendResponseAsync(response().MakeHeadResponse());
        } else if (request.method() == "GET") {
            // std::string url = "info/pathname&token=xxx";
            // std::string url = "download/pathname&token=xxx&offset=xxx";
            std::string url = std::string(request.url());

            size_t pathEnd = url.find("&token");
            std::string path = url.substr(0, pathEnd);

            size_t queryStart = url.find("&token") + 1;
            std::string query = url.substr(queryStart);

            std::unordered_map<std::string, std::string> queryParams = parseQueryParams(query);
            std::string ename = path.substr(path.find('/') + 1);
            // dcode realname from base64, which may include '&'
            std::string name = CppCommon::Encoding::Base64Decode(ename);
            std::string token = queryParams["token"];

            std::string method = path.substr(0, path.find('/'));

            // 检查token是否正确
            if (TokenCache::GetInstance().verifyToken(token)) {
                CppCommon::Path diskpath = WebBinder::GetInstance().getPath(name);

                if (diskpath.empty()) {
                    std::cout << "request >> name: " << name << " > " << diskpath.string() << std::endl;
                    SendResponseAsync(response().MakeErrorResponse(404, "Not found."));
                    return;
                }

                // 处理predownload或download请求的name
                if (method.find("info") != std::string::npos) {
                    // 处理predownload请求的name
                    serveInfo(diskpath);
                } else if (method.find("download") != std::string::npos) {
                    // 处理download请求的name
                    std::string offstr = queryParams["offset"];
                    size_t offset = 0;
                    if (!offstr.empty()) {
                        offset = std::stoll(offstr);
                    }

                    serveContent(diskpath, offset);
                } else {
                    SendResponseAsync(response().MakeErrorResponse("Unsupported HTTP request: " + method));
                }
            } else {
                std::cout << "Token invalid" << std::endl;
                // Response reject
                SendResponseAsync(response().MakeErrorResponse(404, "Invalid auth token!"));
            }
        } else
            SendResponseAsync(response().MakeErrorResponse("Unsupported HTTP method: " + std::string(request.method())));
    }

    void onReceivedRequestError(const CppServer::HTTP::HTTPRequest &request, const std::string &error) override
    {
        std::cout << "Request error: " << error << std::endl;
        _handler(RES_ERROR, nullptr, 0);
    }

    void onError(int error, const std::string &category, const std::string &message) override
    {
//        std::cout << "HTTP session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
        _handler(RES_ERROR, nullptr, 0);
    }

private:
    ResponseHandler _handler { nullptr };
};

std::shared_ptr<CppServer::Asio::SSLSession>
FileServer::CreateSession(const std::shared_ptr<CppServer::Asio::SSLServer> &server)
{
    ResponseHandler cb([this](int status, const char *buffer, uint64_t size) -> bool {
        if (_callback) {
            if (RES_OKHEADER == status) {
                std::string path(buffer);
                _callback->onWebChanged(WEB_FILE_BEGIN, path, size);
            } else if (RES_BODY == status) {
                // return true to canceled from outside.
                return _callback->onProgress(size);
            } else if (RES_FINISH == status) {
                std::string path(buffer);
                _callback->onWebChanged(WEB_FILE_END, path, size);
            } else if (RES_NOTFOUND == status) {
                std::string path(buffer);
                _callback->onWebChanged(WEB_NOT_FOUND, "not_found");
            } else if (RES_ERROR == status) {
                _callback->onWebChanged(WEB_DISCONNECTED, "net_error");
            } else if (RES_INDEX_CHANGE == status) {
                std::string path(buffer);
                _callback->onWebChanged(WEB_INDEX_BEGIN, path);
            } else if (RES_WEB_START == status) {
                _callback->onWebChanged(WEB_TRANS_START);
            } else if (RES_WEB_FINISH == status) {
                _callback->onWebChanged(WEB_TRANS_FINISH);
            }
        }
        return _stop.load();
    });

    auto session = std::make_shared<HTTPFileSession>(std::dynamic_pointer_cast<CppServer::HTTP::HTTPSServer>(server));
    session->setResponseHandler(std::move(cb));
    return session;
}

void FileServer::onError(int error, const std::string &category, const std::string &message)
{
    std::cout << "HTTP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
}

bool FileServer::start()
{
    _stop.store(false);
    SetupReuseAddress(true);
    SetupReusePort(true);
    return IsStarted() ? Restart() : Start();
}

bool FileServer::stop()
{
    _stop.store(true);
    return Stop();
}

// bind: "/images", "C:/Users/username/Pictures/images"
// getpath(): return
//    "/images/photo.jpg" -> C:/Users/username/Pictures/images/photo.jpg
//    "/images/2022/" -> C:/Users/username/Pictures/images/2022/

// bind: "/images/*", "C:/Users/username/Pictures/"
// getpath(): return
//    "/images/photo.jpg" -> C:/Users/username/Pictures/photo.jpg
//    "/images/2022/" -> C:/Users/username/Pictures/2022/
int FileServer::webBind(std::string webDir, std::string diskDir)
{
    int bind =  WebBinder::GetInstance().bind(webDir, diskDir);
    if (bind == -1) throw std::invalid_argument("Web binding exists.");
    if (bind == -2) throw std::invalid_argument("Not a valid web path.");
    if (bind == -3) throw std::invalid_argument("Not a valid disk path.");
    if (bind == -4) throw std::invalid_argument("Not a valid combinaton of web and disk path.");
    return bind;
}

int FileServer::webUnbind(std::string webDir)
{
    return WebBinder::GetInstance().unbind(webDir);
}

void FileServer::clearBind()
{
    WebBinder::GetInstance().clear();
}

std::string FileServer::genToken(std::string info)
{
    return TokenCache::GetInstance().genToken(info);
}

bool FileServer::verifyToken(std::string &token)
{
    return TokenCache::GetInstance().verifyToken(token);
}
