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

#ifndef FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INNER_MISSION_INFO_H
#define FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INNER_MISSION_INFO_H

#include <string>

#include "mission_info.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
enum class JsonType {
    NULLABLE,
    BOOLEAN,
    NUMBER,
    OBJECT,
    ARRAY,
    STRING,
};

/**
 * @struct InnerMissionInfo
 * InnerMissionInfo is used to save informations about mission information.
 */
struct InnerMissionInfo {
    MissionInfo missionInfo;
    std::string missionName;
    bool isSingletonMode;

    std::string ToJsonStr() const;
    void FromJsonStr(const std::string &jsonStr);
    void Dump(std::vector<std::string> &info) const;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INNER_MISSION_INFO_H
