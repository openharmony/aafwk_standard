/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Zidl {
namespace ModuleTest {
class JsCodeEmitterInterface : public testing::Test, public ZidlCommon {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void JsCodeEmitterInterface::SetUpTestCase()
{}

void JsCodeEmitterInterface::TearDownTestCase()
{}

void JsCodeEmitterInterface::SetUp()
{}

void JsCodeEmitterInterface::TearDown()
{}

/*
 * Feature: zidl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterInterface, CheckInterfaceType_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), UNKNOW_TYPE_CASE1_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: zidl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: list type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterInterface, CheckInterfaceType_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), UNKNOW_TYPE_CASE2_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: zidl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(Map<int, char>).
 */
HWTEST_F(JsCodeEmitterInterface, CheckInterfaceType_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), UNKNOW_TYPE_CASE3_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: zidl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: list type exists in .zidl(Map<int, List<int>>).
 */
HWTEST_F(JsCodeEmitterInterface, CheckInterfaceType_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), UNKNOW_TYPE_CASE4_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: zidl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(return value).
 */
HWTEST_F(JsCodeEmitterInterface, CheckInterfaceType_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), UNKNOW_TYPE_CASE5_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: zidl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: list type exists in .zidl(return value).
 */
HWTEST_F(JsCodeEmitterInterface, CheckInterfaceType_006, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), UNKNOW_TYPE_CASE6_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: zidl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(return Map<int, char>).
 */
HWTEST_F(JsCodeEmitterInterface, CheckInterfaceType_007, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), UNKNOW_TYPE_CASE7_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: zidl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(return Map<int, List<int>>).
 */
HWTEST_F(JsCodeEmitterInterface, CheckInterfaceType_008, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), UNKNOW_TYPE_CASE8_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: zidl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: normal type exists in .zidl(int).
 */
HWTEST_F(JsCodeEmitterInterface, CheckInterfaceType_009, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), NORMAL_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_TRUE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: zidl
 * Function: EmitInterfaceImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceImports is not empty.
 */
HWTEST_F(JsCodeEmitterInterface, EmitInterfaceImports_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareZidlFile(UNKNOW_TYPE_ZIDL_NAME.c_str(), NORMAL_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_ZIDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    jsCodeGen_->EmitInterfaceImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}
}  // namespace ModuleTest
}  // namespace Zidl
}  // namespace OHOS
