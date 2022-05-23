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

#include <cstring>
#include <gtest/gtest.h>
#include "idl_common.h"

using namespace testing::ext;
using namespace OHOS::Idl::TestCommon;
namespace OHOS {
namespace Idl {
namespace UnitTest {
class TsCodeEmitterProxyTest : public testing::Test, public IdlCommon {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TsCodeEmitterProxyTest::SetUpTestCase(void)
{}
void TsCodeEmitterProxyTest::TearDownTestCase(void)
{}
void TsCodeEmitterProxyTest::SetUp()
{}
void TsCodeEmitterProxyTest::TearDown()
{}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitWriteArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitWriteArrayVariable_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_IN) != 0) {
                MetaType* innerType = tsCodeGen_->metaComponent_->types_[mt->nestedTypeIndexes_[0]];
                tsCodeGen_->EmitWriteArrayVariable("data", mp->name_, innerType,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    EXPECT_TRUE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitWriteArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitWriteArrayVariable_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_IN) != 0) {
                MetaType* innerType = tsCodeGen_->metaComponent_->types_[mt->nestedTypeIndexes_[0]];
                tsCodeGen_->EmitWriteArrayVariable("data", mp->name_, innerType,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitWriteArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitWriteArrayVariable_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_IN) != 0) {
                MetaType* innerType = tsCodeGen_->metaComponent_->types_[mt->nestedTypeIndexes_[0]];
                tsCodeGen_->EmitWriteArrayVariable("data", mp->name_, innerType,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitWriteArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitWriteArrayVariable_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_IN) != 0) {
                MetaType* innerType = tsCodeGen_->metaComponent_->types_[mt->nestedTypeIndexes_[0]];
                tsCodeGen_->EmitWriteArrayVariable("data", mp->name_, innerType,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    EXPECT_TRUE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitWriteArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitWriteArrayVariable_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    int count = 0;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_IN) != 0) {
                count++;
                MetaType* innerType = tsCodeGen_->metaComponent_->types_[mt->nestedTypeIndexes_[0]];
                tsCodeGen_->EmitWriteArrayVariable("data", mp->name_, innerType,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    const int size = 18;
    EXPECT_TRUE(count == size);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadOutArrayVariable_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_MASK) == ATTR_OUT) {
                tsCodeGen_->EmitReadOutArrayVariable("data", mp->name_, mt,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadOutArrayVariable_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_MASK) == ATTR_OUT) {
                tsCodeGen_->EmitReadOutArrayVariable("data", mp->name_, mt,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_TRUE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitReadOutArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadOutArrayVariable_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array) {
                tsCodeGen_->EmitReadOutArrayVariable("data", mp->name_, mt,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadOutArrayVariable_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array) {
                tsCodeGen_->EmitReadOutArrayVariable("data", mp->name_, mt,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_TRUE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitReadOutArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadOutArrayVariable_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array) {
                tsCodeGen_->EmitReadOutArrayVariable("data", mp->name_, mt,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteMethodParameter
 * SubFunction: NA
 * FunctionPoints: EmitWriteMethodParameter validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitWriteMethodParameter_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_IN) != 0) {
                tsCodeGen_->EmitWriteMethodParameter(mp, "reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("writeInt"), std::string::npos);
    EXPECT_NE(data.find("writeLong"), std::string::npos);
    EXPECT_NE(data.find("writeFloat"), std::string::npos);
    EXPECT_NE(data.find("writeDouble"), std::string::npos);
    EXPECT_NE(data.find("writeString"), std::string::npos);
    EXPECT_NE(data.find("writeSequenceable"), std::string::npos);
    EXPECT_NE(data.find("writeRemoteObject"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitWriteMethodParameter
 * SubFunction: NA
 * FunctionPoints: EmitWriteMethodParameter validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitWriteMethodParameter_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitWriteMethodParameter(mp, "reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("writeInt"), std::string::npos);
    EXPECT_NE(data.find("writeLong"), std::string::npos);
    EXPECT_NE(data.find("writeFloat"), std::string::npos);
    EXPECT_NE(data.find("writeDouble"), std::string::npos);
    EXPECT_NE(data.find("writeString"), std::string::npos);
    EXPECT_NE(data.find("writeSequenceable"), std::string::npos);
    EXPECT_NE(data.find("writeRemoteObject"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitWriteMethodParameter
 * SubFunction: NA
 * FunctionPoints: EmitWriteMethodParameter validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitWriteMethodParameter_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitWriteMethodParameter(mp, "reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("writeInt"), std::string::npos);
    EXPECT_NE(data.find("writeLong"), std::string::npos);
    EXPECT_NE(data.find("writeFloat"), std::string::npos);
    EXPECT_NE(data.find("writeDouble"), std::string::npos);
    EXPECT_NE(data.find("writeString"), std::string::npos);
    EXPECT_NE(data.find("writeSequenceable"), std::string::npos);
    EXPECT_NE(data.find("writeRemoteObject"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitWriteMethodParameter
 * SubFunction: NA
 * FunctionPoints: EmitWriteMethodParameter validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitWriteMethodParameter_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitWriteMethodParameter(mp, "reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_TRUE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitWriteMethodParameter
 * SubFunction: NA
 * FunctionPoints: EmitWriteMethodParameter validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitWriteMethodParameter_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitWriteMethodParameter(mp, "reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("writeInt"), std::string::npos);
    EXPECT_NE(data.find("writeLong"), std::string::npos);
    EXPECT_NE(data.find("writeFloat"), std::string::npos);
    EXPECT_NE(data.find("writeDouble"), std::string::npos);
    EXPECT_NE(data.find("writeString"), std::string::npos);
    EXPECT_NE(data.find("writeSequenceable"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitReadMethodParameter
 * SubFunction: NA
 * FunctionPoints: EmitReadMethodParameter validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadMethodParameter_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_IN) != 0) {
                tsCodeGen_->EmitReadMethodParameter(mp, "result.reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("readInt"), std::string::npos);
    EXPECT_NE(data.find("readLong"), std::string::npos);
    EXPECT_NE(data.find("readFloat"), std::string::npos);
    EXPECT_NE(data.find("readDouble"), std::string::npos);
    EXPECT_NE(data.find("readString"), std::string::npos);
    EXPECT_NE(data.find("readSequenceable"), std::string::npos);
    EXPECT_NE(data.find("readRemoteObject"), std::string::npos);
    EXPECT_NE(data.find("new Map()"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitReadMethodParameter
 * SubFunction: NA
 * FunctionPoints: EmitReadMethodParameter validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadMethodParameter_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitReadMethodParameter(mp, "result.reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("readInt"), std::string::npos);
    EXPECT_NE(data.find("readLong"), std::string::npos);
    EXPECT_NE(data.find("readFloat"), std::string::npos);
    EXPECT_NE(data.find("readDouble"), std::string::npos);
    EXPECT_NE(data.find("readString"), std::string::npos);
    EXPECT_NE(data.find("readSequenceable"), std::string::npos);
    EXPECT_NE(data.find("readRemoteObject"), std::string::npos);
    EXPECT_NE(data.find("new Map()"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitReadMethodParameter
 * SubFunction: NA
 * FunctionPoints: EmitReadMethodParameter validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadMethodParameter_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitReadMethodParameter(mp, "result.reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("readInt"), std::string::npos);
    EXPECT_NE(data.find("readLong"), std::string::npos);
    EXPECT_NE(data.find("readFloat"), std::string::npos);
    EXPECT_NE(data.find("readDouble"), std::string::npos);
    EXPECT_NE(data.find("readString"), std::string::npos);
    EXPECT_NE(data.find("readSequenceable"), std::string::npos);
    EXPECT_NE(data.find("readRemoteObject"), std::string::npos);
    EXPECT_NE(data.find("new Map()"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitReadMethodParameter
 * SubFunction: NA
 * FunctionPoints: EmitReadMethodParameter validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadMethodParameter_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitReadMethodParameter(mp, "result.reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_TRUE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitReadMethodParameter
 * SubFunction: NA
 * FunctionPoints: EmitReadMethodParameter validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadMethodParameter_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT2.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitReadMethodParameter(mp, "result.reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("readIntArray"), std::string::npos);
    EXPECT_NE(data.find("readLongArray"), std::string::npos);
    EXPECT_NE(data.find("readFloatArray"), std::string::npos);
    EXPECT_NE(data.find("readStringArray"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitReadOutVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadOutVariable_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if ((mp->attributes_ & ATTR_IN) != 0) {
                tsCodeGen_->EmitReadOutVariable("result.reply", mp->name_, mt, stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("readInt"), std::string::npos);
    EXPECT_NE(data.find("readLong"), std::string::npos);
    EXPECT_NE(data.find("readFloat"), std::string::npos);
    EXPECT_NE(data.find("readDouble"), std::string::npos);
    EXPECT_NE(data.find("readString"), std::string::npos);
    EXPECT_NE(data.find("readSequenceable"), std::string::npos);
    EXPECT_NE(data.find("readRemoteObject"), std::string::npos);
    EXPECT_NE(data.find("new Map()"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitReadOutVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadOutVariable_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitReadOutVariable("result.reply", mp->name_, mt, stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("readInt"), std::string::npos);
    EXPECT_NE(data.find("readLong"), std::string::npos);
    EXPECT_NE(data.find("readFloat"), std::string::npos);
    EXPECT_NE(data.find("readDouble"), std::string::npos);
    EXPECT_NE(data.find("readString"), std::string::npos);
    EXPECT_NE(data.find("readSequenceable"), std::string::npos);
    EXPECT_NE(data.find("readRemoteObject"), std::string::npos);
    EXPECT_NE(data.find("new Map()"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitReadOutVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadOutVariable_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitReadOutVariable("result.reply", mp->name_, mt, stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("readInt"), std::string::npos);
    EXPECT_NE(data.find("readLong"), std::string::npos);
    EXPECT_NE(data.find("readFloat"), std::string::npos);
    EXPECT_NE(data.find("readDouble"), std::string::npos);
    EXPECT_NE(data.find("readString"), std::string::npos);
    EXPECT_NE(data.find("readSequenceable"), std::string::npos);
    EXPECT_NE(data.find("readRemoteObject"), std::string::npos);
    EXPECT_NE(data.find("new Map()"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitReadOutVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadOutVariable_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitReadOutVariable("result.reply", mp->name_, mt, stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_TRUE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitReadOutVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitReadOutVariable_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT2.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                tsCodeGen_->EmitReadOutVariable("result.reply", mp->name_, mt, stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("readIntArray"), std::string::npos);
    EXPECT_NE(data.find("readLongArray"), std::string::npos);
    EXPECT_NE(data.find("readFloatArray"), std::string::npos);
    EXPECT_NE(data.find("readStringArray"), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitLicense
 * SubFunction: NA
 * FunctionPoints: TsCodeEmitter EmitLicense
 * EnvConditions: NA
 * CaseDescription: Incoming normal license, stringBuilder appended the license.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitLicense_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitLicense(stringBuilder);

    std::string data(stringBuilder.ToString().string());
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("Ltd."), std::string::npos);
}

/*
 * Feature: idl
 * Function: EmitLicense
 * SubFunction: NA
 * FunctionPoints: TsCodeEmitter EmitLicense
 * EnvConditions: NA
 * CaseDescription: Incoming null license, stringBuilder not have content.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitLicense_002, TestSize.Level1)
{
    std::string fileLicense;
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_IDL_CONTENT.c_str(), fileLicense), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitLicense(stringBuilder);

    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    std::string data(stringBuilder.ToString().string());
    std::string value = "\n";
    EXPECT_EQ(data, value);
}

/*
 * Feature: idl
 * Function: EmitType
 * SubFunction: NA
 * FunctionPoints: TsCodeEmitter EmitType
 * EnvConditions: NA
 * CaseDescription: Incoming differernt metaType, return expected data type.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitType_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; parameterIndex < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = tsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            String result = tsCodeGen_->EmitType(mt);
            stringBuilder.Append(result.string());
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    std::string data = stringBuilder.ToString().string();
    EXPECT_TRUE(data.length());
    EXPECT_NE(data.find("void"), std::string::npos);
    EXPECT_NE(data.find("number"), std::string::npos);
    EXPECT_NE(data.find("string"), std::string::npos);
    EXPECT_NE(data.find("MySequenceable"), std::string::npos);
    EXPECT_NE(data.find("IInterfaceDeclare"), std::string::npos);
    EXPECT_NE(data.find("boolean[]"), std::string::npos);
    EXPECT_NE(data.find("string[]"), std::string::npos);
    EXPECT_NE(data.find("number[]"), std::string::npos);
    EXPECT_NE(data.find("MySequenceable[]"), std::string::npos);
}

/*
 * Feature: idl
 * Function: FileName
 * SubFunction: NA
 * FunctionPoints: TsCodeEmitter EmitType
 * EnvConditions: NA
 * CaseDescription: Incoming differernt filename, return expected filename.
 */
HWTEST_F(TsCodeEmitterProxyTest, FileName_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    String param1;
    EXPECT_TRUE(tsCodeGen_->FileName(param1).Equals(param1));
    String param2("testFileName");
    EXPECT_TRUE(tsCodeGen_->FileName(param2).Equals("test_file_name"));
    String param3("app.data.testFileName");
    EXPECT_TRUE(tsCodeGen_->FileName(param3).Equals("app/data/test_file_name"));
    String param4("testfile");
    EXPECT_TRUE(tsCodeGen_->FileName(param4).Equals("testfile"));
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: Generate proxy code.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyImpl_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxyImpl(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("export default class"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("static readonly"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("private proxy"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: Generate proxy code.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyImpl_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxyImpl(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("export default class"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("static readonly"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("private proxy"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: Generate proxy code.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyImpl_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxyImpl(stringBuilder);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("export default class"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("static readonly"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("private proxy"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: Generate proxy code.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyImpl_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxyImpl(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("export default class"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("static readonly"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("private proxy"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: Generate proxy code.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyImpl_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxyImpl(stringBuilder);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("export default class"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("static readonly"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("private proxy"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyConstructor
 * SubFunction: NA
 * FunctionPoints: Proxy Constructor.
 * EnvConditions: NA
 * CaseDescription: Verify Proxy Constructor.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyConstructor_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceProxyConstructor(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("constructor("), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyConstructor
 * SubFunction: NA
 * FunctionPoints: Proxy Constructor.
 * EnvConditions: NA
 * CaseDescription: Verify Proxy Constructor.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyConstructor_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceProxyConstructor(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("constructor("), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyConstructor
 * SubFunction: NA
 * FunctionPoints: Proxy Constructor.
 * EnvConditions: NA
 * CaseDescription: Verify Proxy Constructor.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyConstructor_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceProxyConstructor(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("constructor("), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyConstructor
 * SubFunction: NA
 * FunctionPoints: Proxy Constructor.
 * EnvConditions: NA
 * CaseDescription: Verify Proxy Constructor.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyConstructor_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceProxyConstructor(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("constructor("), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyConstructor
 * SubFunction: NA
 * FunctionPoints: Proxy Constructor.
 * EnvConditions: NA
 * CaseDescription: Verify Proxy Constructor.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyConstructor_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterfaceProxyConstructor(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("constructor("), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodImpls
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Impls.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodImpls_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxyMethodImpls(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodImpls
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Impls.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodImpls_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxyMethodImpls(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodImpls
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Impls.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodImpls_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxyMethodImpls(stringBuilder, CodeEmitter::TAB);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodImpls
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Impls.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodImpls_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxyMethodImpls(stringBuilder, CodeEmitter::TAB);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodImpls
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Impls.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodImpls_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxyMethodImpls(stringBuilder, CodeEmitter::TAB);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodImpl
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Impl.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodImpl_001, TestSize.Level1)
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
        tsCodeGen_->EmitInterfaceProxyMethodImpl(metaMethod, index, stringBuilder,  CodeEmitter::TAB);
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodImpl
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Impl.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodImpl_002, TestSize.Level1)
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
        tsCodeGen_->EmitInterfaceProxyMethodImpl(metaMethod, index, stringBuilder,  CodeEmitter::TAB);
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodImpl
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Impl.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodImpl_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceProxyMethodImpl(metaMethod, index, stringBuilder,  CodeEmitter::TAB);
    }
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodImpl
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Impl.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodImpl_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceProxyMethodImpl(metaMethod, index, stringBuilder,  CodeEmitter::TAB);
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodImpl
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Impl.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodImpl_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceProxyMethodImpl(metaMethod, index, stringBuilder,  CodeEmitter::TAB);
    }
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodBody
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Body.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodBody_001, TestSize.Level1)
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
        tsCodeGen_->EmitInterfaceProxyMethodBody(metaMethod, index, stringBuilder,  CodeEmitter::TAB);
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodBody
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Body.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodBody_002, TestSize.Level1)
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
        tsCodeGen_->EmitInterfaceProxyMethodBody(metaMethod, index, stringBuilder,  CodeEmitter::TAB);
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodBody
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Body.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodBody_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceProxyMethodBody(metaMethod, index, stringBuilder,  CodeEmitter::TAB);
    }
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodBody
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Body.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodBody_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceProxyMethodBody(metaMethod, index, stringBuilder,  CodeEmitter::TAB);
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxyMethodBody
 * SubFunction: NA
 * FunctionPoints: Generate proxy Method Body.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxyMethodBody_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    for (int index = 0; index < tsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = tsCodeGen_->metaInterface_->methods_[index];
        tsCodeGen_->EmitInterfaceProxyMethodBody(metaMethod, index, stringBuilder,  CodeEmitter::TAB);
    }
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
}


/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: Generate proxy code.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxy_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxy();
    String filePath = String::Format("%s/%s", "./", "idl_test_proxy.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    char* fileData = new char[header.size_];
    file.ReadData(static_cast<void*>(fileData), header.size_);
    stringBuilder.Append(fileData);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("export default class"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("static readonly"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("private proxy"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    delete[] fileData;
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: Generate proxy code.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxy_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxy();
    String filePath = String::Format("%s/%s", "./", "idl_test_proxy.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    char* fileData = new char[header.size_];
    file.ReadData(static_cast<void*>(fileData), header.size_);
    stringBuilder.Append(fileData);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("export default class"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("static readonly"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("private proxy"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    delete[] fileData;
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: Generate proxy code.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxy_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxy();
    String filePath = String::Format("%s/%s", "./", "idl_test_proxy.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    char* fileData = new char[header.size_];
    file.ReadData(static_cast<void*>(fileData), header.size_);
    stringBuilder.Append(fileData);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("export default class"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("static readonly"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("private proxy"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    delete[] fileData;
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: Generate proxy code.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxy_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxy();
    String filePath = String::Format("%s/%s", "./", "idl_test_proxy.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    char* fileData = new char[header.size_];
    file.ReadData(static_cast<void*>(fileData), header.size_);
    stringBuilder.Append(fileData);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("export default class"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("static readonly"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("private proxy"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    delete[] fileData;
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: Generate proxy code.
 * EnvConditions: NA
 * CaseDescription: Verify key strings in proxy code.
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxy_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    tsCodeGen_->EmitInterface();
    tsCodeGen_->EmitInterfaceProxy();
    String filePath = String::Format("%s/%s", "./", "idl_test_proxy.ts");
    File file(filePath, File::READ);
    MetaComponent header;
    EXPECT_TRUE(file.ReadData((void*)&header, sizeof(MetaComponent)));
    EXPECT_TRUE(file.Reset());
    char* fileData = new char[header.size_];
    file.ReadData(static_cast<void*>(fileData), header.size_);
    stringBuilder.Append(fileData);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceable"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("new Map()"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("export default class"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("static readonly"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("private proxy"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceable"), ERR_FAIL);
    delete[] fileData;
}

/*
 * Feature: idl
 * Function: EmitInterfaceProxy
 * SubFunction: NA
 * FunctionPoints: EmitInterfaceProxy validates the interface name
 * EnvConditions: NA
 * CaseDescription: Content interface name not start with "I"
 */
HWTEST_F(TsCodeEmitterProxyTest, EmitInterfaceProxy_006, TestSize.Level1)
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
    tsCodeGen_->EmitInterfaceProxy();

    String filePath = String::Format("%s/%s", "./", "special_name_test_proxy.ts");
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
    EXPECT_NE(data.find("class SpecialNameTestProxy implements SpecialNameTest"), std::string::npos);
    EXPECT_NE(data.find("voidParameterTypeShort"), std::string::npos);
    EXPECT_NE(data.find("this.proxy.sendRequest"), std::string::npos);
    free(fileData);
}

/*
 * Feature: idl
 * Function: MethodName
 * SubFunction: NA
 * FunctionPoints: Method Name.
 * EnvConditions: NA
 * CaseDescription: Verify Method Name.
 */
HWTEST_F(TsCodeEmitterProxyTest, MethodName_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("voidOrderReturnMap");
    String relustString = tsCodeGen_->MethodName(inPutString);
    EXPECT_EQ(std::string(relustString.string()), std::string(inPutString.string()));
}

/*
 * Feature: idl
 * Function: MethodName
 * SubFunction: NA
 * FunctionPoints: Method Name.
 * EnvConditions: NA
 * CaseDescription: Verify Method Name.
 */
HWTEST_F(TsCodeEmitterProxyTest, MethodName_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("VoidOrderReturnMap");
    const String expectString("voidOrderReturnMap");
    String relustString = tsCodeGen_->MethodName(inPutString);
    EXPECT_EQ(std::string(relustString.string()), std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: MethodName
 * SubFunction: NA
 * FunctionPoints: Method Name.
 * EnvConditions: NA
 * CaseDescription: Verify Method Name.
 */
HWTEST_F(TsCodeEmitterProxyTest, MethodName_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("VVidOrderReturnMap");
    const String expectString("vVidOrderReturnMap");
    String relustString = tsCodeGen_->MethodName(inPutString);
    EXPECT_EQ(std::string(relustString.string()), std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: MethodName
 * SubFunction: NA
 * FunctionPoints: Method Name.
 * EnvConditions: NA
 * CaseDescription: Verify Method Name.
 */
HWTEST_F(TsCodeEmitterProxyTest, MethodName_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("VVid1254789");
    const String expectString("vVid1254789");
    String relustString = tsCodeGen_->MethodName(inPutString);
    EXPECT_EQ(std::string(relustString.string()), std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: ConstantName
 * SubFunction: NA
 * FunctionPoints: Constant Name.
 * EnvConditions: NA
 * CaseDescription: Verify Constant Name.
 */
HWTEST_F(TsCodeEmitterProxyTest, ConstantName_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("VVid1254789");
    const String expectString("VVID1254789");
    String relustString = tsCodeGen_->ConstantName(inPutString);
    EXPECT_EQ(std::string(relustString.string()), std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: ConstantName
 * SubFunction: NA
 * FunctionPoints: Constant Name.
 * EnvConditions: NA
 * CaseDescription: Verify Constant Name.
 */
HWTEST_F(TsCodeEmitterProxyTest, ConstantName_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("VVidVide");
    const String expectString("VVID_VIDE");
    String relustString = tsCodeGen_->ConstantName(inPutString);
    EXPECT_EQ(std::string(relustString.string()), std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: ConstantName
 * SubFunction: NA
 * FunctionPoints: Constant Name.
 * EnvConditions: NA
 * CaseDescription: Verify Constant Name.
 */
HWTEST_F(TsCodeEmitterProxyTest, ConstantName_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("vvidVide_V");
    const String expectString("VVID_VIDE__V");
    String relustString = tsCodeGen_->ConstantName(inPutString);
    EXPECT_EQ(std::string(relustString.string()), std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: StubName
 * SubFunction: NA
 * FunctionPoints: Stub Name.
 * EnvConditions: NA
 * CaseDescription: Verify Stub Name.
 */
HWTEST_F(TsCodeEmitterProxyTest, StubName_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("IVid1254789");
    const String expectString("Vid1254789Stub");
    String relustString = tsCodeGen_->StubName(inPutString);
    EXPECT_EQ(std::string(relustString.string()), std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: StubName
 * SubFunction: NA
 * FunctionPoints: Stub Name.
 * EnvConditions: NA
 * CaseDescription: Verify Stub Name.
 */
HWTEST_F(TsCodeEmitterProxyTest, StubName_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("iVid1254789");
    const String expectString("iVid1254789Stub");
    String relustString = tsCodeGen_->StubName(inPutString);
    EXPECT_EQ(std::string(relustString.string()), std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: StubName
 * SubFunction: NA
 * FunctionPoints: Stub Name.
 * EnvConditions: NA
 * CaseDescription: Verify Stub Name.
 */
HWTEST_F(TsCodeEmitterProxyTest, StubName_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("Vid1254789");
    const String expectString("Vid1254789Stub");
    String relustString = tsCodeGen_->StubName(inPutString);
    EXPECT_EQ(std::string(relustString.string()), std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: UnderlineAdded
 * SubFunction: NA
 * FunctionPoints: UnderlineAdded Name.
 * EnvConditions: NA
 * CaseDescription: Verify UnderlineAdded.
 */
HWTEST_F(TsCodeEmitterProxyTest, UnderlineAdded_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("Vid1254789");
    const String expectString("_Vid1254789");
    std::string relustString = tsCodeGen_->UnderlineAdded(inPutString);
    EXPECT_EQ(relustString, std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: UnderlineAdded
 * SubFunction: NA
 * FunctionPoints: UnderlineAdded Name.
 * EnvConditions: NA
 * CaseDescription: Verify UnderlineAdded.
 */
HWTEST_F(TsCodeEmitterProxyTest, UnderlineAdded_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("iVid1254789");
    const String expectString("_iVid1254789");
    std::string relustString = tsCodeGen_->UnderlineAdded(inPutString);
    EXPECT_EQ(relustString, std::string(expectString.string()));
}

/*
 * Feature: idl
 * Function: UnderlineAdded
 * SubFunction: NA
 * FunctionPoints: UnderlineAdded Name.
 * EnvConditions: NA
 * CaseDescription: Verify UnderlineAdded.
 */
HWTEST_F(TsCodeEmitterProxyTest, UnderlineAdded_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    ParameterArgv parameters(argvArray, argc);
    EXPECT_EQ(Ready(argc, parameters.GetArgv()), ERR_OK);
    EXPECT_NE(tsCodeGen_, nullptr);
    const String inPutString("_Vid1254789");
    const String expectString("__Vid1254789");
    std::string relustString = tsCodeGen_->UnderlineAdded(inPutString);
    EXPECT_EQ(relustString, std::string(expectString.string()));
}
}
}
}
