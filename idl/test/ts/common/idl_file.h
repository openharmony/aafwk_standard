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

#ifndef OHOS_IDL_MODULE_TEST_IDL_FILE_H
#define OHOS_IDL_MODULE_TEST_IDL_FILE_H

#include <string>

namespace OHOS {
namespace Idl {
namespace TestCommon {
const std::string LICENSE_CONTENT =
"/*\n"
" * Copyright (C) 2022 Device Co., Ltd.\n"
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
const std::string UNKNOW_TYPE_IDL_NAME = "IIdlTest.idl";

const std::string UNKNOW_TYPE_CASE1_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    void voidOrderOutChar([in] char param1, [out] char param2);\n"
"}";

const std::string UNKNOW_TYPE_CASE2_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    void voidOrderOutList([in] List<int> param1, [out] List<int> param2);\n"
"}";

const std::string UNKNOW_TYPE_CASE3_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    void voidOrderOutMapChar([in] Map<int, char> param1, [out] Map<int, char> param2);\n"
"}";

const std::string UNKNOW_TYPE_CASE4_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    void voidOrderOutMapList([in] Map<int, List<int>> param1, [out] Map<int, List<int>> param2);\n"
"}";

const std::string UNKNOW_TYPE_CASE5_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    char voidOrderReturnChar();\n"
"}";

const std::string UNKNOW_TYPE_CASE6_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    List<int> voidOrderReturnList();\n"
"}";

const std::string UNKNOW_TYPE_CASE7_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    Map<int, char> voidOrderReturnMapChar();\n"
"}";

const std::string UNKNOW_TYPE_CASE8_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    Map<int, List<int>> voidOrderReturnMapList();\n"
"}";

const std::string NORMAL_TYPE_IDL_CONTENT =
"interface idl.systemtest.IInterfaceDeclare;\n"
"sequenceable idl.systemtest.MySequenceable;\n"
"interface idl.systemtest.IIdlTest {\n"
"    int voidOrderOutInt([in] int param1, [out] int param2);\n"
"}";

const std::string NO_RETURN_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    voidOrderNoReturn();\n"
"}";

const std::string NO_FUNCTION_NAME_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    int ([in] int param1, [out] int param2);\n"
"}";

const std::string SAME_FUNCTION_NAME_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    int voidOrderOutInt([in] int param1, [out] int param2);\n"
"    int voidOrderOutInt([in] int param1);\n"
"}";

const std::string UNSIGNED_CHAR_TYPE_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    unsigned char voidOrderOutUnsignedChar([in] unsigned char param1, [out] unsigned char param2);\n"
"}";

const std::string UNSIGNED_SHORT_TYPE_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    unsigned short voidOrderOutUnsignedShort([in] unsigned short param1, [out] unsigned short param2);\n"
"}";

const std::string UNSIGNED_INT_TYPE_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    unsigned int voidOrderOutUnsignedInt([in] unsigned int param1, [out] unsigned int param2);\n"
"}";

const std::string MULTIPLE_NAMESPACES_IDL_CONTENT =
"interface idl.systemtest.namespace3.namespace4.IIdlTest {\n"
"    void voidParameterTypeVoid([in] int param1);\n"
"}";

const std::string INOUT_SEPARATE_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    void voidParameterTypeVoid([inout] int param1);\n"
"}";

const std::string INTERFACE_NO_CONTENT_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"}";

const std::string PARAM_NO_INOUT_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    void voidParameterTypeVoid(int param1);\n"
"}";

const std::string INOUT_NO_BIGPARANTHESES_IDL_CONTENT =
"interface idl.systemtest.IIdlTest {\n"
"    void voidParameterTypeVoid(in int param1);\n"
"}";

const std::string NORMAL_TYPE_IN_IDL_CONTENT =
"interface idl.systemtest.IInterfaceDeclare;\n"
"sequenceable idl.systemtest.MySequenceable;\n"
"interface idl.systemtest.IIdlTest {\n"
"    void voidParameterTypeVoid([in] void param1);\n"
"    void voidParameterTypeShort([in] short param1);\n"
"    void voidParameterTypeByte([in] byte param2);\n"
"    void voidParameterTypeInt([in] int param1);\n"
"    void voidParameterTypeLong([in] long param1);\n"
"    void voidParameterTypeFloat([in] float param1);\n"
"    void voidParameterTypeString([in] String param1);\n"
"    void voidParameterTypeDouble([in] double param1);\n"
"    void voidParameterTypeBoolean([in] boolean param1);\n"
"    void voidParameterTypeShortArray([in] short[] param1);\n"
"    void voidParameterTypeByteArray([in] byte[] param2);\n"
"    void voidParameterTypeIntArray([in] int[] param1);\n"
"    void voidParameterTypeLongArray([in] long[] param1);\n"
"    void voidParameterTypeFloatArray([in] float[] param1);\n"
"    void voidParameterTypeStringArray([in] String[] param1);\n"
"    void voidParameterTypeDoubleArray([in] double[] param1);\n"
"    void voidParameterTypeBooleanArray([in] boolean[] param1);\n"
"    void voidParameterTypeMySequenceable([in] MySequenceable mysequenceable);\n"
"    void voidParameterTypeMySequenceableArray([in] MySequenceable[] mysequenceable);\n"
"    void voidParameterTypeInterface([in] IInterfaceDeclare myInterface);\n"
"    void voidMap([in] Map<String, MySequenceable> data);\n"
"}";

const std::string NORMAL_TYPE_OUT_IDL_CONTENT =
"interface idl.systemtest.IInterfaceDeclare;\n"
"sequenceable idl.systemtest.MySequenceable;\n"
"interface idl.systemtest.IIdlTest {\n"
"    void voidParameterTypeVoid([out] void param2);\n"
"    void voidParameterTypeShort([out] short param2);\n"
"    void voidParameterTypeByte([out] byte param2);\n"
"    void voidParameterTypeInt([out] int param2);\n"
"    void voidParameterTypeLong([out] long param2);\n"
"    void voidParameterTypeFloat([out] float param2);\n"
"    void voidParameterTypeString([out] String param2);\n"
"    void voidParameterTypeDouble([out] double param2);\n"
"    void voidParameterTypeBoolean([out] boolean param2);\n"
"    void voidParameterTypeShortArray([out] short[] param2);\n"
"    void voidParameterTypeByteArray([out] byte[] param2);\n"
"    void voidParameterTypeIntArray([out] int[] param2);\n"
"    void voidParameterTypeLongArray([out] long[] param2);\n"
"    void voidParameterTypeFloatArray([out] float[] param2);\n"
"    void voidParameterTypeStringArray([out] String[] param2);\n"
"    void voidParameterTypeDoubleArray([out] double[] param2);\n"
"    void voidParameterTypeBooleanArray([out] boolean[] param2);\n"
"    void voidParameterTypeMySequenceable([out] MySequenceable mysequenceable);\n"
"    void voidParameterTypeMySequenceableArray([out] MySequenceable[] mysequenceable);\n"
"    void voidParameterTypeInterface([out] IInterfaceDeclare myInterface);\n"
"    void voidMap([out] Map<String, MySequenceable> data);\n"
"}";

const std::string NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT =
"interface idl.systemtest.IInterfaceDeclare;\n"
"sequenceable idl.systemtest.MySequenceable;\n"
"interface idl.systemtest.IIdlTest {\n"
"    void voidParameterTypeVoid([inout] void param2);\n"
"    void voidParameterTypeShort([inout] short param2);\n"
"    void voidParameterTypeByte([inout] byte param2);\n"
"    void voidParameterTypeInt([inout] int param2);\n"
"    void voidParameterTypeLong([inout] long param2);\n"
"    void voidParameterTypeFloat([inout] float param2);\n"
"    void voidParameterTypeString([inout] String param2);\n"
"    void voidParameterTypeDouble([inout] double param2);\n"
"    void voidParameterTypeBoolean([inout] boolean param2);\n"
"    void voidParameterTypeShortArray([inout] short[] param2);\n"
"    void voidParameterTypeByteArray([inout] byte[] param2);\n"
"    void voidParameterTypeIntArray([inout] int[] param2);\n"
"    void voidParameterTypeLongArray([inout] long[] param2);\n"
"    void voidParameterTypeFloatArray([inout] float[] param2);\n"
"    void voidParameterTypeStringArray([inout] String[] param2);\n"
"    void voidParameterTypeDoubleArray([inout] double[] param2);\n"
"    void voidParameterTypeBooleanArray([inout] boolean[] param2);\n"
"    void voidParameterTypeMySequenceable([inout] MySequenceable mysequenceable);\n"
"    void voidParameterTypeMySequenceableArray([inout] MySequenceable[] mysequenceable);\n"
"    void voidParameterTypeInterface([inout] IInterfaceDeclare myInterface);\n"
"    void voidMap([inout] Map<String, MySequenceable> data);\n"
"}";

const std::string NORMAL_TYPE_RETURN_IDL_CONTENT =
"interface idl.systemtest.IInterfaceDeclare;\n"
"sequenceable idl.systemtest.MySequenceable;\n"
"interface idl.systemtest.IIdlTest {\n"
"    void voidParameterTypeVoid();\n"
"    short voidParameterTypeShort();\n"
"    byte voidParameterTypeByte();\n"
"    int voidParameterTypeInt();\n"
"    long voidParameterTypeLong();\n"
"    float voidParameterTypeFloat();\n"
"    String voidParameterTypeString();\n"
"    double voidParameterTypeDouble();\n"
"    boolean voidParameterTypeBoolean();\n"
"    short[] voidParameterTypeShortArray();\n"
"    byte[] voidParameterTypeByteArray();\n"
"    int[] voidParameterTypeIntArray();\n"
"    long[] voidParameterTypeLongArray();\n"
"    float[] voidParameterTypeFloatArray();\n"
"    String[] voidParameterTypeStringArray();\n"
"    double[] voidParameterTypeDoubleArray();\n"
"    boolean[] voidParameterTypeBooleanArray();\n"
"    MySequenceable voidParameterTypeMySequenceable();\n"
"    MySequenceable[] voidParameterTypeMySequenceableArray();\n"
"    Map<String, MySequenceable> voidMap();\n"
"}";

const std::string NORMAL_TYPE_COMBINATION_IDL_CONTENT =
"interface idl.systemtest.IInterfaceDeclare;\n"
"sequenceable idl.systemtest.MySequenceable;\n"
"interface idl.systemtest.IIdlTest {\n"
"    short voidParameterTypeShort([in] short param1, [out] short param2, [inout] short param3);\n"
"    byte voidParameterTypeByte([in] byte param1, [out] byte param2, [inout] byte param3);\n"
"    short[] voidParameterTypeShortArray([in] short[] param1, [out] short[] param2, [inout] short[] param3);\n"
"    int[] voidParameterTypeIntArray([in] int[] param1, [inout] int[] param2 , [out] int[] param3);\n"
"    long[] voidParameterTypeLongArray([in] long[] param1, [inout] long[] param2, [out] long[] param3);\n"
"    float[] voidParameterTypeFloatArray([in] float[] param1, [inout] float[] param2, [out] float[] param3);\n"
"    String[] voidParameterTypeStringArray([in] String[] param1, [inout] String[] param2, [out] String[] param3);\n"
"    double[] voidParameterTypeDoubleArray([in] double[] param1, [inout] double[] param2, [out] double[] param3);\n"
"    boolean[] voidParameterTypeBooleanArray([in] boolean[] param1,"
"[inout] boolean[] param2, [out] boolean[] param3);\n"
"    byte[] voidParameterTypeByteArray([in] byte[] param1, [out] byte[] param2, [inout] byte[] param3);\n"
"    MySequenceable TestIntTransaction([in] MySequenceable mysequenceable1,"
"[in, out] MySequenceable mysequenceable2, [out] MySequenceable mysequenceable3);\n"
"    MySequenceable[] voidParameterTypeMySequenceableArray([in] MySequenceable[] mysequenceable1,"
"[inout] MySequenceable[] mysequenceable2, [out] MySequenceable[] mysequenceable3);\n"
"    void InOutMap([inout] Map<String, MySequenceable> data);\n"
"    void InOutMapArray([inout] Map<String, MySequenceable[]> data);\n"
"}";


const std::string NORMAL_TYPE_COMBINATION_IDL_CONTENT2 =
"interface idl.systemtest.IInterfaceDeclare;\n"
"sequenceable idl.systemtest.MySequenceable;\n"
"interface idl.systemtest.IIdlTest {\n"
"    short voidParameterTypeShort([in] short param1, [out] short param2, [inout] short param3);\n"
"    byte voidParameterTypeByte([in] float param1, [out] byte param2, [inout] byte param3);\n"
"    short[] voidParameterTypeShortArray([in] short[] param1,"
"[out] short[] param2, [inout] MySequenceable[] param3);\n"
"    int[] voidParameterTypeIntArray([in] int[] param1,"
"[inout] int[] param2 , [out] int[] param3);\n"
"    long[] voidParameterTypeLongArray([in] short[] param1,"
"[inout] MySequenceable[] param2, [out] long[] param3);\n"
"    float[] voidParameterTypeFloatArray([in] float[] param1,"
"[inout] float[] param2, [out] float[] param3);\n"
"    String[] voidParameterTypeStringArray([in] MySequenceable[] param1,"
"[inout] short[] param2, [out] String[] param3);\n"
"}";

const std::string CAPITALIZATION_IDL_NAME = "CapitalizationIdlTest.idl";

const std::string CAPITALIZATION_VOID_TYPE_IDL_CONTENT =
"interface idl.systemtest.CapitalizationIdlTest {\n"
"    VOID voidOrderReturnVoid();\n"
"}";

const std::string CAPITALIZATION_BOOL_TYPE_IDL_CONTENT =
"interface idl.systemtest.CapitalizationIdlTest {\n"
"    BOOLEAN voidOrderReturnBOOLEAN();\n"
"}";

const std::string CAPITALIZATION_INT_TYPE_IDL_CONTENT =
"interface idl.systemtest.CapitalizationIdlTest {\n"
"    INT voidOrderReturnINT();\n"
"}";

const std::string CAPITALIZATION_STRING_TYPE_IDL_CONTENT =
"interface idl.systemtest.CapitalizationIdlTest {\n"
"    STRING voidOrderReturnSTRING();\n"
"}";

const std::string CAPITALIZATION_INTERFACE_TYPE_IDL_CONTENT =
"INTERFACE idl.systemtest.CapitalizationIdlTest {\n"
"    int voidOrderReturnInt();\n"
"}";

const std::string CAPITALIZATION_IN_TYPE_IDL_CONTENT =
"interface idl.systemtest.CapitalizationIdlTest {\n"
"    int voidOrderReturnInt([IN] int param1);\n"
"}";

const std::string CAPITALIZATION_OUT_TYPE_IDL_CONTENT =
"interface idl.systemtest.CapitalizationIdlTest {\n"
"    void voidOrderReturnVoid([OUT] int param1);\n"
"}";

const std::string CAPITALIZATION_ONEWAY_TYPE_IDL_CONTENT =
"interface idl.systemtest.CapitalizationIdlTest {\n"
"    [ONEWAY] void voidOrderReturnVoid();\n"
"}";

const std::string CAPITALIZATION_SEQUENCEABLE_TYPE_IDL_CONTENT =
"interface idl.systemtest.IInterfaceDeclare;\n"
"SEQUENCEABLE idl.systemtest.MySequenceable;\n"
"interface idl.systemtest.CapitalizationIdlTest {\n"
"    void voidOrderReturnVoid([in] MySequenceable mysequenceable);\n"
"}";

const std::string CAPITALIZATION_MAP_TYPE_IDL_CONTENT =
"interface idl.systemtest.CapitalizationIdlTest {\n"
"    MAP<int, int> voidOrderReturnMap();\n"
"}";

const std::string NO_LICENSE_IDL_NAME = "NoLicenseIdlTest.idl";

const std::string NO_LICENSE_VOID_TYPE_IDL_CONTENT =
"interface idl.systemtest.NoLicenseIdlTest {\n"
"    void voidOrderReturnVoid();\n"
"}";

const std::string NO_LICENSE_BOOL_TYPE_IDL_CONTENT =
"interface idl.systemtest.NoLicenseIdlTest {\n"
"    boolean voidOrderReturnBOOLEAN();\n"
"}";

const std::string NO_LICENSE_INT_TYPE_IDL_CONTENT =
"interface idl.systemtest.NoLicenseIdlTest {\n"
"    int voidOrderReturnINT();\n"
"}";

const std::string NO_LICENSE_STRING_TYPE_IDL_CONTENT =
"interface idl.systemtest.NoLicenseIdlTest {\n"
"    String voidOrderReturnSTRING();\n"
"}";

const std::string NO_LICENSE_INTERFACE_TYPE_IDL_CONTENT =
"interface idl.systemtest.IInterfaceDeclare;\n"
"interface idl.systemtest.NoLicenseIdlTest {\n"
"    void inInterface([in] IInterfaceDeclare obj);\n"
"}";

const std::string NO_LICENSE_IN_TYPE_IDL_CONTENT =
"interface idl.systemtest.NoLicenseIdlTest {\n"
"    int voidOrderReturnInt([in] int param1);\n"
"}";

const std::string NO_LICENSE_OUT_TYPE_IDL_CONTENT =
"interface idl.systemtest.NoLicenseIdlTest {\n"
"    void voidOrderReturnVoid([out] int param1);\n"
"}";

const std::string NO_LICENSE_ONEWAY_TYPE_IDL_CONTENT =
"interface idl.systemtest.NoLicenseIdlTest {\n"
"    [oneway] void voidOrderReturnVoid();\n"
"}";

const std::string NO_LICENSE_SEQUENCEABLE_TYPE_IDL_CONTENT =
"sequenceable idl.systemtest.MySequenceable;\n"
"interface idl.systemtest.NoLicenseIdlTest {\n"
"    void voidOrderReturnVoid([in] MySequenceable mysequenceable);\n"
"}";

const std::string NO_LICENSE_MAP_TYPE_IDL_CONTENT =
"interface idl.systemtest.NoLicenseIdlTest {\n"
"    Map<int, int> voidOrderReturnMap();\n"
"}";

const std::string NO_INTERFACE_IDL_NAME = "NoInterfaceIdlTest.idl";

const std::string NO_INTERFACE_VOID_TYPE_IDL_CONTENT =
"idl.systemtest.NoInterfaceIdlTest {\n"
"    void voidOrderReturnVoid();\n"
"}";

const std::string NO_INTERFACE_BOOL_TYPE_IDL_CONTENT =
"idl.systemtest.NoInterfaceIdlTest {\n"
"    boolean voidOrderReturnBOOLEAN();\n"
"}";

const std::string NO_INTERFACE_INT_TYPE_IDL_CONTENT =
"idl.systemtest.NoInterfaceIdlTest {\n"
"    int voidOrderReturnINT();\n"
"}";

const std::string NO_INTERFACE_STRING_TYPE_IDL_CONTENT =
"idl.systemtest.NoInterfaceIdlTest {\n"
"    String voidOrderReturnSTRING();\n"
"}";

const std::string NO_INTERFACE_INTERFACE_TYPE_IDL_CONTENT =
"interface idl.systemtest.IInterfaceDeclare;\n"
"idl.systemtest.NoInterfaceIdlTest {\n"
"    int voidOrderReturnInt([in] IInterfaceDeclare obj);\n"
"}";

const std::string NO_INTERFACE_IN_TYPE_IDL_CONTENT =
"idl.systemtest.NoInterfaceIdlTest {\n"
"    int voidOrderReturnInt([in] int param1);\n"
"}";

const std::string NO_INTERFACE_OUT_TYPE_IDL_CONTENT =
"idl.systemtest.NoInterfaceIdlTest {\n"
"    void voidOrderReturnVoid([out] int param1);\n"
"}";

const std::string NO_INTERFACE_ONEWAY_TYPE_IDL_CONTENT =
"idl.systemtest.NoInterfaceIdlTest {\n"
"    [oneway] void voidOrderReturnVoid();\n"
"}";

const std::string NO_INTERFACE_SEQUENCEABLE_TYPE_IDL_CONTENT =
"sequenceable idl.systemtest.MySequenceable;\n"
"idl.systemtest.NoInterfaceIdlTest {\n"
"    void voidOrderReturnVoid([in] MySequenceable mysequenceable);\n"
"}";

const std::string NO_INTERFACE_MAP_TYPE_IDL_CONTENT =
"idl.systemtest.NoInterfaceIdlTest {\n"
"    Map<int, int> voidOrderReturnMap();\n"
"}";

const std::string I_ABNORMAL_IDL_NAME = "IAbnormalIdlTest.idl";

const std::string IN_KEYWORD_AFTER_TYPE_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"    void voidReturn(int [in] data);\n"
"}";

const std::string NOT_EQUAL_TO_FILENAME_IDL_CONTENT =
"interface idl.NotEqualToFileNameTest {\n"
"    void voidReturn([in] int data);\n"
"}";

const std::string I_ABNORMAL_SCENE_IDL_NAME = "IAbnormalIdlTest.abc";

const std::string ABNORMAL_SCENE_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"    void voidReturn([in] int data);\n"
"}";

const std::string UNSIGNED_CHAR_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"    void voidReturn([in] unsigned char data);\n"
"}";

const std::string UNSIGNED_SHORT_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"    void voidReturn([in] unsigned short data);\n"
"}";

const std::string UNSIGNED_INT_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"    void voidReturn([in] unsigned int data);\n"
"}";

const std::string UNSIGNED_LONG_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"    void voidReturn([in] unsigned long data);\n"
"}";

const std::string EMPTY_IDL_CONTENT = "";

const std::string NOT_RETURN_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"    voidReturn([in] int data);\n"
"}";

const std::string NOT_FUNCTION_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"   void ([in] int data);\n"
"}";

const std::string SAME_FUNCTION_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"   void func([in] int data);\n"
"   void func([in] int data, [in] int size);\n"
"}";

const std::string FUNCTION_NAME_TOO_LONG_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"   void funcdsadasdsadasdasdasdasdasdasdassadasdasdasdasdasd\
asdasdasdasdasdasdasddddddddddddddddddddddddddddddddddddddddd\
dsaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaassddd\
asddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\
saddddddddddddddddsssssssssssssssssssssssssssssssssssssssssss\
sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss\
sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss\
sssssssssssssssssssssssssssssssssssssssssss([in] int data);\n"
"}";

const std::string VARIABLE_NAME_TOO_LONG_IDL_CONTENT =
"interface idl.IAbnormalIdlTest {\n"
"   void func([in] int datadddddddddddddddddddddddddddddddddddd\
ddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\
ddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\
ddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\
ddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\
ddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\
ddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\
ddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd);\n"
"}";

const std::string INTERFACE_SPECIAL_NAME_IDL_NAME = "SpecialNameTest.idl";

const std::string INTERFACE_SPECIAL_NAME_IDL_CONTENT =
"interface idl.systemtest.SpecialNameTest {\n"
"    void voidParameterTypeShort([in] short param1);\n"
"}";
}
}
}
#endif  // OHOS_AAFWK_ABILITY_CONFIG_H