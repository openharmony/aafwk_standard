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

#include "sender_info.h"

#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"

#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {
bool SenderInfo::ReadFromParcel(Parcel &parcel)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    code = parcel.ReadInt32();
    std::unique_ptr<Want> wantResquest(parcel.ReadParcelable<Want>());
    if (wantResquest == nullptr) {
        HILOG_ERROR("%{public}s:wantResquest is nullptr.", __func__);
        return false;
    }
    want = *wantResquest;
    resolvedType = Str16ToStr8(parcel.ReadString16());
    sptr<IRemoteObject> finishedReceiverResquest = parcel.ReadParcelable<IRemoteObject>();
    if (finishedReceiverResquest != nullptr) {
        finishedReceiver = iface_cast<IWantReceiver>(finishedReceiverResquest);
        if (!finishedReceiver) {
            return false;
        }
    } else {
        HILOG_ERROR("%{public}s:finishedReceiverResquest is nullptr.", __func__);
    }

    requiredPermission = Str16ToStr8(parcel.ReadString16());
    return true;
}

SenderInfo *SenderInfo::Unmarshalling(Parcel &parcel)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    SenderInfo *info = new (std::nothrow) SenderInfo();
    if (info == nullptr) {
        HILOG_ERROR("%{public}s:senderInfo is nullptr.", __func__);
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        HILOG_ERROR("%{public}s:ReadFromParcel failed.", __func__);
        delete info;
        info = nullptr;
    }
    return info;
}

bool SenderInfo::Marshalling(Parcel &parcel) const
{
    HILOG_INFO("%{public}s:begin.", __func__);

    parcel.WriteInt32(code);
    parcel.WriteParcelable(&want);
    parcel.WriteString16(Str8ToStr16(resolvedType));
    if (finishedReceiver != nullptr) {
        parcel.WriteParcelable(finishedReceiver->AsObject());
    } else {
        parcel.WriteParcelable(nullptr);
    }
    parcel.WriteString16(Str8ToStr16(requiredPermission));
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS