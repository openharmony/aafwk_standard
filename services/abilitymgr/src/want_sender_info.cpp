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

#include "want_sender_info.h"

#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool WantSenderInfo::ReadFromParcel(Parcel &parcel)
{
    type = parcel.ReadInt32();
    bundleName = Str16ToStr8(parcel.ReadString16());
    resultWho = Str16ToStr8(parcel.ReadString16());
    requestCode = parcel.ReadInt32();
    int32_t wantsInfoSize = parcel.ReadInt32();
    for (int32_t i = 0; i < wantsInfoSize; i++) {
        std::unique_ptr<WantsInfo> wantsInfo(parcel.ReadParcelable<WantsInfo>());
        if (!wantsInfo) {
            HILOG_ERROR("ReadParcelable<WantsInfo> failed");
            return false;
        }
        allWants.emplace_back(*wantsInfo);
    }
    flags = parcel.ReadInt32();
    userId = parcel.ReadInt32();
    return true;
}

WantSenderInfo *WantSenderInfo::Unmarshalling(Parcel &parcel)
{
    WantSenderInfo *info = new (std::nothrow) WantSenderInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool WantSenderInfo::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(type);
    parcel.WriteString16(Str8ToStr16(bundleName));
    parcel.WriteString16(Str8ToStr16(resultWho));
    parcel.WriteInt32(requestCode);
    size_t wantsInfoSize = allWants.size();
    if (!parcel.WriteInt32(wantsInfoSize)) {
        return false;
    }
    for (size_t i = 0; i < wantsInfoSize; i++) {
        if (!parcel.WriteParcelable(&allWants[i])) {
            return false;
        }
    }
    parcel.WriteInt32(flags);
    parcel.WriteInt32(userId);
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS