// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#include <string.h>
#include "specodec.h"
#include "utils.h"
#include "co/log.h"
#include "abstractdata.h"
#include "specdata.h"

namespace zrpc_ns {

static const char PB_START = 0x02; // start char
static const char PB_END = 0x03;   // end char
static const int MSG_REQ_LEN = 20; // default length of msg_req

ZRpcCodeC::ZRpcCodeC() {
}

ZRpcCodeC::~ZRpcCodeC() {
}

void ZRpcCodeC::encode(TcpBuffer *buf, AbstractData *data) {
    if (!buf || !data) {
        ELOG << "encode error! buf or data nullptr";
        return;
    }
    // DLOG << "test encode start";
    SpecDataStruct *tmp = dynamic_cast<SpecDataStruct *>(data);

    uint32_t len = 0;
    const char *re = encodePbData(tmp, len);
    if (re == nullptr || len == 0 || !tmp->encode_succ) {
        ELOG << "encode error";
        data->encode_succ = false;
        return;
    }
    // DLOG << "encode package len = " << len;
    if (buf != nullptr) {
        buf->writeToBuffer(re, len);
        // DLOG << "succ encode and write to buffer, writeindex=" << buf->writeIndex();
    }
    data = tmp;
    if (re) {
        free((void *)re);
        re = NULL;
    }
    // DLOG << "test encode end";
}

const char *ZRpcCodeC::encodePbData(SpecDataStruct *data, uint32_t &len) {
    if (data->service_full_name.empty()) {
        ELOG << "parse error, service_full_name is empty";
        data->encode_succ = false;
        return nullptr;
    }
    if (data->msg_req.empty()) {
        data->msg_req = Util::genMsgNumber();
        data->msg_req_len = data->msg_req.length();
        // DLOG << "generate msgno = " << data->msg_req;
    }

    uint32_t pk_len = 2 * sizeof(char) + 6 * sizeof(uint32_t) + data->pb_data.length() +
                     data->service_full_name.length() + data->msg_req.length() +
                     data->err_info.length();

    // DLOG << "encode pk_len = " << pk_len;
    char *buf = reinterpret_cast<char *>(malloc(pk_len));
    char *tmp = buf;
    *tmp = PB_START;
    tmp++;

    uint32_t pk_len_net = hton32(pk_len);
    memcpy(tmp, &pk_len_net, sizeof(uint32_t));
    tmp += sizeof(uint32_t);

    uint32_t msg_req_len = data->msg_req.length();
    // DLOG << "msg_req_len= " << msg_req_len;
    uint32_t msg_req_len_net = hton32(msg_req_len);
    memcpy(tmp, &msg_req_len_net, sizeof(uint32_t));
    tmp += sizeof(uint32_t);

    if (msg_req_len != 0) {

        memcpy(tmp, &(data->msg_req[0]), msg_req_len);
        tmp += msg_req_len;
    }

    uint32_t service_full_name_len = data->service_full_name.length();
    // DLOG << "src service_full_name_len = " << service_full_name_len;
    uint32_t service_full_name_len_net = hton32(service_full_name_len);
    memcpy(tmp, &service_full_name_len_net, sizeof(uint32_t));
    tmp += sizeof(uint32_t);

    if (service_full_name_len != 0) {
        memcpy(tmp, &(data->service_full_name[0]), service_full_name_len);
        tmp += service_full_name_len;
    }

    uint32_t err_code = data->err_code;
    // DLOG << "err_code= " << err_code;
    uint32_t err_code_net = hton32(err_code);
    memcpy(tmp, &err_code_net, sizeof(uint32_t));
    tmp += sizeof(uint32_t);

    uint32_t err_info_len = data->err_info.length();
    // DLOG << "err_info_len= " << err_info_len;
    uint32_t err_info_len_net = hton32(err_info_len);
    memcpy(tmp, &err_info_len_net, sizeof(uint32_t));
    tmp += sizeof(uint32_t);

    if (err_info_len != 0) {
        memcpy(tmp, &(data->err_info[0]), err_info_len);
        tmp += err_info_len;
    }

    memcpy(tmp, &(data->pb_data[0]), data->pb_data.length());
    tmp += data->pb_data.length();
    // DLOG << "pb_data_len= " << data->pb_data.length();

    int32_t checksum = 1;
    int32_t checksum_net = hton32(checksum);
    memcpy(tmp, &checksum_net, sizeof(int32_t));
    tmp += sizeof(int32_t);

    *tmp = PB_END;

    data->pk_len = pk_len;
    data->msg_req_len = msg_req_len;
    data->service_name_len = service_full_name_len;
    data->err_info_len = err_info_len;

    // checksum has not been implemented yet, directly skip chcksum
    data->check_num = checksum;
    data->encode_succ = true;

    len = pk_len;

    return buf;
}

void ZRpcCodeC::decode(TcpBuffer *buf, AbstractData *data) {

    if (!buf || !data) {
        ELOG << "decode error! buf or data nullptr";
        return;
    }

    std::vector<char> tmp = buf->getBufferVector();
    // int total_size = buf->readAble();
    int start_index = buf->readIndex();
    int end_index = -1;
    uint32_t pk_len = 0;

    bool parse_full_pack = false;

    for (int i = start_index; i < buf->writeIndex(); ++i) {
        // first find start
        if (tmp[i] == PB_START) {
            if (i + 1 < buf->writeIndex()) {
                pk_len = Util::netByteToInt32(&tmp[i + 1]);
                // DLOG << "prase pk_len =" << pk_len;
                int j = i + pk_len - 1;
                // DLOG << "j =" << j << ", i=" << i;

                if (j >= buf->writeIndex()) {
                    // DLOG << "recv package not complete, or pk_start find error, continue next
                    // parse";
                    continue;
                }
                if (tmp[j] == PB_END) {
                    start_index = i;
                    end_index = j;
                    // DLOG << "parse succ, now break";
                    parse_full_pack = true;
                    break;
                }
            }
        }
    }

    if (!parse_full_pack) {
        ELOG << "not parse full package, return";
        return;
    }

    buf->recycleRead(end_index + 1 - start_index);

    // DLOG << "m_read_buffer size=" << buf->getBufferVector().size() << "rd=" << buf->readIndex()
    //      << "wd=" << buf->writeIndex();

    SpecDataStruct *pb_struct = dynamic_cast<SpecDataStruct *>(data);
    pb_struct->pk_len = pk_len;
    pb_struct->decode_succ = false;

    int msg_req_len_index = start_index + sizeof(char) + sizeof(int32_t);
    if (msg_req_len_index >= end_index) {
        ELOG << "parse error, msg_req_len_index[" << msg_req_len_index << "] >= end_index["
             << end_index << "]";
        // drop this error package
        return;
    }

    pb_struct->msg_req_len = Util::netByteToInt32(&tmp[msg_req_len_index]);
    if (pb_struct->msg_req_len == 0) {
        ELOG << "prase error, msg_req emptr";
        return;
    }

    // DLOG << "msg_req_len= " << pb_struct->msg_req_len;
    int msg_req_index = msg_req_len_index + sizeof(int32_t);
    // DLOG << "msg_req_len_index= " << msg_req_index;

    char msg_req[50] = {0};

    memcpy(&msg_req[0], &tmp[msg_req_index], pb_struct->msg_req_len);
    pb_struct->msg_req = std::string(msg_req);
    // DLOG << "msg_req= " << pb_struct->msg_req;

    int service_name_len_index = msg_req_index + pb_struct->msg_req_len;
    if (service_name_len_index >= end_index) {
        ELOG << "parse error, service_name_len_index[" << service_name_len_index
             << "] >= end_index[" << end_index << "]";
        // drop this error package
        return;
    }

    // DLOG << "service_name_len_index = " << service_name_len_index;
    int service_name_index = service_name_len_index + sizeof(int32_t);

    if (service_name_index >= end_index) {
        ELOG << "parse error, service_name_index[" << service_name_index << "] >= end_index["
             << end_index << "]";
        return;
    }

    pb_struct->service_name_len = Util::netByteToInt32(&tmp[service_name_len_index]);

    if (pb_struct->service_name_len > pk_len) {
        ELOG << "parse error, service_name_len[" << pb_struct->service_name_len << "] >= pk_len ["
             << pk_len << "]";
        return;
    }
    // DLOG << "service_name_len = " << pb_struct->service_name_len;

    char service_name[512] = {0};

    memcpy(&service_name[0], &tmp[service_name_index], pb_struct->service_name_len);
    pb_struct->service_full_name = std::string(service_name);
    // DLOG << "service_name = " << pb_struct->service_full_name;

    int err_code_index = service_name_index + pb_struct->service_name_len;
    pb_struct->err_code = Util::netByteToInt32(&tmp[err_code_index]);

    int err_info_len_index = err_code_index + sizeof(int32_t);

    if (err_info_len_index >= end_index) {
        ELOG << "parse error, err_info_len_index[" << err_info_len_index << "] >= end_index["
             << end_index << "]";
        // drop this error package
        return;
    }
    pb_struct->err_info_len = Util::netByteToInt32(&tmp[err_info_len_index]);
    // DLOG << "err_info_len = " << pb_struct->err_info_len;
    int err_info_index = err_info_len_index + sizeof(int32_t);

    char err_info[512] = {0};

    memcpy(&err_info[0], &tmp[err_info_index], pb_struct->err_info_len);
    pb_struct->err_info = std::string(err_info);

    int pb_data_len = pb_struct->pk_len - pb_struct->service_name_len - pb_struct->msg_req_len -
                      pb_struct->err_info_len - 2 * sizeof(char) - 6 * sizeof(int32_t);

    int pb_data_index = err_info_index + pb_struct->err_info_len;
    // DLOG << "pb_data_len= " << pb_data_len << ", pb_index = " << pb_data_index;

    if (pb_data_index >= end_index) {
        ELOG << "parse error, pb_data_index[" << pb_data_index << "] >= end_index[" << end_index
             << "]";
        return;
    }
    // DLOG << "pb_data_index = " << pb_data_index << ", pb_data.length = " << pb_data_len;

    std::string pb_data_str(&tmp[pb_data_index], pb_data_len);
    pb_struct->pb_data = pb_data_str;

    // DLOG << "decode succ,  pk_len = " << pk_len << ", service_name = " <<
    // pb_struct->service_full_name;

    pb_struct->decode_succ = true;
    data = pb_struct;
}

} // namespace zrpc_ns
