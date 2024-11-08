/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#ifndef LIBKYPRINTER_H
#define LIBKYPRINTER_H

/**
 * @file libkyprinter.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 打印信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-hardware
 * @{
 * 
 */

//打印机状态
#define CUPS_PRINTER_STATE_BUSY 6       //"Busy"
#define CUPS_PRINTER_STATE_STOPPED 4    //"Stop"
#define CUPS_PRINTER_STATE_IDLE 2       //"Idle"
#define CUPS_PRINTER_STATE_PROCESSING 3 //"Processing"

//打印任务状态
#define IPP_JOB_PENDING	3               //挂起等待打印
#define IPP_JOB_HELD	4                 //等待打印
#define IPP_JOB_PROCESSING	5           //打印中
#define IPP_JOB_STOPPED	6               //停止
#define IPP_JOB_CANCELED	7             //取消
#define IPP_JOB_ABORTED	8               //由于错误跳过该打印任务
#define IPP_JOB_COMPLETED	9             //正常完成

//下载远程相关状态
#define URL_UNAVAILABLE	-5                
#define CAN_NOT_CHECK_URL -6
#define PATH_NOT_EXIST -7
#define PATH_NO_PERMISSION -8
#define PATH_ERROR -9

//下载状态
// typedef enum {
//   CURLE_OK = 0,
//   CURLE_UNSUPPORTED_PROTOCOL,    /* 1 */
//   CURLE_FAILED_INIT,             /* 2 */
//   CURLE_URL_MALFORMAT,           /* 3 */
//   CURLE_NOT_BUILT_IN,            /* 4 - [was obsoleted in August 2007 for
//                                     7.17.0, reused in April 2011 for 7.21.5] */
//   CURLE_COULDNT_RESOLVE_PROXY,   /* 5 */
//   CURLE_COULDNT_RESOLVE_HOST,    /* 6 */
//   CURLE_COULDNT_CONNECT,         /* 7 */
//   CURLE_WEIRD_SERVER_REPLY,      /* 8 */
//   CURLE_REMOTE_ACCESS_DENIED,    /* 9 a service was denied by the server
//                                     due to lack of access - when login fails
//                                     this is not returned. */
//   CURLE_FTP_ACCEPT_FAILED,       /* 10 - [was obsoleted in April 2006 for
//                                     7.15.4, reused in Dec 2011 for 7.24.0]*/
//   CURLE_FTP_WEIRD_PASS_REPLY,    /* 11 */
//   CURLE_FTP_ACCEPT_TIMEOUT,      /* 12 - timeout occurred accepting server
//                                     [was obsoleted in August 2007 for 7.17.0,
//                                     reused in Dec 2011 for 7.24.0]*/
//   CURLE_FTP_WEIRD_PASV_REPLY,    /* 13 */
//   CURLE_FTP_WEIRD_227_FORMAT,    /* 14 */
//   CURLE_FTP_CANT_GET_HOST,       /* 15 */
//   CURLE_HTTP2,                   /* 16 - A problem in the http2 framing layer.
//                                     [was obsoleted in August 2007 for 7.17.0,
//                                     reused in July 2014 for 7.38.0] */
//   CURLE_FTP_COULDNT_SET_TYPE,    /* 17 */
//   CURLE_PARTIAL_FILE,            /* 18 */
//   CURLE_FTP_COULDNT_RETR_FILE,   /* 19 */
//   CURLE_OBSOLETE20,              /* 20 - NOT USED */
//   CURLE_QUOTE_ERROR,             /* 21 - quote command failure */
//   CURLE_HTTP_RETURNED_ERROR,     /* 22 */
//   CURLE_WRITE_ERROR,             /* 23 */
//   CURLE_OBSOLETE24,              /* 24 - NOT USED */
//   CURLE_UPLOAD_FAILED,           /* 25 - failed upload "command" */
//   CURLE_READ_ERROR,              /* 26 - couldn't open/read from file */
//   CURLE_OUT_OF_MEMORY,           /* 27 */
//   /* Note: CURLE_OUT_OF_MEMORY may sometimes indicate a conversion error
//            instead of a memory allocation error if CURL_DOES_CONVERSIONS
//            is defined
//   */
//   CURLE_OPERATION_TIMEDOUT,      /* 28 - the timeout time was reached */
//   CURLE_OBSOLETE29,              /* 29 - NOT USED */
//   CURLE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
//   CURLE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
//   CURLE_OBSOLETE32,              /* 32 - NOT USED */
//   CURLE_RANGE_ERROR,             /* 33 - RANGE "command" didn't work */
//   CURLE_HTTP_POST_ERROR,         /* 34 */
//   CURLE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
//   CURLE_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
//   CURLE_FILE_COULDNT_READ_FILE,  /* 37 */
//   CURLE_LDAP_CANNOT_BIND,        /* 38 */
//   CURLE_LDAP_SEARCH_FAILED,      /* 39 */
//   CURLE_OBSOLETE40,              /* 40 - NOT USED */
//   CURLE_FUNCTION_NOT_FOUND,      /* 41 - NOT USED starting with 7.53.0 */
//   CURLE_ABORTED_BY_CALLBACK,     /* 42 */
//   CURLE_BAD_FUNCTION_ARGUMENT,   /* 43 */
//   CURLE_OBSOLETE44,              /* 44 - NOT USED */
//   CURLE_INTERFACE_FAILED,        /* 45 - CURLOPT_INTERFACE failed */
//   CURLE_OBSOLETE46,              /* 46 - NOT USED */
//   CURLE_TOO_MANY_REDIRECTS,      /* 47 - catch endless re-direct loops */
//   CURLE_UNKNOWN_OPTION,          /* 48 - User specified an unknown option */
//   CURLE_TELNET_OPTION_SYNTAX,    /* 49 - Malformed telnet option */
//   CURLE_OBSOLETE50,              /* 50 - NOT USED */
//   CURLE_OBSOLETE51,              /* 51 - NOT USED */
//   CURLE_GOT_NOTHING,             /* 52 - when this is a specific error */
//   CURLE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
//   CURLE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
//                                     default */
//   CURLE_SEND_ERROR,              /* 55 - failed sending network data */
//   CURLE_RECV_ERROR,              /* 56 - failure in receiving network data */
//   CURLE_OBSOLETE57,              /* 57 - NOT IN USE */
//   CURLE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
//   CURLE_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
//   CURLE_PEER_FAILED_VERIFICATION, /* 60 - peer's certificate or fingerprint
//                                      wasn't verified fine */
//   CURLE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized/bad encoding */
//   CURLE_LDAP_INVALID_URL,        /* 62 - Invalid LDAP URL */
//   CURLE_FILESIZE_EXCEEDED,       /* 63 - Maximum file size exceeded */
//   CURLE_USE_SSL_FAILED,          /* 64 - Requested FTP SSL level failed */
//   CURLE_SEND_FAIL_REWIND,        /* 65 - Sending the data requires a rewind
//                                     that failed */
//   CURLE_SSL_ENGINE_INITFAILED,   /* 66 - failed to initialise ENGINE */
//   CURLE_LOGIN_DENIED,            /* 67 - user, password or similar was not
//                                     accepted and we failed to login */
//   CURLE_TFTP_NOTFOUND,           /* 68 - file not found on server */
//   CURLE_TFTP_PERM,               /* 69 - permission problem on server */
//   CURLE_REMOTE_DISK_FULL,        /* 70 - out of disk space on server */
//   CURLE_TFTP_ILLEGAL,            /* 71 - Illegal TFTP operation */
//   CURLE_TFTP_UNKNOWNID,          /* 72 - Unknown transfer ID */
//   CURLE_REMOTE_FILE_EXISTS,      /* 73 - File already exists */
//   CURLE_TFTP_NOSUCHUSER,         /* 74 - No such user */
//   CURLE_CONV_FAILED,             /* 75 - conversion failed */
//   CURLE_CONV_REQD,               /* 76 - caller must register conversion
//                                     callbacks using curl_easy_setopt options
//                                     CURLOPT_CONV_FROM_NETWORK_FUNCTION,
//                                     CURLOPT_CONV_TO_NETWORK_FUNCTION, and
//                                     CURLOPT_CONV_FROM_UTF8_FUNCTION */
//   CURLE_SSL_CACERT_BADFILE,      /* 77 - could not load CACERT file, missing
//                                     or wrong format */
//   CURLE_REMOTE_FILE_NOT_FOUND,   /* 78 - remote file not found */
//   CURLE_SSH,                     /* 79 - error from the SSH layer, somewhat
//                                     generic so the error message will be of
//                                     interest when this has happened */

//   CURLE_SSL_SHUTDOWN_FAILED,     /* 80 - Failed to shut down the SSL
//                                     connection */
//   CURLE_AGAIN,                   /* 81 - socket is not ready for send/recv,
//                                     wait till it's ready and try again (Added
//                                     in 7.18.2) */
//   CURLE_SSL_CRL_BADFILE,         /* 82 - could not load CRL file, missing or
//                                     wrong format (Added in 7.19.0) */
//   CURLE_SSL_ISSUER_ERROR,        /* 83 - Issuer check failed.  (Added in
//                                     7.19.0) */
//   CURLE_FTP_PRET_FAILED,         /* 84 - a PRET command failed */
//   CURLE_RTSP_CSEQ_ERROR,         /* 85 - mismatch of RTSP CSeq numbers */
//   CURLE_RTSP_SESSION_ERROR,      /* 86 - mismatch of RTSP Session Ids */
//   CURLE_FTP_BAD_FILE_LIST,       /* 87 - unable to parse FTP file list */
//   CURLE_CHUNK_FAILED,            /* 88 - chunk callback reported error */
//   CURLE_NO_CONNECTION_AVAILABLE, /* 89 - No connection available, the
//                                     session will be queued */
//   CURLE_SSL_PINNEDPUBKEYNOTMATCH, /* 90 - specified pinned public key did not
//                                      match */
//   CURLE_SSL_INVALIDCERTSTATUS,   /* 91 - invalid certificate status */
//   CURLE_HTTP2_STREAM,            /* 92 - stream error in HTTP/2 framing layer
//                                     */
//   CURLE_RECURSIVE_API_CALL,      /* 93 - an api function was called from
//                                     inside a callback */
//   CURLE_AUTH_ERROR,              /* 94 - an authentication function returned an
//                                     error */
//   CURLE_HTTP3,                   /* 95 - An HTTP/3 layer problem */
//   CURL_LAST /* never use! */
// } CURLcode;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取本地的所有打印机(包含可用打印机和不可用打印机)
 * 
 * @return char** 打印机名称列表，由NULL字符串表示结尾；由alloc生成，需要被kdk_printer_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_printer_get_list();

/**
 * @brief 获取本地的所有可用打印机(打印机处于idle状态)
 * 
 * @return char** 打印机名称列表，由NULL字符串表示结尾；由alloc生成，需要被kdk_printer_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_printer_get_available_list();


/**
 * @brief 设置打印参数，每次设置会清除之前设置打印参数,这个设置是全局设置，设置了之后所有打印机都会用这个参数进行打印
 * 
 * @param printername       打印机名，kdk_printer_get_list返回的值
 * @param number_up         cups属性，一张纸打印几页，如2 4等
 * @param media             cups属性，纸张类型，如A4
 * @param number_up_layout  cups属性，如lrtb
 * @param sides             cups属性，单面如one-sided，双面如two-sided-long-edge(长边翻转),two-sided-short-edge(短边翻转)
 * 
 * @return 无
 */
extern void kdk_printer_set_options(int number_up,const char *media,const char *number_up_layout,const char *sides);

/**
 * @brief 下载网络文件
 * 
 * @param url               要打印的url
 * @param filepath          保存的文件路径
 * 
 * @return 打印作业的id，失败返回-1
 * 
 */
extern int kdk_printer_print_download_remote_file(const char *url, const char *filepath);

/**
 * @brief 打印本地文件
 * 
 * @param printername       打印机名，kdk_printer_get_list返回的值
 * @param filepath          文件绝对路径
 * 
 * @return 打印作业的id
 * 
 * @notice 虽然支持多种格式的打印，但是打印除pdf之外的格式打印的效果都不够好，建议打印pdf格式的文件
 *         打印机处于stop状态函数返回0
 *         格式不对的文件可以成功创建打印任务，但是打印不出来。什么叫格式不对，举个例子，比如当前要打印日志文件a.log,
 *         然后把a.log改名叫a.pdf,这个时候a.pdf就变成了格式不对的文件，打印不出来了
 * 
 */
extern int kdk_printer_print_local_file(const char *printername, const char *filepath);

/**
 * @brief 清除某个打印机的所有打印队列
 * 
 * @param printername       打印机名，kdk_printer_get_list返回的值
 * 
 * @return 成功返回0,失败返回-1
 * 
 * @notice 同kdk_printer_print_local_file
 */
extern int kdk_printer_cancel_all_jobs(const char *printername);

/**
 * @brief  获取当前打印机状态(状态不是实时更新)
 * 
 * @param  printername      打印机名，kdk_printer_get_list返回的值
 * 
 * @return  状态码
 */
extern int kdk_printer_get_status(const char *printername);

/**
 * @brief  获取当前打印任务状态（发送打印任务之后下需要等待一会再获取状态,状态不是实时更新）
 * 
 * @param  printername      打印机名，kdk_printer_get_list返回的值
 * 
 * @return  状态码
 */
extern int kdk_printer_get_job_status(const char *printername, int jobid);

/**
 * @brief  获取url内的文件名
 * 
 * @param  url     
 * 
 * @return 解析出来的函数名
 */
extern char* kdk_printer_get_filename(const char *url);

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 */
extern inline void kdk_printer_freeall(char **ptr);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/hardware/test/kyprinterlist-test.c
 * \example lingmosdk-system/src/hardware/test/kyprinterprint-test.c
 * 
 */

/**
  * @}
  */
