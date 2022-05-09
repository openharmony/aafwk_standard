/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_WINDOW_TRANSITION_INFO_H
#define OHOS_AAFWK_WINDOW_TRANSITION_INFO_H

#ifdef SUPPORT_GRAPHICS
#include <typeinfo>

#include "iremote_object.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
struct AbilityTransitionInfo : public Parcelable {
    std::string bundleName_;
    std::string abilityName_;
    uint32_t mode_ = 1;
    sptr<IRemoteObject> abilityToken_ = nullptr;
    uint64_t displayId_ = 0;
    bool isShowWhenLocked_ = false;
    bool isRecent_ = false;

    virtual bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteString(bundleName_)) {
            return false;
        }

        if (!parcel.WriteString(abilityName_)) {
            return false;
        }

        if (!parcel.WriteUint32(mode_)) {
            return false;
        }

        if (!abilityToken_) {
            if (!parcel.WriteBool(false)) {
                return false;
            }
        } else {
            if (!parcel.WriteBool(true)) {
                return false;
            }
            if (!parcel.WriteObject(abilityToken_)) {
                return false;
            }
        }

        if (!parcel.WriteUint64(displayId_)) {
            return false;
        }

        if (!parcel.WriteBool(isShowWhenLocked_)) {
            return false;
        }

        if (!parcel.WriteBool(isRecent_)) {
            return false;
        }

        return true;
    }

    static AbilityTransitionInfo* Unmarshalling(Parcel& parcel)
    {
        AbilityTransitionInfo* info = new AbilityTransitionInfo();
        info->bundleName_ = parcel.ReadString();
        info->abilityName_ = parcel.ReadString();
        info->mode_ = parcel.ReadUint32();
        if (parcel.ReadBool()) {
            info->abilityToken_ = parcel.ReadObject<IRemoteObject>();
        }
        info->displayId_ = parcel.ReadUint64();
        info->isShowWhenLocked_ = parcel.ReadBool();
        info->isRecent_ = parcel.ReadBool();
        return info;
    }
};
} // namespace AAFwk
} // namespace OHOS
#endif
#endif // OHOS_AAFWK_WINDOW_TRANSITION_INFO_H