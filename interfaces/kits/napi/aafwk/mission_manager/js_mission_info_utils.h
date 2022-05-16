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

#ifndef OHOS_ABILITY_RUNTIME_JS_MISSION_INFO_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_MISSION_INFO_UTILS_H

#include "js_runtime_utils.h"
#include "mission_info.h"
#include "mission_snapshot.h"
#include "ability_running_info.h"
#include "extension_running_info.h"
#include "running_process_info.h"
#include "native_engine/native_engine.h"
#include "want_params_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsMissionInfo(NativeEngine &engine, const AAFwk::MissionInfo &missionInfo);
NativeValue* CreateJsWant(NativeEngine &engine, const AAFwk::Want &want);
NativeValue* CreateJsWantParams(NativeEngine &engine, const AAFwk::WantParams &wantParams);
NativeValue* CreateJsMissionInfoArray(NativeEngine &engine, const std::vector<AAFwk::MissionInfo> &missionInfos);

template<class TBase, class T, class NativeT>
bool InnerWrapJsWantParams(
    NativeEngine &engine, NativeObject* object, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    TBase *ao = TBase::Query(value);
    if (ao != nullptr) {
        NativeT natValue = T::Unbox(ao);
        object->SetProperty(key.c_str(), CreateJsValue(engine, natValue));
        return true;
    }
    return false;
}

bool InnerWrapJsWantParamsWantParams(
    NativeEngine &engine, NativeObject* object, const std::string &key, const AAFwk::WantParams &wantParams);

bool WrapJsWantParamsArray(
    NativeEngine &engine, NativeObject* object, const std::string &key, sptr<AAFwk::IArray> &ao);

template<class TBase, class T, class NativeT>
bool InnerWrapWantParamsArray(
    NativeEngine &engine, NativeObject* object, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }
    std::vector<NativeT> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            TBase *iValue = TBase::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(T::Unbox(iValue));
            }
        }
    }
    object->SetProperty(key.c_str(), CreateNativeArray(engine, natArray));
    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_MISSION_INFO_UTILS_H