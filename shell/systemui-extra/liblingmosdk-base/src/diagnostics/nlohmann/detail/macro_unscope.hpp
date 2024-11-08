/*
 * MIT License 

 * Copyright (c) 2013-2022 Niels Lohmann

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

// restore clang diagnostic settings
#if defined(__clang__)
    #pragma clang diagnostic pop
#endif

// clean up
#undef JSON_ASSERT
#undef JSON_INTERNAL_CATCH
#undef JSON_CATCH
#undef JSON_THROW
#undef JSON_TRY
#undef JSON_PRIVATE_UNLESS_TESTED
#undef JSON_HAS_CPP_11
#undef JSON_HAS_CPP_14
#undef JSON_HAS_CPP_17
#undef JSON_HAS_CPP_20
#undef JSON_HAS_FILESYSTEM
#undef JSON_HAS_EXPERIMENTAL_FILESYSTEM
#undef NLOHMANN_BASIC_JSON_TPL_DECLARATION
#undef NLOHMANN_BASIC_JSON_TPL
#undef JSON_EXPLICIT
#undef NLOHMANN_CAN_CALL_STD_FUNC_IMPL

#include <nlohmann/thirdparty/hedley/hedley_undef.hpp>
