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

#include "js_mission_info_utils.h"

#include "hilog_wrapper.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "bool_wrapper.h"
#include "byte_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "zchar_wrapper.h"
#include "array_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsMissionInfo(NativeEngine &engine, const AAFwk::MissionInfo &missionInfo)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("missionId", CreateJsValue(engine, missionInfo.id));
    object->SetProperty("runningState", CreateJsValue(engine, missionInfo.runningState));
    object->SetProperty("lockedState", CreateJsValue(engine, missionInfo.lockedState));
    object->SetProperty("continuable", CreateJsValue(engine, missionInfo.continuable));
    object->SetProperty("timestamp", CreateJsValue(engine, missionInfo.time));
    object->SetProperty("want", CreateJsWant(engine, missionInfo.want));
    object->SetProperty("label", CreateJsValue(engine, missionInfo.label));
    object->SetProperty("iconPath", CreateJsValue(engine, missionInfo.iconPath));
    return objValue;
}

NativeValue* CreateJsWant(NativeEngine &engine, const AAFwk::Want &want)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("deviceId", CreateJsValue(engine, want.GetElement().GetDeviceID()));
    object->SetProperty("bundleName", CreateJsValue(engine, want.GetElement().GetBundleName()));
    object->SetProperty("abilityName", CreateJsValue(engine, want.GetElement().GetAbilityName()));
    object->SetProperty("uri", CreateJsValue(engine, want.GetUriString()));
    object->SetProperty("type", CreateJsValue(engine, want.GetType()));
    object->SetProperty("flags", CreateJsValue(engine, want.GetFlags()));
    object->SetProperty("action", CreateJsValue(engine, want.GetAction()));
    object->SetProperty("parameters", CreateJsWantParams(engine, want.GetParams()));
    object->SetProperty("entities", CreateNativeArray(engine, want.GetEntities()));
    return objValue;
}

NativeValue* CreateJsWantParams(NativeEngine &engine, const AAFwk::WantParams &wantParams)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    const std::map<std::string, sptr<AAFwk::IInterface>> paramList = wantParams.GetParams();
    for (auto iter = paramList.begin(); iter != paramList.end(); iter++) {
        if (AAFwk::IString::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IString, AAFwk::String, std::string>(
                engine, object, iter->first, wantParams);
        } else if (AAFwk::IBoolean::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IBoolean, AAFwk::Boolean, bool>(
                engine, object, iter->first, wantParams);
        } else if (AAFwk::IShort::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IShort, AAFwk::Short, short>(
                engine, object, iter->first, wantParams);
        } else if (AAFwk::IInteger::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IInteger, AAFwk::Integer, int>(
                engine, object, iter->first, wantParams);
        } else if (AAFwk::ILong::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::ILong, AAFwk::Long, int64_t>(
                engine, object, iter->first, wantParams);
        } else if (AAFwk::IFloat::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IFloat, AAFwk::Float, float>(
                engine, object, iter->first, wantParams);
        } else if (AAFwk::IDouble::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IDouble, AAFwk::Double, double>(
                engine, object, iter->first, wantParams);
        } else if (AAFwk::IChar::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IChar, AAFwk::Char, char>(
                engine, object, iter->first, wantParams);
        } else if (AAFwk::IByte::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IByte, AAFwk::Byte, int>(
                engine, object, iter->first, wantParams);
        } else if (AAFwk::IArray::Query(iter->second) != nullptr) {
            AAFwk::IArray *ao = AAFwk::IArray::Query(iter->second);
            if (ao != nullptr) {
                sptr<AAFwk::IArray> array(ao);
                WrapJsWantParamsArray(engine, object, iter->first, array);
            }
        } else if (AAFwk::IWantParams::Query(iter->second) != nullptr) {
            InnerWrapJsWantParamsWantParams(engine, object, iter->first, wantParams);
        }
    }
    return objValue;
}

NativeValue* CreateJsMissionInfoArray(NativeEngine &engine, const std::vector<AAFwk::MissionInfo> &missionInfos)
{
    NativeValue* arrayValue = engine.CreateArray(missionInfos.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto &missionInfo : missionInfos) {
        array->SetElement(index++, CreateJsMissionInfo(engine, missionInfo));
    }
    return arrayValue;
}

bool InnerWrapJsWantParamsWantParams(
    NativeEngine &engine, NativeObject* object, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IWantParams *o = AAFwk::IWantParams::Query(value);
    if (o != nullptr) {
        AAFwk::WantParams wp = AAFwk::WantParamWrapper::Unbox(o);
        object->SetProperty(key.c_str(), CreateJsWantParams(engine, wp));
        return true;
    }
    return false;
}

bool WrapJsWantParamsArray(NativeEngine &engine, NativeObject* object, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    HILOG_INFO("%{public}s called. key=%{public}s", __func__, key.c_str());
    if (AAFwk::Array::IsStringArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IString, AAFwk::String, std::string>(
            engine, object, key, ao);
    } else if (AAFwk::Array::IsBooleanArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IBoolean, AAFwk::Boolean, bool>(
            engine, object, key, ao);
    } else if (AAFwk::Array::IsShortArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IShort, AAFwk::Short, short>(
            engine, object, key, ao);
    } else if (AAFwk::Array::IsIntegerArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IInteger, AAFwk::Integer, int>(
            engine, object, key, ao);
    } else if (AAFwk::Array::IsLongArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::ILong, AAFwk::Long, int64_t>(
            engine, object, key, ao);
    } else if (AAFwk::Array::IsFloatArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IFloat, AAFwk::Float, float>(
            engine, object, key, ao);
    } else if (AAFwk::Array::IsByteArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IByte, AAFwk::Byte, int>(
            engine, object, key, ao);
    } else if (AAFwk::Array::IsCharArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IChar, AAFwk::Char, char>(
            engine, object, key, ao);
    } else if (AAFwk::Array::IsDoubleArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IDouble, AAFwk::Double, double>(
            engine, object, key, ao);
    } else {
        return false;
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS