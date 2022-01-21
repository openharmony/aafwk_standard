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

#include "inner_mission_info.h"

#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {
std::string InnerMissionInfo::ToJsonStr() const
{
    nlohmann::json value;
    value["MissionName"] = missionName;
    value["IsSingleton"] = isSingletonMode;
    value["MissionId"] = missionInfo.id;
    value["RunningState"] = missionInfo.runningState;
    value["LockedState"] = missionInfo.lockedState;
    value["Continuable"] = missionInfo.continuable;
    value["Time"] = missionInfo.time;
    value["Label"] = missionInfo.label;
    value["IconPath"] = missionInfo.iconPath;
    value["Want"] = missionInfo.want.ToUri();

    return value.dump();
}

void InnerMissionInfo::FromJsonStr(const std::string &jsonStr)
{
    nlohmann::json value = nlohmann::json::parse(jsonStr);
    if (value.is_discarded()) {
        HILOG_ERROR("failed to parse json sting: %{private}s.", jsonStr.c_str());
        return;
    }

    auto CheckJsonNode = [value] (const std::string &node, JsonType jsonType) -> bool {
        if (value.find(node) == value.end()) {
            HILOG_ERROR("node %{private}s not exists.", node.c_str());
            return false;
        }

        if (jsonType == JsonType::NUMBER) {
            return value[node].is_number();
        }
        if (jsonType == JsonType::STRING) {
            return value[node].is_string();
        }
        if (jsonType == JsonType::BOOLEAN) {
            return value[node].is_boolean();
        }
        return false;
    };
    if (CheckJsonNode("MissionName", JsonType::STRING)) {
        missionName = value["MissionName"];
    }
    if (CheckJsonNode("IsSingleton", JsonType::BOOLEAN)) {
        isSingletonMode = value["IsSingleton"];
    }
    if (CheckJsonNode("MissionId", JsonType::NUMBER)) {
        missionInfo.id = value["MissionId"];
    }
    if (CheckJsonNode("RunningState", JsonType::NUMBER)) {
        missionInfo.runningState = value["RunningState"];
    }
    if (CheckJsonNode("LockedState", JsonType::BOOLEAN)) {
        missionInfo.lockedState = value["LockedState"];
    }
    if (CheckJsonNode("Continuable", JsonType::BOOLEAN)) {
        missionInfo.continuable = value["Continuable"];
    }
    if (CheckJsonNode("Time", JsonType::STRING)) {
        missionInfo.time = value["Time"];
    }
    if (CheckJsonNode("Label", JsonType::STRING)) {
        missionInfo.label = value["Label"];
    }
    if (CheckJsonNode("IconPath", JsonType::STRING)) {
        missionInfo.iconPath = value["IconPath"];
    }
    if (CheckJsonNode("Want", JsonType::STRING)) {
        missionInfo.want = *(Want::ParseUri(value["Want"]));
    }
}

void InnerMissionInfo::Dump(std::vector<std::string> &info) const
{
    std::string dumpInfo = "      Mission ID #" + std::to_string(missionInfo.id);
    info.push_back(dumpInfo);
    dumpInfo = "        mission name [" + missionName + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        isSingleton [" + std::to_string(isSingletonMode) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        runningState [" + std::to_string(missionInfo.runningState) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        lockedState [" + std::to_string(missionInfo.lockedState) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        continuable [" + std::to_string(missionInfo.continuable) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        timeStamp [" + missionInfo.time + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        label [" + missionInfo.label + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        iconPath [" + missionInfo.iconPath + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        want [" + missionInfo.want.ToUri() + "]";
    info.push_back(dumpInfo);
}
}  // namespace AAFwk
}  // namespace OHOS
