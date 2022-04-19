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
class TsCodeEmitterTest : public testing::Test, public IdlCommon {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void TsCodeEmitterTest::SetUpTestCase()
{}

void TsCodeEmitterTest::TearDownTestCase()
{}

void TsCodeEmitterTest::SetUp()
{}

void TsCodeEmitterTest::TearDown()
{}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no license test
 * EnvConditions: NA
 * CaseDescription: no license and void type resolution succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_LICENSE_IDL_NAME.c_str(), NO_LICENSE_VOID_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_LICENSE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no license test
 * EnvConditions: NA
 * CaseDescription: no license and bool type resolution succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_LICENSE_IDL_NAME.c_str(), NO_LICENSE_BOOL_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_LICENSE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no license test
 * EnvConditions: NA
 * CaseDescription: no license and return int type resolution succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_LICENSE_IDL_NAME.c_str(), NO_LICENSE_INT_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_LICENSE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no license test
 * EnvConditions: NA
 * CaseDescription: no license and return string resolution succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_LICENSE_IDL_NAME.c_str(), NO_LICENSE_STRING_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_LICENSE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no license test
 * EnvConditions: NA
 * CaseDescription: no license and interface type resolution succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_LICENSE_IDL_NAME.c_str(), NO_LICENSE_INTERFACE_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_LICENSE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no license test
 * EnvConditions: NA
 * CaseDescription: no license and in keywords resolution succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_006, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_LICENSE_IDL_NAME.c_str(), NO_LICENSE_IN_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_LICENSE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no license test
 * EnvConditions: NA
 * CaseDescription: no license and out keywords resolution succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_007, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_LICENSE_IDL_NAME.c_str(), NO_LICENSE_OUT_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_LICENSE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no license test
 * EnvConditions: NA
 * CaseDescription: no license and oneway keywords resolution succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_008, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_LICENSE_IDL_NAME.c_str(), NO_LICENSE_ONEWAY_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_LICENSE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no license test
 * EnvConditions: NA
 * CaseDescription: no license and sequenceable type resolution succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_009, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(
        NO_LICENSE_IDL_NAME.c_str(), NO_LICENSE_SEQUENCEABLE_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_LICENSE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no license test
 * EnvConditions: NA
 * CaseDescription: no license and map type resolution succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_010, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_LICENSE_IDL_NAME.c_str(), NO_LICENSE_MAP_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_LICENSE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no interface test
 * EnvConditions: NA
 * CaseDescription: no interface and void type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_011, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_INTERFACE_IDL_NAME.c_str(), NO_INTERFACE_VOID_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_INTERFACE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no interface test
 * EnvConditions: NA
 * CaseDescription: no interface and bool type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_012, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_INTERFACE_IDL_NAME.c_str(), NO_INTERFACE_BOOL_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_INTERFACE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no interface test
 * EnvConditions: NA
 * CaseDescription: no interface and int type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_013, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_INTERFACE_IDL_NAME.c_str(), NO_INTERFACE_INT_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_INTERFACE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no interface test
 * EnvConditions: NA
 * CaseDescription: no interface and string type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_014, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_INTERFACE_IDL_NAME.c_str(), NO_INTERFACE_STRING_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_INTERFACE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no interface test
 * EnvConditions: NA
 * CaseDescription: no interface and interface type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_015, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(
        NO_INTERFACE_IDL_NAME.c_str(), NO_INTERFACE_INTERFACE_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_INTERFACE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no interface test
 * EnvConditions: NA
 * CaseDescription: no interface and in keywords parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_016, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_INTERFACE_IDL_NAME.c_str(), NO_INTERFACE_IN_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_INTERFACE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no interface test
 * EnvConditions: NA
 * CaseDescription: no interface and out keywords parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_017, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_INTERFACE_IDL_NAME.c_str(), NO_INTERFACE_OUT_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_INTERFACE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no interface test
 * EnvConditions: NA
 * CaseDescription: no interface and oneway keywords parsing failed.
 */
HWTEST_F(TsCodeEmitterTest, Ready_018, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_INTERFACE_IDL_NAME.c_str(), NO_INTERFACE_ONEWAY_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_INTERFACE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no interface test
 * EnvConditions: NA
 * CaseDescription: no interface and sequenceable type parsing failed.
 */
HWTEST_F(TsCodeEmitterTest, Ready_019, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(
        NO_INTERFACE_IDL_NAME.c_str(), NO_INTERFACE_SEQUENCEABLE_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_INTERFACE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: no interface test
 * EnvConditions: NA
 * CaseDescription: no interface and map type parsing failed.
 */
HWTEST_F(TsCodeEmitterTest, Ready_020, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(NO_INTERFACE_IDL_NAME.c_str(), NO_INTERFACE_MAP_TYPE_IDL_CONTENT.c_str(), ""), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", NO_INTERFACE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: Uppercase void type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_021, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_IDL_NAME.c_str(), CAPITALIZATION_VOID_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", CAPITALIZATION_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: Uppercase bool type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_022, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_IDL_NAME.c_str(), CAPITALIZATION_BOOL_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", CAPITALIZATION_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: Uppercase int type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_023, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_IDL_NAME.c_str(), CAPITALIZATION_INT_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", CAPITALIZATION_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: Uppercase string type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_024, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_IDL_NAME.c_str(), CAPITALIZATION_STRING_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", CAPITALIZATION_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: Uppercase interface type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_025, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(
        CAPITALIZATION_IDL_NAME.c_str(), CAPITALIZATION_INTERFACE_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", CAPITALIZATION_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: Uppercase in keywords parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_026, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_IDL_NAME.c_str(), CAPITALIZATION_IN_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", CAPITALIZATION_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: Uppercase out keywords parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_027, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_IDL_NAME.c_str(), CAPITALIZATION_OUT_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", CAPITALIZATION_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: Uppercase oneway keywords parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_028, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_IDL_NAME.c_str(), CAPITALIZATION_ONEWAY_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", CAPITALIZATION_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: Uppercase sequenceable type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_029, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(
        CAPITALIZATION_IDL_NAME.c_str(), CAPITALIZATION_SEQUENCEABLE_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", CAPITALIZATION_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect keyword capitalization issues test
 * EnvConditions: NA
 * CaseDescription: Uppercase map type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_030, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(CAPITALIZATION_IDL_NAME.c_str(), CAPITALIZATION_MAP_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", CAPITALIZATION_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: in keyword after type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_031, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), IN_KEYWORD_AFTER_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: not equal to filename parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_032, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), NOT_EQUAL_TO_FILENAME_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: The file name does not match the interface name
 */
HWTEST_F(TsCodeEmitterTest, Ready_033, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_SCENE_IDL_NAME.c_str(), ABNORMAL_SCENE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_SCENE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: unsigned char type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_034, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), UNSIGNED_CHAR_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: unsigned short type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_035, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), UNSIGNED_SHORT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: unsigned int type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_036, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), UNSIGNED_INT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: unsigned long type parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_037, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), UNSIGNED_LONG_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: empty content parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_038, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), EMPTY_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: no return value parsing failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_039, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), NOT_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: No function name resolution failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_040, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), NOT_FUNCTION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: Same function name prasing successed
 */
HWTEST_F(TsCodeEmitterTest, Ready_041, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), SAME_FUNCTION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: The function name is too long and the parsing is successful
 */
HWTEST_F(TsCodeEmitterTest, Ready_042, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), FUNCTION_NAME_TOO_LONG_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: abnormal scene test
 * EnvConditions: NA
 * CaseDescription: The variable name is too long and the parsing succeeded
 */
HWTEST_F(TsCodeEmitterTest, Ready_043, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(I_ABNORMAL_IDL_NAME.c_str(), VARIABLE_NAME_TOO_LONG_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", I_ABNORMAL_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}


/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect namespace issues test
 * EnvConditions: NA
 * CaseDescription: Use multiple namespaces failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_044, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), MULTIPLE_NAMESPACES_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect Keyword grammar problems Test
 * EnvConditions: NA
 * CaseDescription: Keyword 'in' 'out' sepatate failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_045, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), INOUT_SEPARATE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect Keyword grammar problems Test
 * EnvConditions: NA
 * CaseDescription: Interface no content failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_046, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), INTERFACE_NO_CONTENT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect Keyword grammar problems Test
 * EnvConditions: NA
 * CaseDescription: Paramter no keyword 'in' 'out' failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_047, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), PARAM_NO_INOUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}

/*
 * Feature: idl
 * Function: Ready
 * SubFunction: NA
 * FunctionPoints: Detect Keyword grammar problems Test
 * EnvConditions: NA
 * CaseDescription: Keyword 'in' 'out' not use big parentheses failed
 */
HWTEST_F(TsCodeEmitterTest, Ready_048, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), INOUT_NO_BIGPARANTHESES_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_NE(Ready(argc, parameters.GetArgv()), ERR_OK);
}
}  // namespace ModuleTest
}  // namespace Idl
}  // namespace OHOS
