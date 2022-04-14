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

#include <gtest/gtest.h>
#include "zidl_common.h"

using namespace testing::ext;
namespace OHOS {
namespace Zidl {
namespace ModuleTest {
class JsCodeEmitterProxyTest : public testing::Test, public ZidlCommon {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void JsCodeEmitterProxyTest::SetUpTestCase(void)
{}
void JsCodeEmitterProxyTest::TearDownTestCase(void)
{}
void JsCodeEmitterProxyTest::SetUp()
{}
void JsCodeEmitterProxyTest::TearDown()
{}

/*
 * Feature: zidl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterProxyTest, CheckInterfaceType_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), UNKNOW_TYPE_CASE1_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}
}
}
}
