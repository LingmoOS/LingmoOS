// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_ABSTRACTDATA_H
#define ZRPC_ABSTRACTDATA_H

namespace zrpc_ns {

class AbstractData {
 public:
  AbstractData() = default;
  virtual ~AbstractData() {}

  bool decode_succ {false};
  bool encode_succ {false};
};


}

#endif
