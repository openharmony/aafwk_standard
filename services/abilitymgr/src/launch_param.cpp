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

#include "launch_param.h"

namespace OHOS {
namespace AAFwk {
bool LaunchParam::ReadFromParcel(Parcel &parcel)
{
    int32_t reason = 0;
    if (!parcel.ReadInt32(reason)) {
        return false;
    }
    launchReason = static_cast<LaunchReason>(reason);

    if (!parcel.ReadInt32(reason)) {
        return false;
    }
    lastExitReason = static_cast<LastExitReason>(reason);
    return true;
}

LaunchParam *LaunchParam::Unmarshalling(Parcel &parcel)
{
    LaunchParam *param = new (std::nothrow) LaunchParam();
    if (param == nullptr) {
        return nullptr;
    }

    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool LaunchParam::Marshalling(Parcel &parcel) const
{
    // write launchReason
    if (!parcel.WriteInt32(static_cast<int32_t>(launchReason))) {
        return false;
    }
    // write lastExitReason
    if (!parcel.WriteInt32(static_cast<int32_t>(lastExitReason))) {
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS