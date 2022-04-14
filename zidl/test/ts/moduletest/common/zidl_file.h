/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
 */

#ifndef OHOS_ZIDL_MODULE_TEST_ZIDL_FILE_H
#define OHOS_ZIDL_MODULE_TEST_ZIDL_FILE_H

#include <string>

namespace OHOS {
namespace Zidl {
namespace ModuleTest {
const std::string LICENSE_CONTENT =
"/*\n"
" * Copyright (C) 2021 Huawei Device Co., Ltd.\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");\n"
" * you may not use this file except in compliance with the License.\n"
" * You may obtain a copy of the License at\n"
" *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0\n"
" *\n"
" * Unless required by applicable law or agreed to in writing, software\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
" * See the License for the specific language governing permissions and\n"
" * limitations under the License.\n"
" */\n"
"\n";
const std::string UNKNOW_TYPE_ZIDL_NAME = "IZidlTest.zidl";

const std::string UNKNOW_TYPE_CASE1_ZIDL_CONTENT =
"interface zidl.systemtest.IZidlTest {\n"
"    void voidOrderOutChar([in] char param1, [out] char param2);\n"
"}";

const std::string UNKNOW_TYPE_CASE2_ZIDL_CONTENT =
"interface zidl.systemtest.IZidlTest {\n"
"    void voidOrderOutList([in] List<int> param1, [out] List<int> param2);\n"
"}";

const std::string UNKNOW_TYPE_CASE3_ZIDL_CONTENT =
"interface zidl.systemtest.IZidlTest {\n"
"    void voidOrderOutMapChar([in] Map<int, char> param1, [out] Map<int, char> param2);\n"
"}";

const std::string UNKNOW_TYPE_CASE4_ZIDL_CONTENT =
"interface zidl.systemtest.IZidlTest {\n"
"    void voidOrderOutMapList([in] Map<int, List<int>> param1, [out] Map<int, List<int>> param2);\n"
"}";

const std::string UNKNOW_TYPE_CASE5_ZIDL_CONTENT =
"interface zidl.systemtest.IZidlTest {\n"
"    char voidOrderReturnChar();\n"
"}";

const std::string UNKNOW_TYPE_CASE6_ZIDL_CONTENT =
"interface zidl.systemtest.IZidlTest {\n"
"    List<int> voidOrderReturnList();\n"
"}";

const std::string UNKNOW_TYPE_CASE7_ZIDL_CONTENT =
"interface zidl.systemtest.IZidlTest {\n"
"    Map<int, char> voidOrderReturnMapChar();\n"
"}";

const std::string UNKNOW_TYPE_CASE8_ZIDL_CONTENT =
"interface zidl.systemtest.IZidlTest {\n"
"    Map<int, List<int>> voidOrderReturnMapList();\n"
"}";

const std::string NORMAL_TYPE_ZIDL_CONTENT =
"interface zidl.systemtest.IInterfaceDeclare;"
"sequenceable zidl.systemtest.MySequenceable;"
"interface zidl.systemtest.IZidlTest {\n"
"    int voidOrderOutInt([in] int param1, [out] int param2);\n"
"}";
}
}
}
#endif  // OHOS_AAFWK_ABILITY_CONFIG_H