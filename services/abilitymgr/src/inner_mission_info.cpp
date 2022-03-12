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

#include "inner_mission_info.h"

#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string KEY_MISSION_NAME = "MissionName";
const std::string KEY_IS_SINGLETON = "IsSingleton";
const std::string KEY_MISSION_ID = "MissionId";
const std::string KEY_RUNNING_STATE = "RunningState";
const std::string KEY_LOCKED_STATE = "LockedState";
const std::string KEY_CONTINUABLE = "Continuable";
const std::string KEY_TIME = "Time";
const std::string KEY_LABEL = "Label";
const std::string KEY_ICON_PATH = "IconPath";
const std::string KEY_WANT = "Want";
const std::string KEY_START_METHOD = "StartMethod";
const std::string KEY_BUNDLE_NAME = "BundleName";
const std::string KEY_UID = "Uid";
}
std::string InnerMissionInfo::ToJsonStr() const
{
    nlohmann::json value;
    value[KEY_MISSION_NAME] = missionName;
    value[KEY_IS_SINGLETON] = isSingletonMode;
    value[KEY_BUNDLE_NAME] = bundleName;
    value[KEY_UID] = uid;
    value[KEY_MISSION_ID] = missionInfo.id;
    value[KEY_RUNNING_STATE] = missionInfo.runningState;
    value[KEY_LOCKED_STATE] = missionInfo.lockedState;
    value[KEY_CONTINUABLE] = missionInfo.continuable;
    value[KEY_TIME] = missionInfo.time;
    value[KEY_LABEL] = missionInfo.label;
    value[KEY_ICON_PATH] = missionInfo.iconPath;
    value[KEY_WANT] = missionInfo.want.ToUri();

    return value.dump();
}

bool InnerMissionInfo::FromJsonStr(const std::string &jsonStr)
{
    // Do not throw exceptions in nlohmann::json::parse
    nlohmann::json value = nlohmann::json::parse(jsonStr, nullptr, false);
    if (value.is_discarded()) {
        HILOG_ERROR("failed to parse json sting: %{private}s.", jsonStr.c_str());
        return false;
    }

    auto CheckJsonNode = [&value] (const std::string &node, JsonType jsonType) -> bool {
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
    if (!CheckJsonNode(KEY_MISSION_NAME, JsonType::STRING)) {
        return false;
    }
    missionName = value[KEY_MISSION_NAME].get<std::string>();
    if (!CheckJsonNode(KEY_IS_SINGLETON, JsonType::BOOLEAN)) {
        return false;
    }
    isSingletonMode = value[KEY_IS_SINGLETON].get<bool>();
    if (!CheckJsonNode(KEY_START_METHOD, JsonType::NUMBER)) {
        return false;
    }
    startMethod = value[KEY_START_METHOD].get<int32_t>();
    if (!CheckJsonNode(KEY_BUNDLE_NAME, JsonType::STRING)) {
        return false;
    }
    bundleName = value[KEY_BUNDLE_NAME].get<std::string>();
    if (!CheckJsonNode(KEY_UID, JsonType::NUMBER)) {
        return false;
    }
    uid = value[KEY_UID].get<int32_t>();
    if (!CheckJsonNode(KEY_MISSION_ID, JsonType::NUMBER)) {
        return false;
    }
    missionInfo.id = value[KEY_MISSION_ID].get<int32_t>();
    if (!CheckJsonNode(KEY_RUNNING_STATE, JsonType::NUMBER)) {
        return false;
    }
    missionInfo.runningState = value[KEY_RUNNING_STATE].get<int32_t>();
    if (!CheckJsonNode(KEY_LOCKED_STATE, JsonType::BOOLEAN)) {
        return false;
    }
    missionInfo.lockedState = value[KEY_LOCKED_STATE].get<bool>();
    if (!CheckJsonNode(KEY_CONTINUABLE, JsonType::BOOLEAN)) {
        return false;
    }
    missionInfo.continuable = value[KEY_CONTINUABLE].get<bool>();
    if (!CheckJsonNode(KEY_TIME, JsonType::STRING)) {
        return false;
    }
    missionInfo.time = value[KEY_TIME].get<std::string>();
    if (!CheckJsonNode(KEY_LABEL, JsonType::STRING)) {
        return false;
    }
    missionInfo.label = value[KEY_LABEL].get<std::string>();
    if (!CheckJsonNode(KEY_ICON_PATH, JsonType::STRING)) {
        return false;
    }
    missionInfo.iconPath = value[KEY_ICON_PATH].get<std::string>();
    if (!CheckJsonNode(KEY_WANT, JsonType::STRING)) {
        return false;
    }
    Want* want = Want::ParseUri(value[KEY_WANT].get<std::string>());
    if (want) {
        missionInfo.want = *want;
    }
    return true;
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
