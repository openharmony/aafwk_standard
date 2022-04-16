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
 * Feature: zidl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterTest, Ready_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_ZIDL_NAME.c_str(), CAPITALIZATION_VOID_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", CAPITALIZATION_ZIDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
}

/*
 * Feature: zidl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterTest, Ready_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_ZIDL_NAME.c_str(), CAPITALIZATION_BOOL_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", CAPITALIZATION_ZIDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
}

/*
 * Feature: zidl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterTest, Ready_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_ZIDL_NAME.c_str(), CAPITALIZATION_INT_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", CAPITALIZATION_ZIDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
}

/*
 * Feature: zidl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterTest, Ready_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_ZIDL_NAME.c_str(), CAPITALIZATION_STRING_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", CAPITALIZATION_ZIDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
}

/*
 * Feature: zidl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterTest, Ready_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_ZIDL_NAME.c_str(), CAPITALIZATION_INTERFACE_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", CAPITALIZATION_ZIDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
}

/*
 * Feature: zidl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterTest, Ready_006, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_ZIDL_NAME.c_str(), CAPITALIZATION_IN_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", CAPITALIZATION_ZIDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
}

/*
 * Feature: zidl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterTest, Ready_007, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_ZIDL_NAME.c_str(), CAPITALIZATION_OUT_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", CAPITALIZATION_ZIDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
}

/*
 * Feature: zidl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterTest, Ready_008, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_ZIDL_NAME.c_str(), CAPITALIZATION_ONEWAY_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", CAPITALIZATION_ZIDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
}

/*
 * Feature: zidl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterTest, Ready_009, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_ZIDL_NAME.c_str(), CAPITALIZATION_SEQUENCEABLE_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", CAPITALIZATION_ZIDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
}

/*
 * Feature: zidl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: char type exists in .zidl(value type).
 */
HWTEST_F(JsCodeEmitterTest, Ready_010, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_ZIDL_NAME.c_str(), CAPITALIZATION_MAP_TYPE_ZIDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", CAPITALIZATION_ZIDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
}

}  // namespace ModuleTest
}  // namespace Idl
}  // namespace OHOS
