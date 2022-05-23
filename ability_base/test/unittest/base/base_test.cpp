/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "light_refbase.h"
#include "base_def.h"
#include "base_object.h"
#include "bool_wrapper.h"
#include "zchar_wrapper.h"
#include "byte_wrapper.h"
#include "short_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "float_wrapper.h"
#include "double_wrapper.h"
#include "string_wrapper.h"
#include "refbase.h"

using namespace OHOS;
using namespace OHOS::AAFwk;
using testing::ext::TestSize;

namespace OHOS {
namespace AAFwk {
class AAFwkBaseTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AAFwkBaseTest::SetUpTestCase(void)
{}

void AAFwkBaseTest::TearDownTestCase(void)
{}

void AAFwkBaseTest::SetUp(void)
{}

void AAFwkBaseTest::TearDown(void)
{}

class LightRefCountBaseTestClass : public LightRefCountBase {
public:
    LightRefCountBaseTestClass()
    {
        gDestructorCalled_ = false;
    }

    virtual ~LightRefCountBaseTestClass()
    {
        gDestructorCalled_ = true;
    }

public:
    static bool gDestructorCalled_;
};

bool LightRefCountBaseTestClass::gDestructorCalled_ = false;

class RefBaseTestClass : public RefBase {
public:
    RefBaseTestClass()
    {
        gDestructorCalled_ = false;
    }

    virtual ~RefBaseTestClass()
    {
        gDestructorCalled_ = true;
    }

public:
    static bool gDestructorCalled_;
};

bool RefBaseTestClass::gDestructorCalled_ = false;

CLASS(ObjectTestClass, 5afc4756 - 8f3c - 4d80 - a88b - 54521890beca)
{
public:
    ObjectTestClass(
        /* [in] */ int type)
        : type_(type)
    {
        gDestructorCalled_ = false;
    }

    ~ObjectTestClass()
    {
        gDestructorCalled_ = true;
    }

    OBJECT_DECL();

    int GetHashCode() override
    {
        return 19;
    }

    bool Equals(
        /* [in] */
        IObject * other) override
    {
        if (other == nullptr) {
            return false;
        }

        if (other->GetClassID() == CID_ObjectTestClass) {
            return type_ == static_cast<ObjectTestClass *>(other)->type_;
        }

        return false;
    }

    std::string ToString() override
    {
        return std::string("object[") + std::to_string(reinterpret_cast<uintptr_t>(this)) + std::string("]type_=") +
               std::to_string(type_) + std::string(";");
    }

public:
    static bool gDestructorCalled_;

private:
    int type_;
};

const ClassID CID_ObjectTestClass = {
    0x5afc4756, 0x8f3c, 0x4d80, 0xa88b, {
        0x5, 0x4, 0x5, 0x2, 0x1, 0x8, 0x9, 0x0, 0xb, 0xe, 0xc, 0xa
        }
    };

bool ObjectTestClass::gDestructorCalled_ = false;

OBJECT_IMPL(ObjectTestClass);

/*
 * Feature: LightRefCountBase
 * Function: IncStrongRef, DecStrongRef and GetRefCount
 * SubFunction: NA
 * FunctionPoints: IncStrongRef, DecStrongRef and GetRefCount
 * EnvConditions: NA
 * CaseDescription: Verify IncStrongRef, DecStrongRef and GetRefCount methods of RefBase.
 */
HWTEST_F(AAFwkBaseTest, LightRefCountBase_test_001, TestSize.Level1)
{
    sptr<LightRefCountBaseTestClass> testObject = new LightRefCountBaseTestClass();
    EXPECT_FALSE(LightRefCountBaseTestClass::gDestructorCalled_);
    EXPECT_EQ(testObject->GetRefCount(), 1);
    testObject = nullptr;
    EXPECT_TRUE(LightRefCountBaseTestClass::gDestructorCalled_);
}

/*
 * Feature: RefBase
 * Function: IncStrongRef and DecStrongRef
 * SubFunction: NA
 * FunctionPoints: IncStrongRef and DecStrongRef
 * EnvConditions: NA
 * CaseDescription: Verify IncStrongRef and DecStrongRef methods of RefBase.
 */
HWTEST_F(AAFwkBaseTest, RefBase_test_001, TestSize.Level1)
{
    sptr<RefBaseTestClass> testObject = new RefBaseTestClass();
    EXPECT_FALSE(RefBaseTestClass::gDestructorCalled_);
    testObject = nullptr;
    EXPECT_TRUE(RefBaseTestClass::gDestructorCalled_);
}

/*
 * Feature: Object
 * Function: GetInterfaceID
 * SubFunction: NA
 * FunctionPoints: GetInterfaceID
 * EnvConditions: NA
 * CaseDescription: Verify the GetInterfaceID method of subclass.
 */
HWTEST_F(AAFwkBaseTest, object_test_001, TestSize.Level1)
{
    sptr<ObjectTestClass> testObject = new ObjectTestClass(999);
    sptr<IObject> object = static_cast<IObject *>(testObject.GetRefPtr());
    EXPECT_EQ(g_IID_IObject, object->GetInterfaceID(object));
}

/*
 * Feature: Object
 * Function: GetClassID
 * SubFunction: NA
 * FunctionPoints: GetClassID
 * EnvConditions: NA
 * CaseDescription: Verify the override GetClassID method of subclass.
 */
HWTEST_F(AAFwkBaseTest, object_test_002, TestSize.Level1)
{
    sptr<ObjectTestClass> testObject = new ObjectTestClass(999);
    EXPECT_EQ(CID_ObjectTestClass, testObject->GetClassID());
    testObject = nullptr;
    EXPECT_TRUE(ObjectTestClass::gDestructorCalled_);
}

/*
 * Feature: Object
 * Function: GetHashCode
 * SubFunction: NA
 * FunctionPoints: GetHashCode
 * EnvConditions: NA
 * CaseDescription: Verify the override GetHashCode method of subclass.
 */
HWTEST_F(AAFwkBaseTest, object_test_003, TestSize.Level1)
{
    sptr<ObjectTestClass> testObject = new ObjectTestClass(999);
    EXPECT_EQ(19, testObject->GetHashCode());
    testObject = nullptr;
    EXPECT_TRUE(ObjectTestClass::gDestructorCalled_);
}

/*
 * Feature: Object
 * Function: Equal
 * SubFunction: NA
 * FunctionPoints: Equal
 * EnvConditions: NA
 * CaseDescription: Verify the override Equal method of subclass.
 */
HWTEST_F(AAFwkBaseTest, object_test_004, TestSize.Level1)
{
    sptr<ObjectTestClass> testObject1 = new ObjectTestClass(999);
    sptr<ObjectTestClass> testObject2 = new ObjectTestClass(9999);
    sptr<ObjectTestClass> testObject3 = new ObjectTestClass(999);
    EXPECT_FALSE(testObject1->Equals(testObject2));
    EXPECT_TRUE(testObject1->Equals(testObject3));
}

/*
 * Feature: Object
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify the override ToString method of subclass.
 */
HWTEST_F(AAFwkBaseTest, object_test_005, TestSize.Level1)
{
    sptr<ObjectTestClass> testObject1 = new ObjectTestClass(999);
    std::string objectStr = testObject1->ToString();
    EXPECT_TRUE(objectStr.find("type_=999") != std::string::npos);
}

/*
 * Feature: Object
 * Function: GetWeakReference
 * SubFunction: NA
 * FunctionPoints: GetWeakReference
 * EnvConditions: NA
 * CaseDescription: Verify GetWeakReference method of subclass.
 */
HWTEST_F(AAFwkBaseTest, object_test_006, TestSize.Level1)
{
    sptr<ObjectTestClass> testObject1 = new ObjectTestClass(999);
    EXPECT_FALSE(ObjectTestClass::gDestructorCalled_);
    sptr<IWeakReference> weakRef;
    testObject1->GetWeakReference(weakRef);
    EXPECT_TRUE(weakRef != nullptr);
    sptr<IObject> object;
    weakRef->Resolve(g_IID_IObject, reinterpret_cast<IInterface **>(&object));
    EXPECT_TRUE(object != nullptr);
    EXPECT_EQ(static_cast<ObjectTestClass *>(object.GetRefPtr())->GetHashCode(), 19);
    testObject1 = nullptr;
    object = nullptr;
    EXPECT_TRUE(ObjectTestClass::gDestructorCalled_);
    weakRef->Resolve(g_IID_IObject, reinterpret_cast<IInterface **>(&object));
    EXPECT_TRUE(object == nullptr);
}

/*
 * Feature: wptr
 * Function: wptr
 * SubFunction: NA
 * FunctionPoints: wptr
 * EnvConditions: NA
 * CaseDescription: Verify wptr.
 */
HWTEST_F(AAFwkBaseTest, object_test_007, TestSize.Level1)
{
    sptr<ObjectTestClass> testObject1 = new ObjectTestClass(999);
    EXPECT_FALSE(ObjectTestClass::gDestructorCalled_);
    wptr<ObjectTestClass> weakObject(testObject1);
    testObject1 = nullptr;
    EXPECT_TRUE(ObjectTestClass::gDestructorCalled_);
    EXPECT_TRUE(weakObject.promote() == nullptr);
}

/*
 * Feature: Boolean
 * Function: GetValue
 * SubFunction: NA
 * FunctionPoints: GetValue
 * EnvConditions: NA
 * CaseDescription: Verify GetValue method of Boolean.
 */
HWTEST_F(AAFwkBaseTest, boolean_test_001, TestSize.Level1)
{
    sptr<Boolean> boolean = new Boolean(true);
    bool value = false;
    boolean->GetValue(&value);
    EXPECT_TRUE(value);
    boolean = new Boolean(false);
    boolean->GetValue(&value);
    EXPECT_FALSE(value);
}

/*
 * Feature: Boolean
 * Function: Box and Unbox
 * SubFunction: NA
 * FunctionPoints: Box and Unbox
 * EnvConditions: NA
 * CaseDescription: Verify Box and Unbox method of Boolean.
 */
HWTEST_F(AAFwkBaseTest, boolean_test_002, TestSize.Level1)
{
    sptr<IBoolean> boolean = Boolean::Box(true);
    EXPECT_TRUE(Boolean::Unbox(boolean));
    boolean = Boolean::Box(false);
    EXPECT_FALSE(Boolean::Unbox(boolean));
}

/*
 * Feature: Boolean
 * Function: Parse
 * SubFunction: NA
 * FunctionPoints: Parse
 * EnvConditions: NA
 * CaseDescription: Verify Parse method of Boolean.
 */
HWTEST_F(AAFwkBaseTest, boolean_test_003, TestSize.Level1)
{
    sptr<IBoolean> boolean = Boolean::Parse("true");
    EXPECT_TRUE(Boolean::Unbox(boolean));
    boolean = Boolean::Parse("false");
    EXPECT_FALSE(Boolean::Unbox(boolean));
}

/*
 * Feature: Boolean
 * Function: Equals
 * SubFunction: NA
 * FunctionPoints: Equals
 * EnvConditions: NA
 * CaseDescription: Verify Equals method of Boolean.
 */
HWTEST_F(AAFwkBaseTest, boolean_test_004, TestSize.Level1)
{
    sptr<Boolean> boolean1 = new Boolean(true);
    sptr<Boolean> boolean2 = new Boolean(false);
    sptr<Boolean> boolean3 = new Boolean(true);
    EXPECT_FALSE(boolean1->Equals(boolean2));
    EXPECT_TRUE(boolean1->Equals(boolean3));
}

/*
 * Feature: Boolean
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify ToString method of Boolean.
 */
HWTEST_F(AAFwkBaseTest, boolean_test_005, TestSize.Level1)
{
    EXPECT_EQ(Object::ToString(Boolean::Box(true)), "true");
    EXPECT_EQ(Object::ToString(Boolean::Box(false)), "false");
}

/*
 * Feature: Char
 * Function: GetValue
 * SubFunction: NA
 * FunctionPoints: GetValue
 * EnvConditions: NA
 * CaseDescription: Verify GetValue method of Char.
 */
HWTEST_F(AAFwkBaseTest, char_test_001, TestSize.Level1)
{
    sptr<Char> charObj = new Char(U'中');
    zchar value;
    charObj->GetValue(&value);
    EXPECT_EQ(value, U'中');
}

/*
 * Feature: Char
 * Function: Box and Unbox
 * SubFunction: NA
 * FunctionPoints: Box and Unbox
 * EnvConditions: NA
 * CaseDescription: Verify Box and Unbox method of Char.
 */
HWTEST_F(AAFwkBaseTest, char_test_002, TestSize.Level1)
{
    sptr<IChar> charObj = Char::Box(U'天');
    EXPECT_EQ(Char::Unbox(charObj), U'天');
}

/*
 * Feature: Char
 * Function: Parse
 * SubFunction: NA
 * FunctionPoints: Parse
 * EnvConditions: NA
 * CaseDescription: Verify Parse method of Char.
 */
HWTEST_F(AAFwkBaseTest, char_test_003, TestSize.Level1)
{
    sptr<IChar> charObj = Char::Parse("气");
    EXPECT_EQ(Char::Unbox(charObj), U'气');
}

/*
 * Feature: Char
 * Function: Equals
 * SubFunction: NA
 * FunctionPoints: Equals
 * EnvConditions: NA
 * CaseDescription: Verify Equals method of Char.
 */
HWTEST_F(AAFwkBaseTest, char_test_004, TestSize.Level1)
{
    sptr<Char> charObj1 = new Char(U'H');
    sptr<Char> charObj2 = new Char('I');
    sptr<Char> charObj3 = new Char(U'H');
    EXPECT_FALSE(charObj1->Equals(charObj2));
    EXPECT_TRUE(charObj1->Equals(charObj3));
}

/*
 * Feature: Char
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify ToString method of Char.
 */
HWTEST_F(AAFwkBaseTest, char_test_005, TestSize.Level1)
{
    EXPECT_EQ(Object::ToString(Char::Box(U'好')), "好");
}

/*
 * Feature: Char
 * Function: GetChar
 * SubFunction: NA
 * FunctionPoints: GetChar
 * EnvConditions: NA
 * CaseDescription: Verify GetChar method of Char.
 */
HWTEST_F(AAFwkBaseTest, char_test_006, TestSize.Level1)
{
    std::string str = "今天气温真不错有23摄氏度呢！";
    EXPECT_EQ(Char::GetChar(str, 0), U'今');
    EXPECT_EQ(Char::GetChar(str, 1), U'天');
    EXPECT_EQ(Char::GetChar(str, 2), U'气');
    EXPECT_EQ(Char::GetChar(str, 3), U'温');
    EXPECT_EQ(Char::GetChar(str, 4), U'真');
    EXPECT_EQ(Char::GetChar(str, 5), U'不');
    EXPECT_EQ(Char::GetChar(str, 6), U'错');
    EXPECT_EQ(Char::GetChar(str, 7), U'有');
    EXPECT_EQ(Char::GetChar(str, 8), '2');
    EXPECT_EQ(Char::GetChar(str, 9), '3');
    EXPECT_EQ(Char::GetChar(str, 10), U'摄');
    EXPECT_EQ(Char::GetChar(str, 11), U'氏');
    EXPECT_EQ(Char::GetChar(str, 12), U'度');
    EXPECT_EQ(Char::GetChar(str, 13), U'呢');
    EXPECT_EQ(Char::GetChar(str, 14), U'！');
}

/*
 * Feature: Byte
 * Function: GetValue
 * SubFunction: NA
 * FunctionPoints: GetValue
 * EnvConditions: NA
 * CaseDescription: Verify GetValue method of Byte.
 */
HWTEST_F(AAFwkBaseTest, byte_test_001, TestSize.Level1)
{
    sptr<Byte> byteObj = new Byte(129);
    byte value;
    byteObj->GetValue(&value);
    EXPECT_EQ(value, 129);
}

/*
 * Feature: Byte
 * Function: Box and Unbox
 * SubFunction: NA
 * FunctionPoints: Box and Unbox
 * EnvConditions: NA
 * CaseDescription: Verify Box and Unbox method of Byte.
 */
HWTEST_F(AAFwkBaseTest, byte_test_002, TestSize.Level1)
{
    sptr<IByte> byteObj = Byte::Box(129);
    EXPECT_EQ(Byte::Unbox(byteObj), 129);
}

/*
 * Feature: Byte
 * Function: Parse
 * SubFunction: NA
 * FunctionPoints: Parse
 * EnvConditions: NA
 * CaseDescription: Verify Parse method of Byte.
 */
HWTEST_F(AAFwkBaseTest, byte_test_003, TestSize.Level1)
{
    sptr<IByte> byteObj = Byte::Parse("129");
    EXPECT_EQ(Byte::Unbox(byteObj), 129);
}

/*
 * Feature: Byte
 * Function: Equals
 * SubFunction: NA
 * FunctionPoints: Equals
 * EnvConditions: NA
 * CaseDescription: Verify Equals method of Byte.
 */
HWTEST_F(AAFwkBaseTest, byte_test_004, TestSize.Level1)
{
    sptr<Byte> byteObj1 = new Byte(129);
    sptr<Byte> byteObj2 = new Byte(130);
    sptr<Byte> byteObj3 = new Byte(129);
    EXPECT_FALSE(byteObj1->Equals(byteObj2));
    EXPECT_TRUE(byteObj1->Equals(byteObj3));
}

/*
 * Feature: Byte
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify ToString method of Byte.
 */
HWTEST_F(AAFwkBaseTest, byte_test_005, TestSize.Level1)
{
    EXPECT_EQ(Object::ToString(Byte::Box(129)), "129");
}

/*
 * Feature: Short
 * Function: GetValue
 * SubFunction: NA
 * FunctionPoints: GetValue
 * EnvConditions: NA
 * CaseDescription: Verify GetValue method of Short.
 */
HWTEST_F(AAFwkBaseTest, short_test_001, TestSize.Level1)
{
    sptr<Short> shortObj = new Short(32767);
    short value;
    shortObj->GetValue(&value);
    EXPECT_EQ(value, 32767);
}

/*
 * Feature: Short
 * Function: Box and Unbox
 * SubFunction: NA
 * FunctionPoints: Box and Unbox
 * EnvConditions: NA
 * CaseDescription: Verify Box and Unbox method of Short.
 */
HWTEST_F(AAFwkBaseTest, short_test_002, TestSize.Level1)
{
    sptr<IShort> shortObj = Short::Box(32767);
    EXPECT_EQ(Short::Unbox(shortObj), 32767);
}

/*
 * Feature: Short
 * Function: Parse
 * SubFunction: NA
 * FunctionPoints: Parse
 * EnvConditions: NA
 * CaseDescription: Verify Parse method of Short.
 */
HWTEST_F(AAFwkBaseTest, short_test_003, TestSize.Level1)
{
    sptr<IShort> shortObj = Short::Parse("32767");
    EXPECT_EQ(Short::Unbox(shortObj), 32767);
}

/*
 * Feature: Short
 * Function: Equals
 * SubFunction: NA
 * FunctionPoints: Equals
 * EnvConditions: NA
 * CaseDescription: Verify Equals method of Short.
 */
HWTEST_F(AAFwkBaseTest, short_test_004, TestSize.Level1)
{
    sptr<Short> shortObj1 = new Short(32767);
    sptr<Short> shortObj2 = new Short(-32768);
    sptr<Short> shortObj3 = new Short(32767);
    EXPECT_FALSE(shortObj1->Equals(shortObj2));
    EXPECT_TRUE(shortObj1->Equals(shortObj3));
}

/*
 * Feature: Short
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify ToString method of Short.
 */
HWTEST_F(AAFwkBaseTest, short_test_005, TestSize.Level1)
{
    EXPECT_EQ(Object::ToString(Short::Box(32767)), "32767");
}

/*
 * Feature: Integer
 * Function: GetValue
 * SubFunction: NA
 * FunctionPoints: GetValue
 * EnvConditions: NA
 * CaseDescription: Verify GetValue method of Integer.
 */
HWTEST_F(AAFwkBaseTest, integer_test_001, TestSize.Level1)
{
    sptr<Integer> intObj = new Integer(2147483647);
    int value;
    intObj->GetValue(&value);
    EXPECT_EQ(value, 2147483647);
}

/*
 * Feature: Integer
 * Function: Box and Unbox
 * SubFunction: NA
 * FunctionPoints: Box and Unbox
 * EnvConditions: NA
 * CaseDescription: Verify Box and Unbox method of Integer.
 */
HWTEST_F(AAFwkBaseTest, integer_test_002, TestSize.Level1)
{
    sptr<IInteger> intObj = Integer::Box(2147483647);
    EXPECT_EQ(Integer::Unbox(intObj), 2147483647);
}

/*
 * Feature: Integer
 * Function: Parse
 * SubFunction: NA
 * FunctionPoints: Parse
 * EnvConditions: NA
 * CaseDescription: Verify Parse method of Integer.
 */
HWTEST_F(AAFwkBaseTest, integer_test_003, TestSize.Level1)
{
    sptr<IInteger> intObj = Integer::Parse("-1");
    EXPECT_EQ(Integer::Unbox(intObj), -1);
}

/*
 * Feature: Integer
 * Function: Equals
 * SubFunction: NA
 * FunctionPoints: Equals
 * EnvConditions: NA
 * CaseDescription: Verify Equals method of Integer.
 */
HWTEST_F(AAFwkBaseTest, integer_test_004, TestSize.Level1)
{
    sptr<Integer> intObj1 = new Integer(2147483647);
    sptr<Integer> intObj2 = new Integer(-2147483648);
    sptr<Integer> intObj3 = new Integer(2147483647);
    EXPECT_FALSE(intObj1->Equals(intObj2));
    EXPECT_TRUE(intObj1->Equals(intObj3));
}

/*
 * Feature: Integer
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify ToString method of Integer.
 */
HWTEST_F(AAFwkBaseTest, integer_test_005, TestSize.Level1)
{
    EXPECT_EQ(Object::ToString(Integer::Box(-2147483648)), "-2147483648");
}

/*
 * Feature: Long
 * Function: GetValue
 * SubFunction: NA
 * FunctionPoints: GetValue
 * EnvConditions: NA
 * CaseDescription: Verify GetValue method of Long.
 */
HWTEST_F(AAFwkBaseTest, long_test_001, TestSize.Level1)
{
    sptr<Long> longObj = new Long(2147483647L);
    long value;
    longObj->GetValue(&value);
    EXPECT_EQ(value, 2147483647L);
}

/*
 * Feature: Long
 * Function: Box and Unbox
 * SubFunction: NA
 * FunctionPoints: Box and Unbox
 * EnvConditions: NA
 * CaseDescription: Verify Box and Unbox method of Long.
 */
HWTEST_F(AAFwkBaseTest, long_test_002, TestSize.Level1)
{
    sptr<ILong> longObj = Long::Box(2147483647L);
    EXPECT_EQ(Long::Unbox(longObj), 2147483647L);
}

/*
 * Feature: Long
 * Function: Parse
 * SubFunction: NA
 * FunctionPoints: Parse
 * EnvConditions: NA
 * CaseDescription: Verify Parse method of Long.
 */
HWTEST_F(AAFwkBaseTest, long_test_003, TestSize.Level1)
{
    sptr<ILong> longObj = Long::Parse("-1");
    EXPECT_EQ(Long::Unbox(longObj), -1);
}

/*
 * Feature: Long
 * Function: Equals
 * SubFunction: NA
 * FunctionPoints: Equals
 * EnvConditions: NA
 * CaseDescription: Verify Equals method of Long.
 */
HWTEST_F(AAFwkBaseTest, long_test_004, TestSize.Level1)
{
    sptr<Long> longObj1 = new Long(2147483647L);
    sptr<Long> longObj2 = new Long(-2147483647L);
    sptr<Long> longObj3 = new Long(2147483647L);
    EXPECT_FALSE(longObj1->Equals(longObj2));
    EXPECT_TRUE(longObj1->Equals(longObj3));
}

/*
 * Feature: Long
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify ToString method of Long.
 */
HWTEST_F(AAFwkBaseTest, long_test_005, TestSize.Level1)
{
    EXPECT_EQ(Object::ToString(Long::Box(-2147483647L)), "-9223372036854775807");
}

/*
 * Feature: Float
 * Function: GetValue
 * SubFunction: NA
 * FunctionPoints: GetValue
 * EnvConditions: NA
 * CaseDescription: Verify GetValue method of Float.
 */
HWTEST_F(AAFwkBaseTest, float_test_001, TestSize.Level1)
{
    sptr<Float> floatObj = new Float(-1.020);
    float value;
    floatObj->GetValue(&value);
    EXPECT_FLOAT_EQ(value, -1.020);
}

/*
 * Feature: Float
 * Function: Box and Unbox
 * SubFunction: NA
 * FunctionPoints: Box and Unbox
 * EnvConditions: NA
 * CaseDescription: Verify Box and Unbox method of Float.
 */
HWTEST_F(AAFwkBaseTest, float_test_002, TestSize.Level1)
{
    sptr<IFloat> floatObj = Float::Box(-0.003);
    EXPECT_FLOAT_EQ(Float::Unbox(floatObj), -0.003);
}

/*
 * Feature: Float
 * Function: Parse
 * SubFunction: NA
 * FunctionPoints: Parse
 * EnvConditions: NA
 * CaseDescription: Verify Parse method of Float.
 */
HWTEST_F(AAFwkBaseTest, float_test_003, TestSize.Level1)
{
    sptr<IFloat> floatObj = Float::Parse("-1.000400");
    EXPECT_FLOAT_EQ(Float::Unbox(floatObj), -1.0004);
}

/*
 * Feature: Float
 * Function: Equals
 * SubFunction: NA
 * FunctionPoints: Equals
 * EnvConditions: NA
 * CaseDescription: Verify Equals method of Float.
 */
HWTEST_F(AAFwkBaseTest, float_test_004, TestSize.Level1)
{
    sptr<Float> floatObj1 = new Float(0.009);
    sptr<Float> floatObj2 = new Float(-0.001);
    sptr<Float> floatObj3 = new Float(0.009);
    EXPECT_FALSE(floatObj1->Equals(floatObj2));
    EXPECT_TRUE(floatObj1->Equals(floatObj3));
}

/*
 * Feature: Float
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify ToString method of Float.
 */
HWTEST_F(AAFwkBaseTest, float_test_005, TestSize.Level1)
{
    EXPECT_EQ(Object::ToString(Float::Box(-10.00006)), "-10.000060");
}

/*
 * Feature: Double
 * Function: GetValue
 * SubFunction: NA
 * FunctionPoints: GetValue
 * EnvConditions: NA
 * CaseDescription: Verify GetValue method of Double.
 */
HWTEST_F(AAFwkBaseTest, double_test_001, TestSize.Level1)
{
    sptr<Double> doubleObj = new Double(-1.00020);
    double value;
    doubleObj->GetValue(&value);
    EXPECT_DOUBLE_EQ(value, -1.00020);
}

/*
 * Feature: Double
 * Function: Box and Unbox
 * SubFunction: NA
 * FunctionPoints: Box and Unbox
 * EnvConditions: NA
 * CaseDescription: Verify Box and Unbox method of Double.
 */
HWTEST_F(AAFwkBaseTest, double_test_002, TestSize.Level1)
{
    sptr<IDouble> doubleObj = Double::Box(-0.00003);
    EXPECT_DOUBLE_EQ(Double::Unbox(doubleObj), -0.00003);
}

/*
 * Feature: Double
 * Function: Parse
 * SubFunction: NA
 * FunctionPoints: Parse
 * EnvConditions: NA
 * CaseDescription: Verify Parse method of Double.
 */
HWTEST_F(AAFwkBaseTest, double_test_003, TestSize.Level1)
{
    sptr<IDouble> doubleObj = Double::Parse("-1.0000000400");
    EXPECT_DOUBLE_EQ(Double::Unbox(doubleObj), -1.00000004);
}

/*
 * Feature: Double
 * Function: Equals
 * SubFunction: NA
 * FunctionPoints: Equals
 * EnvConditions: NA
 * CaseDescription: Verify Equals method of Double.
 */
HWTEST_F(AAFwkBaseTest, double_test_004, TestSize.Level1)
{
    sptr<Double> doubleObj1 = new Double(0.000009);
    sptr<Double> doubleObj2 = new Double(-0.000001);
    sptr<Double> doubleObj3 = new Double(0.000009);
    EXPECT_FALSE(doubleObj1->Equals(doubleObj2));
    EXPECT_TRUE(doubleObj1->Equals(doubleObj3));
}

/*
 * Feature: Double
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify ToString method of Double.
 */
HWTEST_F(AAFwkBaseTest, double_test_005, TestSize.Level1)
{
    EXPECT_EQ(Object::ToString(Double::Box(-10.000006)), "-10.000006");
}

/*
 * Feature: String
 * Function: GetString
 * SubFunction: NA
 * FunctionPoints: GetString
 * EnvConditions: NA
 * CaseDescription: Verify GetString method of String.
 */
HWTEST_F(AAFwkBaseTest, string_test_001, TestSize.Level1)
{
    sptr<String> stringObj = new String("$hell0-w@rld#");
    std::string string;
    stringObj->GetString(&string);
    EXPECT_EQ(string, std::string("$hell0-w@rld#"));
}

/*
 * Feature: String
 * Function: Box and Unbox
 * SubFunction: NA
 * FunctionPoints: Box and Unbox
 * EnvConditions: NA
 * CaseDescription: Verify Box and Unbox method of String.
 */
HWTEST_F(AAFwkBaseTest, string_test_002, TestSize.Level1)
{
    sptr<IString> stringObj = String::Box("1234567890");
    EXPECT_EQ(String::Unbox(stringObj), std::string("1234567890"));
}

/*
 * Feature: String
 * Function: Parse
 * SubFunction: NA
 * FunctionPoints: Parse
 * EnvConditions: NA
 * CaseDescription: Verify Parse method of String.
 */
HWTEST_F(AAFwkBaseTest, string_test_003, TestSize.Level1)
{
    sptr<IString> stringObj = String::Parse("-1.0000000400");
    EXPECT_EQ(String::Unbox(stringObj), std::string("-1.0000000400"));
}

/*
 * Feature: String
 * Function: Equals
 * SubFunction: NA
 * FunctionPoints: Equals
 * EnvConditions: NA
 * CaseDescription: Verify Equals method of String.
 */
HWTEST_F(AAFwkBaseTest, string_test_004, TestSize.Level1)
{
    sptr<String> stringObj1 = new String("$hell0-w@rld#");
    sptr<String> stringObj2 = new String("-1.0000000400");
    sptr<String> stringObj3 = new String("$hell0-w@rld#");
    EXPECT_FALSE(stringObj1->Equals(stringObj2));
    EXPECT_TRUE(stringObj1->Equals(stringObj3));
}

/*
 * Feature: String
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify ToString method of String.
 */
HWTEST_F(AAFwkBaseTest, string_test_005, TestSize.Level1)
{
    EXPECT_EQ(Object::ToString(String::Box("-10.000006")), std::string("-10.000006"));
}
}  // namespace AAFwk
}  // namespace OHOS
