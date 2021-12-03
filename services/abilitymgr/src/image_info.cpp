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

#include "image_info.h"

#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool ImageInfo::ReadFromParcel(Parcel &parcel)
{
    parcel.ReadUint32(width);
    parcel.ReadUint32(height);
    parcel.ReadUint32(format);
    parcel.ReadUint32(size);
    parcel.ReadInt32(shmKey);
    return true;
}

ImageInfo *ImageInfo::Unmarshalling(Parcel &parcel)
{
    ImageInfo *info = new (std::nothrow) ImageInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool ImageInfo::Marshalling(Parcel &parcel) const
{
    parcel.WriteUint32(width);
    parcel.WriteUint32(height);
    parcel.WriteUint32(format);
    parcel.WriteUint32(size);
    parcel.WriteInt32(shmKey);
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS