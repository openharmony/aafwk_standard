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

#ifndef OHOS_AAFWK_MISSION_OPTION_H
#define OHOS_AAFWK_MISSION_OPTION_H

#include "ability_window_configuration.h"
#include "parcel.h"

#include <map>

namespace OHOS {
namespace AAFwk {
const int32_t DISPLAY_DEFAULT_ID = 0;
const int32_t MISSION_INVAL_VALUE = -1;
const int32_t DEFAULT_USER_ID = 0;
/**
 * @class MissionOption
 * MissionOption contains option information for mission which one needs to be moved.
 */
class MissionOption final : public Parcelable {
public:
    MissionOption() = default;
    virtual ~MissionOption() = default;

    bool IsSameWindowMode(const AbilityWindowConfiguration &key) const;
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static MissionOption *Unmarshalling(Parcel &parcel);

    int32_t userId = DEFAULT_USER_ID;         // stack 's user id ,default value is 0
    int32_t missionId = MISSION_INVAL_VALUE;  // mission record id
    // multi window mode key , default in an undefined
    AbilityWindowConfiguration winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    int32_t displayKey = DISPLAY_DEFAULT_ID;  // display screen device id

    void AddProperty(const std::string &key, const std::string &value);
    void GetProperty(const std::string &key, std::string &value);

private:
    std::map<std::string, std::string> properties_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_MISSION_OPTION_H