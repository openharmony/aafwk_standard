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
#include <cfloat>
#include <chrono>
#include <climits>
#include <functional>
#include <random>

#include "ability.h"
#include "values_bucket.h"
#include "../include/get_param.h"

using namespace std;
using namespace OHOS::AppExecFwk;
using Uri = OHOS::Uri;
namespace OHOS {
namespace AppExecFwk {
const int zero = 0;
const int two = 2;
const int CHAR_MIN = -128;
const int CHAR_MAX = 127;
const int STRING_MAX = 256;
bool GetBoolParam()
{
    bool param;
    if (GetIntParam() % two == zero) {
        param = true;
    } else {
        param = false;
    }
    cout << "Bool param is: " << param << endl;
    return param;
}

size_t GenRandom(size_t min, size_t max)
{
    std::random_device rd;
    static uniform_int_distribution<size_t> u(min, max);
    static default_random_engine e(rd());
    size_t param = u(e);
    return param;
}

int8_t GetS8Param()
{
    std::random_device rd;
    static uniform_int_distribution<int8_t> u(INT8_MIN, INT8_MAX);
    static default_random_engine e(rd());
    int8_t param = u(e);
    cout << "Int8_t param is: " << param << endl;
    return param;
}
int16_t GetS16Param()
{
    std::random_device rd;
    static uniform_int_distribution<int16_t> u(INT16_MIN, INT16_MAX);
    static default_random_engine e(rd());
    int16_t param = u(e);
    cout << "Int16_t param is: " << param << endl;
    return param;
}
int32_t GetS32Param()
{
    std::random_device rd;
    static uniform_int_distribution<int32_t> u(INT32_MIN, INT32_MAX);
    static default_random_engine e(rd());
    int32_t param = u(e);
    cout << "Int32_t param is: " << param << endl;
    return param;
}

int64_t GetS64Param()
{
    std::random_device rd;
    static uniform_int_distribution<int64_t> u(INT64_MIN, INT64_MAX);
    static default_random_engine e(rd());
    int64_t param = u(e);
    cout << "Int64_t param is: " << param << endl;
    return param;
}

template<class T>
T GetUnsignParam()
{
    std::random_device rd;
    static uniform_int_distribution<T> u;
    static default_random_engine e(rd());
    T param = u(e);
    return param;
}

size_t GetSizeTParam()
{
    size_t param = GetUnsignParam<size_t>();
    return param;
}

uint8_t GetU8Param()
{
    uint8_t param = GetUnsignParam<uint8_t>();
    cout << "Uint8_t param is: " << param << endl;
    return param;
}

unsigned int GetUIntParam()
{
    unsigned int param = GetUnsignParam<unsigned int>();
    cout << "Unsigned int param is: " << param << endl;
    return param;
}

uint16_t GetU16Param()
{
    uint16_t param = GetUnsignParam<uint16_t>();
    cout << "Uint16_t param is: " << param << endl;
    return param;
}

uint32_t GetU32Param()
{
    uint32_t param = GetUnsignParam<uint32_t>();
    cout << "Uint32_t param is: " << param << endl;
    return param;
}

uint64_t GetU64Param()
{
    uint64_t param = GetUnsignParam<uint64_t>();
    cout << "Uint64_t param is: " << param << endl;
    return param;
}

short GetShortParam()
{
    std::random_device rd;
    static uniform_int_distribution<short> u(SHRT_MIN, SHRT_MAX);
    static default_random_engine e(rd());
    short param = u(e);
    cout << "Short param is: " << param << endl;
    return param;
}

long GetLongParam()
{
    std::random_device rd;
    static uniform_int_distribution<long> u(LONG_MIN, LONG_MAX);
    static default_random_engine e(rd());
    long param = u(e);
    cout << "Long param is: " << param << endl;
    return param;
}

int GetIntParam()
{
    std::random_device rd;
    static uniform_int_distribution<> u(INT_MIN, INT_MAX);
    static default_random_engine e(rd());
    int param = u(e);
    cout << "Int param is: " << param << endl;
    return param;
}

double GetDoubleParam()
{
    double param = 0;
    std::random_device rd;
    static uniform_real_distribution<double> u(DBL_MIN, DBL_MAX);
    static default_random_engine e(rd());
    param = u(e);
    cout << "double param is: " << param << endl;
    return param;
}

float GetFloatParam()
{
    float param = 0;
    std::random_device rd;
    static uniform_real_distribution<float> u(FLT_MIN, FLT_MAX);
    static default_random_engine e(rd());
    param = u(e);
    cout << "Float param is: " << param << endl;
    return param;
}

char GetCharParam()
{
    std::random_device rd;
    static uniform_int_distribution<> u(CHAR_MIN, CHAR_MAX);
    static default_random_engine e(rd());
    char param = u(e);
    return param;
}

char32_t GetChar32Param()
{
    char32_t param = ' ';
    std::random_device rd;
    static uniform_int_distribution<char32_t> u;
    static default_random_engine e(rd());
    param = u(e);
    return param;
}

string GetStringParam()
{
    string param = "";
    char ch = GetCharParam();
    size_t len = GenRandom(0, 255);
    while (len--) {
        ch = GetCharParam();
        param += ch;
    }
    cout << "String param length is: " << param.length() << endl;
    return param;
}

std::map<int64_t, int32_t> GetS64S32MapParam()
{
    map<int64_t, int32_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int64_t int64Value = GetS64Param();
        int32_t int32Value = GetS32Param();
        param.insert(pair<int64_t, int32_t>(int64Value, int32Value));
    }
    cout << "int64_t, int32_t map param length is: " << param.size() << endl;
    return param;
}

template<class T>
vector<T> GetUnsignVectorParam()
{
    vector<T> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        T t = GetUnsignParam<T>();
        param.push_back(t);
    }
    cout << "Uint vector param length is: " << param.size() << endl;
    return param;
}

template<class T>
T GetClassParam()
{
    T param;
    return param;
}

std::vector<bool> GetBoolVectorParam()
{
    vector<bool> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int t = GetBoolParam();
        param.push_back(t);
    }
    cout << "Bool vector param length is: " << param.size() << endl;
    return param;
}

std::vector<short> GetShortVectorParam()
{
    vector<short> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        short t = GetShortParam();
        param.push_back(t);
    }
    cout << "Short vector param length is: " << param.size() << endl;
    return param;
}

std::vector<long> GetLongVectorParam()
{
    vector<long> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        long t = GetLongParam();
        param.push_back(t);
    }
    cout << "Long vector param length is: " << param.size() << endl;
    return param;
}

vector<int> GetIntVectorParam()
{
    vector<int> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int t = GetIntParam();
        param.push_back(t);
    }
    cout << "Int vector param length is: " << param.size() << endl;
    return param;
}

std::vector<float> GetFloatVectorParam()
{
    vector<float> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        float t = GetIntParam();
        param.push_back(t);
    }
    cout << "Float vector param length is: " << param.size() << endl;
    return param;
}

std::vector<double> GetDoubleVectorParam()
{
    vector<double> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        double t = GetIntParam();
        param.push_back(t);
    }
    cout << "Double vector param length is: " << param.size() << endl;
    return param;
}

vector<char> GetCharVectorParam()
{
    vector<char> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        char t = GetCharParam();
        param.push_back(t);
    }
    cout << "Char vector param length is: " << param.size() << endl;
    return param;
}

vector<char32_t> GetChar32VectorParam()
{
    vector<char32_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        char32_t t = GetChar32Param();
        param.push_back(t);
    }
    cout << "Char32_t vector param length is: " << param.size() << endl;
    return param;
}

vector<string> GetStringVectorParam()
{
    vector<string> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        string t = GetStringParam();
        param.push_back(t);
    }
    cout << "String vector param length is: " << param.size() << endl;
    return param;
}

vector<int8_t> GetS8VectorParam()
{
    vector<int8_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int8_t temp = GetS8Param();
        param.push_back(temp);
    }
    cout << "Int8_t vector param length is: " << param.size() << endl;
    return param;
}

vector<int16_t> GetS16VectorParam()
{
    vector<int16_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int16_t temp = GetS16Param();
        param.push_back(temp);
    }
    cout << "Int16_t vector param length is: " << param.size() << endl;
    return param;
}

vector<int32_t> GetS32VectorParam()
{
    vector<int32_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int32_t temp = GetS32Param();
        param.push_back(temp);
    }
    cout << "Int32_t vector param length is: " << param.size() << endl;
    return param;
}

vector<int64_t> GetS64VectorParam()
{
    vector<int64_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int64_t temp = GetS64Param();
        param.push_back(temp);
    }
    cout << "Int32_t vector param length is: " << param.size() << endl;
    return param;
}

std::shared_ptr<OHOS::AppExecFwk::Ability> GetParamAbility()
{
    return make_shared<OHOS::AppExecFwk::Ability>();
}

OHOS::AppExecFwk::Want GetParamWant()
{
    return OHOS::AppExecFwk::Want();
}

OHOS::AppExecFwk::FormInfo GetParamFormInfo()
{
    return OHOS::AppExecFwk::FormInfo();
}

OHOS::AppExecFwk::FormJsInfo GetParamFormJsInfo()
{
    return OHOS::AppExecFwk::FormJsInfo();
}

std::shared_ptr<OHOS::AppExecFwk::Ability::FormCallback> GetParamFormCallback()
{
    return shared_ptr<OHOS::AppExecFwk::Ability::FormCallback>();
}

OHOS::AppExecFwk::FormProviderData GetParamFormProviderData()
{
    return OHOS::AppExecFwk::FormProviderData();
}

std::vector<OHOS::AppExecFwk::FormInfo> GetmFormInfoVectorParam()
{
    vector<OHOS::AppExecFwk::FormInfo> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        OHOS::AppExecFwk::FormInfo t = GetParamFormInfo();
        param.push_back(t);
    }
    cout << "FormInfo vector param length is: " << param.size() << endl;
    return param;
}
}  // namespace AppExecFwk
}  // namespace OHOS
