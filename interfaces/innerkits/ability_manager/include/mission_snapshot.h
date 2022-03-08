/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_INTERFACES_INNERKITS_MISSION_SNAPSHOT_H
#define OHOS_AAFWK_INTERFACES_INNERKITS_MISSION_SNAPSHOT_H

#include <string>
#include "image_info.h"
#include "parcel.h"
#include "element_name.h"
#ifdef SUPPORT_GRAPHICS
#include "foundation/multimedia/image_standard/interfaces/innerkits/include/pixel_map.h"
#endif

namespace OHOS {
namespace AAFwk {
/**
 * @struct MissionPixelMap
 * MissionPixelMap is used to save information about mission sanpshot.
 */
struct MissionPixelMap : public Parcelable {
    AppExecFwk::ElementName topAbility;
    AAFwk::ImageInfo imageInfo;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static MissionPixelMap *Unmarshalling(Parcel &parcel);
};

struct MissionSnapshot : public Parcelable {
    AppExecFwk::ElementName topAbility;
#ifdef SUPPORT_GRAPHICS
    std::shared_ptr<Media::PixelMap> snapshot;
#endif
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static MissionSnapshot *Unmarshalling(Parcel &parcel);
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_INTERFACES_INNERKITS_MISSION_SNAPSHOT_H
