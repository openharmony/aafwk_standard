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

#include "string_ex.h"

#include "mission_information.h"

namespace OHOS {
namespace AppExecFwk {
bool MissionInformation::ReadFromParcel(Parcel &parcel)
{
    label = Str16ToStr8(parcel.ReadString16());
    iconPath = Str16ToStr8(parcel.ReadString16());

    return true;
}
bool MissionInformation::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(label));
    parcel.WriteString16(Str8ToStr16(iconPath));

    return true;
}
MissionInformation *MissionInformation::Unmarshalling(Parcel &parcel)
{
    MissionInformation *info = new (std::nothrow) MissionInformation();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}
}  // namespace AppExecFwk
}  // namespace OHOS