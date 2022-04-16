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

#define private public
#define protected public
#include "idl_common.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Idl::TestCommon;

namespace OHOS {
namespace Idl {
namespace UnitTest {
class JsCodeEmitterStubTest : public testing::Test, public IdlCommon {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void JsCodeEmitterStubTest::SetUpTestCase()
{}

void JsCodeEmitterStubTest::TearDownTestCase()
{}

void JsCodeEmitterStubTest::SetUp()
{}

void JsCodeEmitterStubTest::TearDown()
{}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(value type).
 */
HWTEST_F(JsCodeEmitterStubTest, CheckInterfaceType_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE1_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

#if 0 // baof 没有编译

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: inout type exists in .idl(value type).
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceStubMethodImpls_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    StringBuilder sb;
    std::string str("onRemoteRequest(code: number, data, reply, option): boolean");

    //const char* CodeEmitter::TAB = "    ";
    jsCodeGen_->EmitInterfaceStubMethodImpls(sb, TAB);
    EXPECT_FALSE(sb.ToString().find(str) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: inout type exists in .idl(value type).
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceStubMethodImpls_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    StringBuilder sb;
    std::string str("onRemoteRequest(code: number, data, reply, option): boolean");

    //const char* CodeEmitter::TAB = "    ";
    jsCodeGen_->EmitInterfaceStubMethodImpls(sb, TAB);
    EXPECT_FALSE(sb.ToString().find(str) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: inout type exists in .idl(value type).
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceStubMethodImpls_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    StringBuilder sb;
    std::string str("onRemoteRequest(code: number, data, reply, option): boolean");

    //const char* CodeEmitter::TAB = "    ";
    jsCodeGen_->EmitInterfaceStubMethodImpls(sb, TAB);
    EXPECT_FALSE(sb.ToString().find(str) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: inout type exists in .idl(value type).
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceStubMethodImpls_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    StringBuilder sb;
    std::string str("onRemoteRequest(code: number, data, reply, option): boolean");

    //const char* CodeEmitter::TAB = "    ";
    jsCodeGen_->EmitInterfaceStubMethodImpls(sb, TAB);
    EXPECT_FALSE(sb.ToString().find(str) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: inout type exists in .idl(value type).
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceStubMethodImpls_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    StringBuilder sb;
    std::string str("onRemoteRequest(code: number, data, reply, option): boolean");

    //const char* CodeEmitter::TAB = "    ";
    jsCodeGen_->EmitInterfaceStubMethodImpls(sb, TAB);
    EXPECT_FALSE(sb.ToString().find(str) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: Interface stub method implementation
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceStubMethodImpl_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    jsCodeGen_->EmitInterfaceStubMethodImpl();
    EXPECT_FALSE(sb.ToString().find(str) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: Interface stub method implementation
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceStubMethodImpl_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    jsCodeGen_->EmitInterfaceStubMethodImpl();
    EXPECT_FALSE(sb.ToString().find(str) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: Interface stub method implementation
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceStubMethodImpl_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    jsCodeGen_->EmitInterfaceStubMethodImpl();
    EXPECT_FALSE(sb.ToString().find(str) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: Interface stub method implementation
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceStubMethodImpl_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    jsCodeGen_->EmitInterfaceStubMethodImpl();
    EXPECT_FALSE(sb.ToString().find(str) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: Interface stub method implementation
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceStubMethodImpl_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    jsCodeGen_->EmitInterfaceStubMethodImpl();
    EXPECT_FALSE(sb.ToString().find(str) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: Issue interface method commands
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceMethodCommands_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    std::string keywords("readonly");
    StringBuilder sb;
    jsCodeGen_->EmitInterfaceMethodCommands(sb);
    EXPECT_FALSE(sb.ToString().find(keywords) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: Issue interface method commands
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceMethodCommands_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    std::string keywords("readonly");
    StringBuilder sb;
    jsCodeGen_->EmitInterfaceMethodCommands(sb);
    EXPECT_FALSE(sb.ToString().find(keywords) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: Issue interface method commands
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceMethodCommands_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    std::string keywords("readonly");
    StringBuilder sb;
    jsCodeGen_->EmitInterfaceMethodCommands(sb);
    EXPECT_FALSE(sb.ToString().find(keywords) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: Issue interface method commands
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceMethodCommands_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    std::string keywords("readonly");
    StringBuilder sb;
    jsCodeGen_->EmitInterfaceMethodCommands(sb);
    EXPECT_FALSE(sb.ToString().find(keywords) == std::string::npos);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: Issue interface method commands
 */
HWTEST_F(JsCodeEmitterStubTest, EmitInterfaceMethodCommands_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    auto num = jsCodeGen_->metaInterface_->methodNumber_;
    std::string keywords("readonly");
    StringBuilder sb;
    jsCodeGen_->EmitInterfaceMethodCommands(sb);
    EXPECT_FALSE(std::count(sb.ToString().begin(), sb.ToString().end(), keywords) == num);
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: emit stub write method parameter
 */
HWTEST_F(JsCodeEmitterStubTest, EmitStubWriteMethodParameter_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);

    std::string keywords("readonly");
    int num = jsCodeGen_->
    StringBuilder sb;
    jsCodeGen_->EmitStubWriteMethodParameter(sb);
    EXPECT_FALSE(sb.ToString().find(keywords) == std::string::npos);
}
#endif
}  // namespace UnitTest
}  // namespace Idl
}  // namespace OHOS
