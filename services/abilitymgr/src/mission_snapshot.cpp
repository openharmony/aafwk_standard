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

#include "mission_snapshot.h"

#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool MissionSnapshot::ReadFromParcel(Parcel &parcel)
{
    auto elementName = parcel.ReadParcelable<AppExecFwk::ElementName>();
    if (elementName == nullptr) {
        return false;
    }
    ability = *elementName;
    auto pixelMap = parcel.ReadParcelable<Media::PixelMap>();
    if (pixelMap == nullptr) {
        return false;
    }
    snapshot = *pixelMap;
    return true;
}

MissionSnapshot *MissionSnapshot::Unmarshalling(Parcel &parcel)
{
    MissionSnapshot *info = new (std::nothrow) MissionSnapshot();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool MissionSnapshot::Marshalling(Parcel &parcel) const
{
    parcel.WriteParcelable(&ability);
    parcel.WriteParcelable(&snapshot);
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS