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
class TsCodeEmitterStubTest : public testing::Test, public IdlCommon {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void TsCodeEmitterStubTest::SetUpTestCase()
{}

void TsCodeEmitterStubTest::TearDownTestCase()
{}

void TsCodeEmitterStubTest::SetUp()
{}

void TsCodeEmitterStubTest::TearDown()
{}


/*
 * Feature: idl
 * Function: EmitInterfaceStub
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStub validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStub_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStub();

    String filePath = String::Format("%s/%s", "./", "idl_test_stub.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    void* fileData = malloc(header.size_);
    file.ReadData(fileData, header.size_);
    std::string data((char*)fileData);

    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("Copyright"), std::string::npos);
    EXPECT_NE(data.find("import IIdlTest"), std::string::npos);
    EXPECT_NE(data.find("extends rpc.RemoteObject implements IIdlTest"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeVoid"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeByte"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInt"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeLong"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeFloat"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeBooleanArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceable"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceableArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInterface"), std::string::npos);
    EXPECT_NE(data.find("voidMap"), std::string::npos);
    free(fileData);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStub
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStub validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStub_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStub();

    String filePath = String::Format("%s/%s", "./", "idl_test_stub.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    void* fileData = malloc(header.size_);
    file.ReadData(fileData, header.size_);
    std::string data((char*)fileData);

    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("Copyright"), std::string::npos);
    EXPECT_NE(data.find("import IIdlTest"), std::string::npos);
    EXPECT_NE(data.find("extends rpc.RemoteObject implements IIdlTest"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeVoid"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeByte"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInt"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeLong"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeFloat"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeBooleanArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceable"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceableArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInterface"), std::string::npos);
    EXPECT_NE(data.find("voidMap"), std::string::npos);
    free(fileData);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStub
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStub validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStub_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStub();

    String filePath = String::Format("%s/%s", "./", "idl_test_stub.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    void* fileData = malloc(header.size_);
    file.ReadData(fileData, header.size_);
    std::string data((char*)fileData);

    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("Copyright"), std::string::npos);
    EXPECT_NE(data.find("import IIdlTest"), std::string::npos);
    EXPECT_NE(data.find("extends rpc.RemoteObject implements IIdlTest"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeVoid"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeByte"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInt"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeLong"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeFloat"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeBooleanArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceable"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceableArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInterface"), std::string::npos);
    EXPECT_NE(data.find("voidMap"), std::string::npos);
    free(fileData);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStub
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStub validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStub_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStub();

    String filePath = String::Format("%s/%s", "./", "idl_test_stub.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    void* fileData = malloc(header.size_);
    file.ReadData(fileData, header.size_);
    std::string data((char*)fileData);

    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("Copyright"), std::string::npos);
    EXPECT_NE(data.find("import IIdlTest"), std::string::npos);
    EXPECT_NE(data.find("extends rpc.RemoteObject implements IIdlTest"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeVoid"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeByte"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInt"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeLong"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeFloat"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeBooleanArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceable"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceableArray"), std::string::npos);
    EXPECT_NE(data.find("voidMap"), std::string::npos);
    free(fileData);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStub
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStub validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStub_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStub();

    String filePath = String::Format("%s/%s", "./", "idl_test_stub.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    void* fileData = malloc(header.size_);
    file.ReadData(fileData, header.size_);
    std::string data((char*)fileData);

    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("Copyright"), std::string::npos);
    EXPECT_NE(data.find("import IIdlTest"), std::string::npos);
    EXPECT_NE(data.find("extends rpc.RemoteObject implements IIdlTest"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeByte"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInt"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeLong"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeFloat"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeBooleanArray"), std::string::npos);
    EXPECT_NE(data.find("testIntTransaction"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceableArray"), std::string::npos);
    EXPECT_NE(data.find("inOutMapArray"), std::string::npos);
    free(fileData);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStub
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStub validates the interface name
 * EnvConditions: NA
 * CaseDescription: Content interface name not start with "I"
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStub_006, TestSize.Level1)
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
    tsCodeGen_->EmitInterfaceStub();

    String filePath = String::Format("%s/%s", "./", "special_name_test_stub.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    void* fileData = malloc(header.size_);
    file.ReadData(fileData, header.size_);
    std::string data((char*)fileData);

    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("Copyright"), std::string::npos);
    EXPECT_NE(data.find("import SpecialNameTest"), std::string::npos);
    EXPECT_NE(data.find("extends rpc.RemoteObject implements SpecialNameTest"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("onRemoteRequest"), std::string::npos);
    free(fileData);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubImpl
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubImpl validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubImpl_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubImpl(stringBuilder);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("export default class IdlTestStub"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubImpl
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubImpl validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubImpl_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubImpl(stringBuilder);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("export default class IdlTestStub"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubImpl
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubImpl validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubImpl_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubImpl(stringBuilder);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("export default class IdlTestStub"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubImpl
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubImpl validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubImpl_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubImpl(stringBuilder);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("export default class IdlTestStub"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubImpl
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubImpl validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubImpl_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubImpl(stringBuilder);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("export default class IdlTestStub"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubConstructor
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubConstructor validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubConstructor_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubConstructor(stringBuilder, CodeEmitter::TAB);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("constructor"), std::string::npos);
    EXPECT_NE(data.find("super(des)"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubConstructor
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubConstructor validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubConstructor_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubConstructor(stringBuilder, CodeEmitter::TAB);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("constructor"), std::string::npos);
    EXPECT_NE(data.find("super(des)"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubConstructor
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubConstructor validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubConstructor_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubConstructor(stringBuilder, CodeEmitter::TAB);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("constructor"), std::string::npos);
    EXPECT_NE(data.find("super(des)"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubConstructor
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubConstructor validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubConstructor_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubConstructor(stringBuilder, CodeEmitter::TAB);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("constructor"), std::string::npos);
    EXPECT_NE(data.find("super(des)"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubConstructor
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubConstructor validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubConstructor_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubConstructor(stringBuilder, CodeEmitter::TAB);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("constructor"), std::string::npos);
    EXPECT_NE(data.find("super(des)"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubMethodImpls
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubMethodImpls validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubMethodImpls_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubMethodImpls(stringBuilder, CodeEmitter::TAB);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("onRemoteRequest(code: number, data, reply, option)"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeVoid"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeByte"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInt"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeLong"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeFloat"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeString"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeBoolean"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShortArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceable"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInterface"), std::string::npos);
    EXPECT_NE(data.find("voidMap"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubMethodImpls
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubMethodImpls validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubMethodImpls_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubMethodImpls(stringBuilder, CodeEmitter::TAB);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("onRemoteRequest(code: number, data, reply, option)"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeVoid"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeByte"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInt"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeLong"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeFloat"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeString"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeBoolean"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShortArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceable"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInterface"), std::string::npos);
    EXPECT_NE(data.find("voidMap"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubMethodImpls
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubMethodImpls validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubMethodImpls_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubMethodImpls(stringBuilder, CodeEmitter::TAB);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("onRemoteRequest(code: number, data, reply, option)"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeVoid"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeByte"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInt"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeLong"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeFloat"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeString"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeBoolean"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShortArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceable"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInterface"), std::string::npos);
    EXPECT_NE(data.find("voidMap"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubMethodImpls
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubMethodImpls validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubMethodImpls_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubMethodImpls(stringBuilder, CodeEmitter::TAB);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("onRemoteRequest(code: number, data, reply, option)"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeVoid"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeByte"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInt"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeLong"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeFloat"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeString"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeBoolean"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShortArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceable"), std::string::npos);
    EXPECT_NE(data.find("voidMap"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubMethodImpls
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubMethodImpls validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubMethodImpls_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceStubMethodImpls(stringBuilder, CodeEmitter::TAB);
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("onRemoteRequest(code: number, data, reply, option)"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeByte"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeInt"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeLong"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeFloat"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeString"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeBoolean"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShortArray"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeMySequenceable"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubMethodImpl
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubMethodImpl validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubMethodImpl_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceStubMethodImpl(metaMethod, index, stringBuilder, CodeEmitter::TAB);
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_VOID"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_SHORT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_BYTE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_INT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_LONG"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_FLOAT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_STRING"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_DOUBLE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_BOOLEAN"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_SHORT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_INTERFACE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_MAP"), std::string::npos);
    EXPECT_NE(data.find("reply.writeInt"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubMethodImpl
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubMethodImpl validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubMethodImpl_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceStubMethodImpl(metaMethod, index, stringBuilder, CodeEmitter::TAB);
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_VOID"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_SHORT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_BYTE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_INT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_LONG"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_FLOAT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_STRING"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_DOUBLE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_BOOLEAN"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_SHORT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_INTERFACE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_MAP"), std::string::npos);
    EXPECT_NE(data.find("reply.writeInt"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubMethodImpl
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubMethodImpl validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubMethodImpl_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceStubMethodImpl(metaMethod, index, stringBuilder, CodeEmitter::TAB);
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_VOID"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_SHORT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_BYTE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_INT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_LONG"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_FLOAT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_STRING"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_DOUBLE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_BOOLEAN"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_SHORT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_INTERFACE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_MAP"), std::string::npos);
    EXPECT_NE(data.find("reply.writeInt"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubMethodImpl
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubMethodImpl validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubMethodImpl_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceStubMethodImpl(metaMethod, index, stringBuilder, CodeEmitter::TAB);
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_VOID"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_SHORT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_BYTE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_INT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_LONG"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_FLOAT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_STRING"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_DOUBLE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_BOOLEAN"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_SHORT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_MAP"), std::string::npos);
    EXPECT_NE(data.find("reply.writeInt"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceStubMethodImpl
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceStubMethodImpl validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceStubMethodImpl_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceStubMethodImpl(metaMethod, index, stringBuilder, CodeEmitter::TAB);
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_SHORT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_BYTE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_INT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_LONG"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_FLOAT"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_STRING"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_DOUBLE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_BOOLEAN"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_SHORT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE"), std::string::npos);
    EXPECT_NE(data.find("IdlTestStub.COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("reply.writeInt"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodCommands
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceMethodCommands validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceMethodCommands_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceMethodCommands(stringBuilder);

    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_VOID"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_SHORT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BYTE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_LONG"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_FLOAT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_STRING"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_DOUBLE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BOOLEAN"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_SHORT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BYTE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_LONG_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_FLOAT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_STRING_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_DOUBLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BOOLEAN_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INTERFACE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_MAP"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodCommands
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceMethodCommands validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceMethodCommands_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceMethodCommands(stringBuilder);

    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_VOID"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_SHORT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BYTE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_LONG"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_FLOAT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_STRING"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_DOUBLE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BOOLEAN"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_SHORT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BYTE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_LONG_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_FLOAT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_STRING_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_DOUBLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BOOLEAN_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INTERFACE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_MAP"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodCommands
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceMethodCommands validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceMethodCommands_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceMethodCommands(stringBuilder);

    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_VOID"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_SHORT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BYTE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_LONG"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_FLOAT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_STRING"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_DOUBLE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BOOLEAN"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_SHORT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BYTE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_LONG_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_FLOAT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_STRING_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_DOUBLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BOOLEAN_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INTERFACE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_MAP"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodCommands
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceMethodCommands validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceMethodCommands_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceMethodCommands(stringBuilder);

    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_VOID"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_SHORT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BYTE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_LONG"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_FLOAT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_STRING"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_DOUBLE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BOOLEAN"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_SHORT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BYTE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_LONG_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_FLOAT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_STRING_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_DOUBLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BOOLEAN_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_MAP"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitInterfaceMethodCommands
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceMethodCommands validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterStubTest, EmitInterfaceMethodCommands_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceMethodCommands(stringBuilder);

    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_SHORT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BYTE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_LONG"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_FLOAT"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_STRING"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_DOUBLE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BOOLEAN"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_SHORT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BYTE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_INT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_LONG_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_FLOAT_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_STRING_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_DOUBLE_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_BOOLEAN_ARRAY"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE"), std::string::npos);
    EXPECT_NE(data.find("static readonly COMMAND_VOID_PARAMETER_TYPE_MY_SEQUENCEABLE_ARRAY"), std::string::npos);
}
}  // namespace UnitTest
}  // namespace Idl
}  // namespace OHOS
