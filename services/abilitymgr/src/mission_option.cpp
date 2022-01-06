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

#include "mission_option.h"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool MissionOption::IsSameWindowMode(const AbilityWindowConfiguration &key) const
{
    if (winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        return (winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY &&
                   key == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) ||
               (key == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY &&
                   winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY);
    } else {
        return (winModeKey == key);
    }
}

bool MissionOption::ReadFromParcel(Parcel &parcel)
{
    userId = parcel.ReadInt32();
    missionId = parcel.ReadInt32();
    winModeKey = static_cast<AbilityWindowConfiguration>(parcel.ReadInt32());
    displayKey = parcel.ReadInt32();

    int32_t size = parcel.ReadInt32();
    for (int32_t i = 0; i < size; i++) {
        std::string key = Str16ToStr8(parcel.ReadString16());
        std::string data = Str16ToStr8(parcel.ReadString16());
        properties_[key] = data;
    }

    return true;
}

MissionOption *MissionOption::Unmarshalling(Parcel &parcel)
{
    MissionOption *option = new (std::nothrow) MissionOption();
    if (option == nullptr) {
        return nullptr;
    }

    if (!option->ReadFromParcel(parcel)) {
        delete option;
        option = nullptr;
    }

    return option;
}

bool MissionOption::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(userId);
    parcel.WriteInt32(missionId);
    parcel.WriteInt32(static_cast<int32_t>(winModeKey));
    parcel.WriteInt32(displayKey);

    parcel.WriteInt32(properties_.size());
    for (auto pair : properties_) {
        parcel.WriteString16(Str8ToStr16(pair.first));
        parcel.WriteString16(Str8ToStr16(pair.second));
    }

    return true;
}

void MissionOption::AddProperty(const std::string &key, const std::string &value)
{
    properties_[key] = value;
}

void MissionOption::GetProperty(const std::string &key, std::string &value)
{
    auto it = properties_.find(key);
    if (it == properties_.end()) {
        value = std::string();
        return;
    }
    value = properties_[key];
}

}  // namespace AAFwk
}  // namespace OHOS