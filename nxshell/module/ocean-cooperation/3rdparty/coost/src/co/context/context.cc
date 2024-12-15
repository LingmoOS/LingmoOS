/*!The Treasure Box Library
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright (C) 2009-present, TBOOX Open Source Group.
 *
 * @author      ruki
 * @file        context.c
 * @ingroup     platform
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "context.h"
#include "arch.h"
#include "co/log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
#if defined(DISABLE_GO)
tb_context_t tb_context_make(char* stackdata, size_t stacksize, tb_context_func_t func)
{
    CHECK(false) << "corutine 'go' NOT supported, use 'UNIGO' instead";
    return nullptr;
}
tb_context_from_t tb_context_jump(tb_context_t ctx, const void* priv)
{
    // noimpl
    CHECK(false) << "corutine 'go' NOT supported, use 'UNIGO' instead";

    // return emtry context
    tb_context_from_t from = {0};
    return from;
}
#endif

#ifdef __cplusplus
}
#endif
