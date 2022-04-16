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
#include "idl_common.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Idl::TestCommon;

namespace OHOS {
namespace Idl {
namespace ModuleTest {
class JsCodeEmitterTest : public testing::Test, public IdlCommon {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void JsCodeEmitterTest::SetUpTestCase()
{}

void JsCodeEmitterTest::TearDownTestCase()
{}

void JsCodeEmitterTest::SetUp()
{}

void JsCodeEmitterTest::TearDown()
{}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(value type).
 */
HWTEST_F(JsCodeEmitterTest, CheckInterfaceType_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE1_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}
}  // namespace ModuleTest
}  // namespace Idl
}  // namespace OHOS
