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

#include "napi_common_want.h"

#include "hilog_wrapper.h"
#include "napi_common_util.h"
#include "ohos/aafwk/base/array_wrapper.h"
#include "ohos/aafwk/base/bool_wrapper.h"
#include "ohos/aafwk/base/byte_wrapper.h"
#include "ohos/aafwk/base/double_wrapper.h"
#include "ohos/aafwk/base/float_wrapper.h"
#include "ohos/aafwk/base/int_wrapper.h"
#include "ohos/aafwk/base/long_wrapper.h"
#include "ohos/aafwk/base/short_wrapper.h"
#include "ohos/aafwk/base/string_wrapper.h"
#include "ohos/aafwk/base/zchar_wrapper.h"
#include "ohos/aafwk/content/want_params_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
EXTERN_C_START
/**
 * @brief Init param of wantOptions.
 *
 * @param flagMap Indicates flag of list in Want .
 */
void InnerInitWantOptionsData(std::map<std::string, unsigned int> &flagMap)
{
    flagMap.emplace("authReadUriPermission", Want::FLAG_AUTH_READ_URI_PERMISSION);
    flagMap.emplace("authWriteUriPermission", Want::FLAG_AUTH_WRITE_URI_PERMISSION);
    flagMap.emplace("abilityForwardResult", Want::FLAG_ABILITY_FORWARD_RESULT);
    flagMap.emplace("abilityContinuation", Want::FLAG_ABILITY_CONTINUATION);
    flagMap.emplace("notOhosComponent", Want::FLAG_NOT_OHOS_COMPONENT);
    flagMap.emplace("abilityFormEnabled", Want::FLAG_ABILITY_FORM_ENABLED);
    flagMap.emplace("authPersistableUriPermission", Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION);
    flagMap.emplace("authPrefixUriPermission", Want::FLAG_AUTH_PREFIX_URI_PERMISSION);
    flagMap.emplace("abilitySliceMultiDevice", Want::FLAG_ABILITYSLICE_MULTI_DEVICE);
    flagMap.emplace("startForegroundAbility", Want::FLAG_START_FOREGROUND_ABILITY);
    flagMap.emplace("installOnDemand", Want::FLAG_INSTALL_ON_DEMAND);
    flagMap.emplace("abilitySliceForwardResult", Want::FLAG_ABILITYSLICE_FORWARD_RESULT);
    flagMap.emplace("installWithBackgroundMode", Want::FLAG_INSTALL_WITH_BACKGROUND_MODE);
}

napi_value WrapElementName(napi_env env, const ElementName &elementName)
{
    HILOG_INFO("%{public}s called.", __func__);

    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    napi_value jsValue = nullptr;
    HILOG_INFO("%{public}s called. deviceID=%{public}s", __func__, elementName.GetDeviceID().c_str());
    NAPI_CALL(env, napi_create_string_utf8(env, elementName.GetDeviceID().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "deviceId", jsValue));

    jsValue = nullptr;
    HILOG_INFO("%{public}s called. GetBundleName=%{public}s", __func__, elementName.GetBundleName().c_str());
    NAPI_CALL(env, napi_create_string_utf8(env, elementName.GetBundleName().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "bundleName", jsValue));

    jsValue = nullptr;
    HILOG_INFO("%{public}s called. GetAbilityName=%{public}s", __func__, elementName.GetAbilityName().c_str());
    NAPI_CALL(env, napi_create_string_utf8(env, elementName.GetAbilityName().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "abilityName", jsValue));

    return jsObject;
}

bool UnwrapElementName(napi_env env, napi_value param, ElementName &elementName)
{
    HILOG_INFO("%{public}s called.", __func__);

    std::string natValue("");
    if (UnwrapStringByPropertyName(env, param, "deviceId", natValue)) {
        elementName.SetDeviceID(natValue);
    }

    natValue = "";
    if (UnwrapStringByPropertyName(env, param, "bundleName", natValue)) {
        elementName.SetBundleName(natValue);
    }

    natValue = "";
    if (UnwrapStringByPropertyName(env, param, "abilityName", natValue)) {
        elementName.SetAbilityName(natValue);
    }
    return true;
}
bool InnerWrapWantParamsChar(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IChar *ao = AAFwk::IChar::Query(value);
    if (ao != nullptr) {
        std::string natValue(static_cast<Char *>(ao)->ToString());
        HILOG_INFO("%{public}s called. key=%{public}s, natValue=%{public}s", __func__, key.c_str(), natValue.c_str());
        napi_value jsValue = WrapStringToJS(env, natValue);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

bool InnerWrapWantParamsString(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IString *ao = AAFwk::IString::Query(value);
    if (ao != nullptr) {
        std::string natValue = AAFwk::String::Unbox(ao);
        HILOG_INFO("%{public}s called. key=%{public}s, natValue=%{public}s", __func__, key.c_str(), natValue.c_str());
        napi_value jsValue = WrapStringToJS(env, natValue);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

bool InnerWrapWantParamsBool(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IBoolean *bo = AAFwk::IBoolean::Query(value);
    if (bo != nullptr) {
        bool natValue = AAFwk::Boolean::Unbox(bo);
        napi_value jsValue = WrapBoolToJS(env, natValue);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

bool InnerWrapWantParamsByte(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IByte *bo = AAFwk::IByte::Query(value);
    if (bo != nullptr) {
        int intValue = AAFwk::Byte::Unbox(bo);
        napi_value jsValue = WrapInt32ToJS(env, intValue);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

bool InnerWrapWantParamsShort(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IShort *ao = AAFwk::IShort::Query(value);
    if (ao != nullptr) {
        short natValue = AAFwk::Short::Unbox(ao);
        napi_value jsValue = WrapInt32ToJS(env, natValue);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

bool InnerWrapWantParamsInt32(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IInteger *ao = AAFwk::IInteger::Query(value);
    if (ao != nullptr) {
        int natValue = AAFwk::Integer::Unbox(ao);
        HILOG_INFO("%{public}s called. key=%{public}s, natValue=%{public}d", __func__, key.c_str(), natValue);
        napi_value jsValue = WrapInt32ToJS(env, natValue);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

bool InnerWrapWantParamsInt64(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::ILong *ao = AAFwk::ILong::Query(value);
    if (ao != nullptr) {
        int64_t natValue = AAFwk::Long::Unbox(ao);
        napi_value jsValue = WrapInt64ToJS(env, natValue);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

bool InnerWrapWantParamsFloat(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IFloat *ao = AAFwk::IFloat::Query(value);
    if (ao != nullptr) {
        float natValue = AAFwk::Float::Unbox(ao);
        napi_value jsValue = WrapDoubleToJS(env, natValue);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

bool InnerWrapWantParamsDouble(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IDouble *ao = AAFwk::IDouble::Query(value);
    if (ao != nullptr) {
        double natValue = AAFwk::Double::Unbox(ao);
        napi_value jsValue = WrapDoubleToJS(env, natValue);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

bool InnerWrapWantParamsWantParams(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    HILOG_INFO("%{public}s called. key=%{public}s", __func__, key.c_str());
    auto value = wantParams.GetParam(key);
    AAFwk::IWantParams *o = AAFwk::IWantParams::Query(value);
    if (o != nullptr) {
        AAFwk::WantParams wp = AAFwk::WantParamWrapper::Unbox(o);
        napi_value jsValue = WrapWantParams(env, wp);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}
bool InnerWrapWantParamsArrayChar(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    HILOG_INFO("%{public}s called.", __func__);
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<std::string> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IChar *iValue = AAFwk::IChar::Query(iface);
            if (iValue != nullptr) {
                std::string str(static_cast<Char *>(iValue)->ToString());
                HILOG_INFO("%{public}s called. str=%{public}s", __func__, str.c_str());
                natArray.push_back(str);
            }
        }
    }

    napi_value jsValue = WrapArrayStringToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayString(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<std::string> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IString *iValue = AAFwk::IString::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::String::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayStringToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayBool(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<bool> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IBoolean *iValue = AAFwk::IBoolean::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Boolean::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayBoolToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayShort(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IShort *iValue = AAFwk::IShort::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Short::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayInt32ToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}
bool InnerWrapWantParamsArrayByte(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IByte *iValue = AAFwk::IByte::Query(iface);
            if (iValue != nullptr) {
                int intValue = AAFwk::Byte::Unbox(iValue);
                natArray.push_back(intValue);
            }
        }
    }

    napi_value jsValue = WrapArrayInt32ToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayInt32(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IInteger *iValue = AAFwk::IInteger::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Integer::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayInt32ToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayInt64(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int64_t> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::ILong *iValue = AAFwk::ILong::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Long::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayInt64ToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayFloat(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<double> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IFloat *iValue = AAFwk::IFloat::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Float::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayDoubleToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayDouble(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<double> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IDouble *iValue = AAFwk::IDouble::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Double::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayDoubleToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArray(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    HILOG_INFO("%{public}s called. key=%{public}s", __func__, key.c_str());
    if (AAFwk::Array::IsStringArray(ao)) {
        return InnerWrapWantParamsArrayString(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsBooleanArray(ao)) {
        return InnerWrapWantParamsArrayBool(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsShortArray(ao)) {
        return InnerWrapWantParamsArrayShort(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsIntegerArray(ao)) {
        return InnerWrapWantParamsArrayInt32(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsLongArray(ao)) {
        return InnerWrapWantParamsArrayInt64(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsFloatArray(ao)) {
        return InnerWrapWantParamsArrayFloat(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsByteArray(ao)) {
        return InnerWrapWantParamsArrayByte(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsCharArray(ao)) {
        return InnerWrapWantParamsArrayChar(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsDoubleArray(ao)) {
        return InnerWrapWantParamsArrayDouble(env, jsObject, key, ao);
    } else {
        return false;
    }
}

napi_value WrapWantParams(napi_env env, const AAFwk::WantParams &wantParams)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    napi_value jsValue = nullptr;
    const std::map<std::string, sptr<AAFwk::IInterface>> paramList = wantParams.GetParams();
    for (auto iter = paramList.begin(); iter != paramList.end(); iter++) {
        jsValue = nullptr;
        if (AAFwk::IString::Query(iter->second) != nullptr) {
            InnerWrapWantParamsString(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IBoolean::Query(iter->second) != nullptr) {
            InnerWrapWantParamsBool(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IShort::Query(iter->second) != nullptr) {
            InnerWrapWantParamsShort(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IInteger::Query(iter->second) != nullptr) {
            InnerWrapWantParamsInt32(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::ILong::Query(iter->second) != nullptr) {
            InnerWrapWantParamsInt64(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IFloat::Query(iter->second) != nullptr) {
            InnerWrapWantParamsFloat(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IDouble::Query(iter->second) != nullptr) {
            InnerWrapWantParamsDouble(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IChar::Query(iter->second) != nullptr) {
            InnerWrapWantParamsChar(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IByte::Query(iter->second) != nullptr) {
            InnerWrapWantParamsByte(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IArray::Query(iter->second) != nullptr) {
            AAFwk::IArray *ao = AAFwk::IArray::Query(iter->second);
            if (ao != nullptr) {
                sptr<AAFwk::IArray> array(ao);
                InnerWrapWantParamsArray(env, jsObject, iter->first, array);
            }
        } else if (AAFwk::IWantParams::Query(iter->second) != nullptr) {
            InnerWrapWantParamsWantParams(env, jsObject, iter->first, wantParams);
        }
    }
    return jsObject;
}

bool InnerSetWantParamsArrayString(
    const std::string &key, const std::vector<std::string> &value, AAFwk::WantParams &wantParams)
{
    long size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IString);
    if (ao != nullptr) {
        for (long i = 0; i < size; i++) {
            ao->Set(i, AAFwk::String::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayInt(const std::string &key, const std::vector<int> &value, AAFwk::WantParams &wantParams)
{
    long size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IInteger);
    if (ao != nullptr) {
        for (long i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Integer::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayLong(const std::string &key, const std::vector<long> &value, AAFwk::WantParams &wantParams)
{
    long size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_ILong);
    if (ao != nullptr) {
        for (long i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Long::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayBool(const std::string &key, const std::vector<bool> &value, AAFwk::WantParams &wantParams)
{
    long size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IBoolean);
    if (ao != nullptr) {
        for (long i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Boolean::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayDouble(
    const std::string &key, const std::vector<double> &value, AAFwk::WantParams &wantParams)
{
    long size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IDouble);
    if (ao != nullptr) {
        for (long i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Double::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerUnwrapWantParamsArray(napi_env env, const std::string &key, napi_value param, AAFwk::WantParams &wantParams)
{
    HILOG_INFO("%{public}s called.", __func__);

    ComplexArrayData natArrayValue;
    if (!UnwrapArrayComplexFromJS(env, param, natArrayValue)) {
        return false;
    }

    if (natArrayValue.stringList.size() > 0) {
        return InnerSetWantParamsArrayString(key, natArrayValue.stringList, wantParams);
    }
    if (natArrayValue.intList.size() > 0) {
        return InnerSetWantParamsArrayInt(key, natArrayValue.intList, wantParams);
    }
    if (natArrayValue.longList.size() > 0) {
        return InnerSetWantParamsArrayLong(key, natArrayValue.longList, wantParams);
    }
    if (natArrayValue.boolList.size() > 0) {
        return InnerSetWantParamsArrayBool(key, natArrayValue.boolList, wantParams);
    }
    if (natArrayValue.doubleList.size() > 0) {
        return InnerSetWantParamsArrayDouble(key, natArrayValue.doubleList, wantParams);
    }

    return false;
}

bool InnerUnwrapWantParams(napi_env env, const std::string &key, napi_value param, AAFwk::WantParams &wantParams)
{
    HILOG_INFO("%{public}s called.", __func__);
    AAFwk::WantParams wp;

    if (UnwrapWantParams(env, param, wp)) {
        sptr<AAFwk::IWantParams> pWantParams = AAFwk::WantParamWrapper::Box(wp);
        if (pWantParams != nullptr) {
            wantParams.SetParam(key, pWantParams);
            return true;
        }
    }
    return false;
}

bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        return false;
    }

    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL_BASE(env, napi_get_property_names(env, param, &jsProNameList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsProNameList, &jsProCount), false);
    HILOG_INFO("%{public}s called. Property size=%{public}d.", __func__, jsProCount);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        NAPI_CALL_BASE(env, napi_get_element(env, jsProNameList, index, &jsProName), false);

        std::string strProName = UnwrapStringFromJS(env, jsProName);
        /* skip reserved param */
        if (strProName == Want::PARAM_RESV_WINDOW_MODE) {
            continue;
        }
        HILOG_INFO("%{public}s called. Property name=%{public}s.", __func__, strProName.c_str());
        NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsProValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);

        switch (jsValueType) {
            case napi_string: {
                std::string natValue = UnwrapStringFromJS(env, jsProValue);
                HILOG_INFO("%{public}s called. Property value=%{public}s.", __func__, natValue.c_str());
                wantParams.SetParam(strProName, AAFwk::String::Box(natValue));
                break;
            }
            case napi_boolean: {
                bool natValue = false;
                NAPI_CALL_BASE(env, napi_get_value_bool(env, jsProValue, &natValue), false);
                HILOG_INFO("%{public}s called. Property value=%{public}s.", __func__, natValue ? "true" : "false");
                wantParams.SetParam(strProName, AAFwk::Boolean::Box(natValue));
                break;
            }
            case napi_number: {
                int32_t natValue32 = 0;
                double natValueDouble = 0.0;
                bool isReadValue32 = false;
                bool isReadDouble = false;
                if (napi_get_value_int32(env, jsProValue, &natValue32) == napi_ok) {
                    HILOG_INFO("%{public}s called. Property value=%{public}d.", __func__, natValue32);
                    isReadValue32 = true;
                }

                if (napi_get_value_double(env, jsProValue, &natValueDouble) == napi_ok) {
                    HILOG_INFO("%{public}s called. Property value=%{public}lf.", __func__, natValueDouble);
                    isReadDouble = true;
                }

                if (isReadValue32 && isReadDouble) {
                    if (abs(natValueDouble - natValue32 * 1.0) > 0.0) {
                        wantParams.SetParam(strProName, AAFwk::Double::Box(natValueDouble));
                    } else {
                        wantParams.SetParam(strProName, AAFwk::Integer::Box(natValue32));
                    }
                } else if (isReadValue32) {
                    wantParams.SetParam(strProName, AAFwk::Integer::Box(natValue32));
                } else if (isReadDouble) {
                    wantParams.SetParam(strProName, AAFwk::Double::Box(natValueDouble));
                }
                break;
            }
            case napi_object: {
                bool isArray = false;
                if (napi_is_array(env, jsProValue, &isArray) == napi_ok) {
                    if (isArray) {
                        HILOG_INFO("%{public}s called. %{public}s is array.", __func__, strProName.c_str());
                        InnerUnwrapWantParamsArray(env, strProName, jsProValue, wantParams);
                    } else {
                        HILOG_INFO("%{public}s called. %{public}s is wantparams.", __func__, strProName.c_str());
                        InnerUnwrapWantParams(env, strProName, jsProValue, wantParams);
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return true;
}

napi_value InnerWrapWantOptions(napi_env env, const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value jsObject = nullptr;
    std::map<std::string, unsigned int> flagMap;
    InnerInitWantOptionsData(flagMap);
    unsigned int flags = want.GetFlags();
    bool natValue = false;
    napi_value jsValue = nullptr;

    NAPI_CALL(env, napi_create_object(env, &jsObject));
    for (auto iter = flagMap.begin(); iter != flagMap.end(); iter++) {
        jsValue = nullptr;
        natValue = ((flags & iter->second) == iter->second);
        if (napi_get_boolean(env, natValue, &jsValue) == napi_ok) {
            SetPropertyValueByPropertyName(env, jsObject, iter->first.c_str(), jsValue);
        }
    }

    return jsObject;
}

bool InnerUnwrapWantOptions(napi_env env, napi_value param, const char *propertyName, Want &want)
{
    napi_value jsValue = GetPropertyValueByPropertyName(env, param, propertyName, napi_object);
    if (jsValue == nullptr) {
        return false;
    }

    bool natValue = false;
    unsigned int flags = 0;
    std::map<std::string, unsigned int> flagMap;
    InnerInitWantOptionsData(flagMap);
    for (auto iter = flagMap.begin(); iter != flagMap.end(); iter++) {
        natValue = false;
        if (UnwrapBooleanByPropertyName(env, jsValue, iter->first.c_str(), natValue)) {
            if (natValue) {
                flags |= iter->second;
            }
        }
    }

    want.SetFlags(flags);
    return true;
}

napi_value WrapWant(napi_env env, const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;

    NAPI_CALL(env, napi_create_object(env, &jsObject));

    napi_value jsElementName = WrapElementName(env, want.GetElement());
    if (jsElementName == nullptr) {
        HILOG_INFO("%{public}s called. Invoke WrapElementName failed.", __func__);
        return nullptr;
    }

    jsValue = GetPropertyValueByPropertyName(env, jsElementName, "deviceId", napi_string);
    SetPropertyValueByPropertyName(env, jsObject, "deviceId", jsValue);

    jsValue = nullptr;
    jsValue = GetPropertyValueByPropertyName(env, jsElementName, "bundleName", napi_string);
    SetPropertyValueByPropertyName(env, jsObject, "bundleName", jsValue);

    jsValue = nullptr;
    jsValue = GetPropertyValueByPropertyName(env, jsElementName, "abilityName", napi_string);
    SetPropertyValueByPropertyName(env, jsObject, "abilityName", jsValue);

    jsValue = nullptr;
    jsValue = WrapStringToJS(env, want.GetUriString());
    SetPropertyValueByPropertyName(env, jsObject, "uri", jsValue);

    jsValue = nullptr;
    jsValue = WrapStringToJS(env, want.GetType());
    SetPropertyValueByPropertyName(env, jsObject, "type", jsValue);

    jsValue = nullptr;
    jsValue = WrapInt32ToJS(env, want.GetFlags());
    SetPropertyValueByPropertyName(env, jsObject, "flags", jsValue);

    jsValue = nullptr;
    jsValue = WrapStringToJS(env, want.GetAction());
    SetPropertyValueByPropertyName(env, jsObject, "action", jsValue);

    jsValue = nullptr;
    jsValue = WrapWantParams(env, want.GetParams());
    SetPropertyValueByPropertyName(env, jsObject, "parameters", jsValue);

    jsValue = nullptr;
    jsValue = WrapArrayStringToJS(env, want.GetEntities());
    SetPropertyValueByPropertyName(env, jsObject, "entities", jsValue);

    return jsObject;
}

bool UnwrapWant(napi_env env, napi_value param, Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        HILOG_INFO("%{public}s called. Params is invalid.", __func__);
        return false;
    }

    napi_value jsValue = GetPropertyValueByPropertyName(env, param, "parameters", napi_object);
    if (jsValue != nullptr) {
        AAFwk::WantParams wantParams;
        if (UnwrapWantParams(env, jsValue, wantParams)) {
            want.SetParams(wantParams);
        }
    }

    std::string natValueString("");
    if (UnwrapStringByPropertyName(env, param, "action", natValueString)) {
        want.SetAction(natValueString);
    }

    std::vector<std::string> natValueStringList;
    if (UnwrapStringArrayByPropertyName(env, param, "entities", natValueStringList)) {
        for (size_t i = 0; i < natValueStringList.size(); i++) {
            want.AddEntity(natValueStringList[i]);
        }
    }

    natValueString = "";
    if (UnwrapStringByPropertyName(env, param, "uri", natValueString)) {
        want.SetUri(natValueString);
    }

    int32_t flags = 0;
    if (UnwrapInt32ByPropertyName(env, param, "flags", flags)) {
        want.SetFlags(flags);
    }

    ElementName natElementName;
    UnwrapElementName(env, param, natElementName);
    want.SetElementName(natElementName.GetDeviceID(), natElementName.GetBundleName(), natElementName.GetAbilityName());

    natValueString = "";
    if (UnwrapStringByPropertyName(env, param, "type", natValueString)) {
        want.SetType(natValueString);
    }

    return true;
}
EXTERN_C_END
}  // namespace AppExecFwk
}  // namespace OHOS
