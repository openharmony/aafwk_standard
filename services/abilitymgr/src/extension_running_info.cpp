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

#include "extension_running_info.h"
#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool ExtensionRunningInfo::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<AppExecFwk::ElementName> readExtension(parcel.ReadParcelable<AppExecFwk::ElementName>());
    if (readExtension == nullptr) {
        return false;
    }
    extension = *readExtension;
    pid = parcel.ReadInt32();
    uid = parcel.ReadInt32();
    processName = Str16ToStr8(parcel.ReadString16());
    startTime = parcel.ReadInt32();
    int32_t clientPackageSize = parcel.ReadInt32();
    for (int32_t i = 0; i < clientPackageSize; i++) {
        clientPackage.emplace_back(Str16ToStr8(parcel.ReadString16()));
    }
    return true;
}

ExtensionRunningInfo *ExtensionRunningInfo::Unmarshalling(Parcel &parcel)
{
    ExtensionRunningInfo *info = new (std::nothrow) ExtensionRunningInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool ExtensionRunningInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(&extension)) {
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
    if (!parcel.WriteInt32(startTime)) {
        return false;
    }
    int32_t clientPackageSize = static_cast<int32_t>(clientPackage.size());
    if (!parcel.WriteInt32(clientPackageSize)) {
        return false;
    }
    for (std::string package : clientPackage) {
        if (!parcel.WriteString16(Str8ToStr16(package))) {
            return false;
        }
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS