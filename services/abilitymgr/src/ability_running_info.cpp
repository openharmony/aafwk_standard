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

#include "ability_running_info.h"
#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool AbilityRunningInfo::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<AppExecFwk::ElementName> abilityInfo(parcel.ReadParcelable<AppExecFwk::ElementName>());
    if (abilityInfo == nullptr) {
        return false;
    }
    ability = *abilityInfo;
    pid = parcel.ReadInt32();
    uid = parcel.ReadInt32();
    processName = Str16ToStr8(parcel.ReadString16());
    startTime = parcel.ReadInt64();
    abilityState = parcel.ReadInt32();
    return true;
}

AbilityRunningInfo *AbilityRunningInfo::Unmarshalling(Parcel &parcel)
{
    AbilityRunningInfo *info = new (std::nothrow) AbilityRunningInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool AbilityRunningInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(&ability)) {
        return false;
    }
    if (!parcel.WriteInt32(pid)) {
        return false;
    }
    if (!parcel.WriteInt32(uid)) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(processName))) {
        return false;
    }
    if (!parcel.WriteInt64(startTime)) {
        return false;
    }
    if (!parcel.WriteInt32(abilityState)) {
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS