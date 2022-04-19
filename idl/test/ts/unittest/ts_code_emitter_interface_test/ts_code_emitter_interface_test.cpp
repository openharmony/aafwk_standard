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
namespace UnitTest {
class TsCodeEmitterInterfaceTest : public testing::Test, public IdlCommon {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void TsCodeEmitterInterfaceTest::SetUpTestCase()
{}

void TsCodeEmitterInterfaceTest::TearDownTestCase()
{}

void TsCodeEmitterInterfaceTest::SetUp()
{}

void TsCodeEmitterInterfaceTest::TearDown()
{}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(value type).
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE1_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_FALSE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: list type exists in .idl(value type).
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE2_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_FALSE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(Map<int, char>).
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE3_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_FALSE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: list type exists in .idl(Map<int, List<int>>).
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE4_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_FALSE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(return value).
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE5_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_FALSE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: list type exists in .idl(return value).
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_006, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE6_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_FALSE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(return Map<int, char>).
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_007, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE7_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_FALSE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(return Map<int, List<int>>).
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_008, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE8_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_FALSE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: normal type exists in .idl(int).
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_009, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_TRUE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: normal type in modified parameter type scene
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_010, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_TRUE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: normal type out modified parameter type scene
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_011, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_TRUE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: normal type in and out modified parameter type scene
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_012, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_TRUE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: normal type retval modified parameter type scene
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_013, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_TRUE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: normal type combination modified parameter type scene
 */
HWTEST_F(TsCodeEmitterInterfaceTest, CheckInterfaceType_014, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    EXPECT_TRUE(tsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: EmitInterfaceImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceImports_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceImports_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceImports_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceImports_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceImports_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceImports_006, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceSelfDefinedTypeImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceSelfDefinedTypeImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceSelfDefinedTypeImports_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceSelfDefinedTypeImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceSelfDefinedTypeImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceSelfDefinedTypeImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceSelfDefinedTypeImports_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceSelfDefinedTypeImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceSelfDefinedTypeImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceSelfDefinedTypeImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceSelfDefinedTypeImports_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceSelfDefinedTypeImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceSelfDefinedTypeImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceSelfDefinedTypeImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceSelfDefinedTypeImports_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceSelfDefinedTypeImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceSelfDefinedTypeImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceSelfDefinedTypeImports is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceSelfDefinedTypeImports_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceSelfDefinedTypeImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceDefinition
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceDefinition is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceDefinition_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceDefinition(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceDefinition
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceDefinition is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceDefinition_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceDefinition(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceDefinition
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceDefinition is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceDefinition_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceDefinition(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceDefinition
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceDefinition is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceDefinition_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceDefinition(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceDefinition
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceDefinition is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceDefinition_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceDefinition(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethods
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethods is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethods_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceMethods(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethods
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethods is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethods_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceMethods(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethods
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethods is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethods_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceMethods(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethods
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethods is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethods_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceMethods(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethods
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethods is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethods_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceMethods(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethod
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethod is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethod_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[0];
    tsCodeGen_->EmitInterfaceMethod(metaMethod, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethod
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethod is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethod_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[0];
    tsCodeGen_->EmitInterfaceMethod(metaMethod, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethod
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethod is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethod_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[0];
    tsCodeGen_->EmitInterfaceMethod(metaMethod, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethod
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethod is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethod_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[0];
    tsCodeGen_->EmitInterfaceMethod(metaMethod, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethod
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethod is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethod_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[0];
    tsCodeGen_->EmitInterfaceMethod(metaMethod, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodParameter
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodParameter is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethodParameter_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[0];
    tsCodeGen_->EmitInterfaceMethodParameter(metaMethod->parameters_[0], stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodParameter
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodParameter is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethodParameter_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[0];
    tsCodeGen_->EmitInterfaceMethodParameter(metaMethod->parameters_[0], stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodParameter
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodParameter is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethodParameter_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[0];
    tsCodeGen_->EmitInterfaceMethodParameter(metaMethod->parameters_[0], stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodParameter
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodParameter is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethodParameter_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    MetaParameter mp = {0};
    mp.name_ = new (std::nothrow) char[] {"name"};
    tsCodeGen_->EmitInterfaceMethodParameter(&mp, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mp.name_ != nullptr) {
        delete[] mp.name_;
        mp.name_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodParameter
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodParameter is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethodParameter_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[0];
    tsCodeGen_->EmitInterfaceMethodParameter(metaMethod->parameters_[0], stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitMethodInParameter
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitMethodInParameter is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitMethodInParameter_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitMethodInParameter(stringBuilder, "name", "type", CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitMethodInParameter
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitMethodInParameter is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitMethodInParameter_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitMethodInParameter(stringBuilder, "name", "type", CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitMethodInParameter
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitMethodInParameter is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitMethodInParameter_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitMethodInParameter(stringBuilder, "name", "type", CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitMethodInParameter
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitMethodInParameter is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitMethodInParameter_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitMethodInParameter(stringBuilder, "name", "type", CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitMethodInParameter
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitMethodInParameter is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitMethodInParameter_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitMethodInParameter(stringBuilder, "name", "boolean", CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodExportCallback
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethodExportCallback_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    TsCodeEmitter::Method m;
    bool isLast = true;
    m.retParameter_.name_ = "name";
    m.retParameter_.type_ = "boolean";
    tsCodeGen_->EmitInterfaceMethodExportCallback(m, m.retParameter_, isLast);
    EXPECT_FALSE(m.exportFunction_.empty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodExportCallback
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethodExportCallback_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    TsCodeEmitter::Method m;
    bool isLast = true;
    m.retParameter_.name_ = "name";
    m.retParameter_.type_ = "boolean";
    tsCodeGen_->EmitInterfaceMethodExportCallback(m, m.retParameter_, isLast);
    EXPECT_FALSE(m.exportFunction_.empty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodExportCallback
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethodExportCallback_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    TsCodeEmitter::Method m;
    bool isLast = true;
    m.retParameter_.name_ = "name";
    m.retParameter_.type_ = "boolean";
    tsCodeGen_->EmitInterfaceMethodExportCallback(m, m.retParameter_, isLast);
    EXPECT_FALSE(m.exportFunction_.empty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodExportCallback
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethodExportCallback_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    TsCodeEmitter::Method m;
    bool isLast = true;
    m.retParameter_.name_ = "name";
    m.retParameter_.type_ = "boolean";
    tsCodeGen_->EmitInterfaceMethodExportCallback(m, m.retParameter_, isLast);
    EXPECT_FALSE(m.exportFunction_.empty());
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodExportCallback
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterfaceMethodExportCallback_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    TsCodeEmitter::Method m;
    bool isLast = true;
    m.retParameter_.name_ = "name";
    m.retParameter_.type_ = "boolean";
    tsCodeGen_->EmitInterfaceMethodExportCallback(m, m.retParameter_, isLast);
    EXPECT_FALSE(m.exportFunction_.empty());
}

/*
 * Feature: idl
 * Function: EmitReadArrayVariable
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitReadArrayVariable is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadArrayVariable_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Boolean, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Boolean;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadArrayVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadArrayVariable
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitReadArrayVariable is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadArrayVariable_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Char, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Char;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadArrayVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadArrayVariable
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitReadArrayVariable is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadArrayVariable_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Byte, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Byte;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadArrayVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadArrayVariable
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitReadArrayVariable is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadArrayVariable_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Short, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Short;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadArrayVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadArrayVariable
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitReadArrayVariable is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadArrayVariable_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Integer, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Integer;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadArrayVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadArrayVariable
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitReadArrayVariable is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadArrayVariable_006, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Long, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Long;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadArrayVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadArrayVariable
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitReadArrayVariable is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadArrayVariable_007, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Float, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Float;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadArrayVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadArrayVariable
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitReadArrayVariable is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadArrayVariable_008, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Double, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Double;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadArrayVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadArrayVariable
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitReadArrayVariable is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadArrayVariable_009, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::String, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::String;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadArrayVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadArrayVariable
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitReadArrayVariable is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadArrayVariable_010, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Sequenceable, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Sequenceable;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadArrayVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitInterface
 * SubFunction: NA
 * FunctionPoints: EmitInterface validates the interface name
 * EnvConditions: NA
 * CaseDescription: Content interface name not start with "I"
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitInterface_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(INTERFACE_SPECIAL_NAME_IDL_NAME.c_str(),
        INTERFACE_SPECIAL_NAME_IDL_CONTENT.c_str()),
        ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", INTERFACE_SPECIAL_NAME_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    tsCodeGen_->EmitInterface();

    String filePath = String::Format("%s/%s", "./", "special_name_test.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    void* fileData = malloc(header.size_);
    file.ReadData(fileData, header.size_);
    std::string data((char*)fileData);

    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("Copyright"), std::string::npos);
    EXPECT_NE(data.find("export default interface SpecialNameTest"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("export type voidParameterTypeShortCallback"), std::string::npos);
    free(fileData);
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Boolean, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Char, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Byte, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Short, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Integer, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_006, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Long, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_007, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Float, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_008, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Double, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_009, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::String, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_010, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Sequenceable, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_011, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Interface, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_012, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::List, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_013, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Map, 0, 1, new (std::nothrow) int[2] {0}};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::String;
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[1]]->kind_ = TypeKind::String;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete[] mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitWriteVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitWriteVariable_014, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Array, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Boolean;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitWriteVariable("parcelName", "name", &mt, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Boolean, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Char, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Byte, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Short, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Integer, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_006, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Long, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_007, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Float, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_008, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Double, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_009, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::String, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_010, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Sequenceable, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_011, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Interface, 0, 1, new (std::nothrow) int(0)};
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_012, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Map, 0, 1, new (std::nothrow) int[2] {0}};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::String;
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[1]]->kind_ = TypeKind::String;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete[] mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_013, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Array, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Boolean;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, ATTR_MASK + ATTR_OUT, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}

/*
 * Feature: idl
 * Function: EmitReadVariable
 * SubFunction: NA
 * FunctionPoints:  For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceMethodExportCallback is not empty.
 */
HWTEST_F(TsCodeEmitterInterfaceTest, EmitReadVariable_014, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    MetaType mt = {TypeKind::Array, 0, 1, new (std::nothrow) int(0)};
    tsCodeGen_->metaComponent_->types_[mt.nestedTypeIndexes_[0]]->kind_ = TypeKind::Boolean;
    StringBuilder stringBuilder;
    tsCodeGen_->EmitReadVariable("parcelName", "name", &mt, 0, stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    if (mt.nestedTypeIndexes_ != nullptr) {
        delete mt.nestedTypeIndexes_;
        mt.nestedTypeIndexes_ = nullptr;
    }
}
}  // namespace UnitTest
}  // namespace Idl
}  // namespace OHOS
