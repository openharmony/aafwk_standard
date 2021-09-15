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

#include "stack_setting.h"

#include "hilog_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool StackSetting::ReadFromParcel(Parcel &parcel)
{
    userId = parcel.ReadInt32();
    stackId = static_cast<STACK_ID>(parcel.ReadInt32());
    maxHoldMission = parcel.ReadInt32();
    isSyncVisual = parcel.ReadBool();
    return true;
}

StackSetting *StackSetting::Unmarshalling(Parcel &parcel)
{
    StackSetting *setiing = new (std::nothrow) StackSetting();
    if (setiing == nullptr) {
        return nullptr;
    }

    if (!setiing->ReadFromParcel(parcel)) {
        delete setiing;
        setiing = nullptr;
    }
    return setiing;
}

bool StackSetting::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(userId);
    parcel.WriteInt32(static_cast<int32_t>(stackId));
    parcel.WriteInt32(maxHoldMission);
    parcel.WriteBool(isSyncVisual);
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS