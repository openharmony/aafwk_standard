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

#include "ams_configuration_parameter.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
using json = nlohmann::json;
static const int EXPERIENCE_MEM_THRESHOLD = 20;

void AmsConfigurationParameter::Parse()
{
    auto ref = LoadAmsConfiguration(AmsConfig::AMS_CONFIG_FILE_PATH);
    HILOG_INFO("load config ref : %{public}d", ref);
}

bool AmsConfigurationParameter::GetStartSettingsDataState() const
{
    return canStartSettingsData_;
}

bool AmsConfigurationParameter::GetStartScreenLockState() const
{
    return canStartScreenLock_;
}

bool AmsConfigurationParameter::NonConfigFile() const
{
    return nonConfigFile_;
}

int AmsConfigurationParameter::GetMissionSaveTime() const
{
    return missionSaveTime_;
}

std::string AmsConfigurationParameter::GetOrientation() const
{
    return orientation_;
}

int AmsConfigurationParameter::GetANRTimeOutTime() const
{
    return anrTime_;
}

int AmsConfigurationParameter::GetAMSTimeOutTime() const
{
    return amsTime_;
}

int AmsConfigurationParameter::GetMaxRestartNum() const
{
    return maxRestartNum_;
}

int AmsConfigurationParameter::LoadAmsConfiguration(const std::string &filePath)
{
    HILOG_DEBUG("%{public}s", __func__);
    int ret[2] = {0};
    std::ifstream inFile;
    inFile.open(filePath, std::ios::in);
    if (!inFile.is_open()) {
        HILOG_INFO("read ams config error ...");
        nonConfigFile_ = true;
        return READ_FAIL;
    }

    json amsJson;
    inFile >> amsJson;
    if (amsJson.is_discarded()) {
        HILOG_INFO("json discarded error ...");
        nonConfigFile_ = true;
        inFile.close();
        return READ_JSON_FAIL;
    }

    ret[0] = LoadAppConfigurationForStartUpService(amsJson);
    if (ret[0] != 0) {
        HILOG_ERROR("LoadAppConfigurationForStartUpService return error");
    }

    ret[1] = LoadAppConfigurationForMemoryThreshold(amsJson);
    if (ret[1] != 0) {
        HILOG_ERROR("LoadAppConfigurationForMemoryThreshold return error");
    }

    LoadSystemConfiguration(amsJson);
    amsJson.clear();
    inFile.close();

    for (auto& i : ret) {
        if (i != 0) {
            HILOG_ERROR("json no have service item ...");
            return READ_JSON_FAIL;
        }
    }

    HILOG_INFO("read ams config success!");
    return READ_OK;
}

int AmsConfigurationParameter::LoadAppConfigurationForStartUpService(nlohmann::json& Object)
{
    int ret = -1;
    if (Object.contains(AmsConfig::SERVICE_ITEM_AMS)) {
        canStartSettingsData_ = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_SETTINGS_DATA).get<bool>();
        canStartScreenLock_ = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_SCREEN_LOCK).get<bool>();
        missionSaveTime_ = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::MISSION_SAVE_TIME).get<int>();
        anrTime_ =
            Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::APP_NOT_RESPONSE_PROCESS_TIMEOUT_TIME).get<int>();
        amsTime_ =
            Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::AMS_TIMEOUT_TIME).get<int>();
        maxRestartNum_ = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::ROOT_LAUNCHER_RESTART_MAX).get<int>();
        HILOG_INFO("get ams service config succes!");
        ret = 0;
    }

    return ret;
}

int AmsConfigurationParameter::LoadAppConfigurationForMemoryThreshold(nlohmann::json &Object)
{
    int ret = 0;
    if (!Object.contains("memorythreshold")) {
        HILOG_ERROR("LoadAppConfigurationForMemoryThreshold return error");
        ret = -1;
        return ret;
    }

    if (Object.at("memorythreshold").contains("home_application")) {
        memThreshold_["home_application"] = Object.at("memorythreshold").at("home_application").get<std::string>();
    } else {
        HILOG_ERROR("LoadAppConfigurationForMemoryThreshold memorythreshold::home_application is nullptr");
    }

    return ret;
}

int AmsConfigurationParameter::LoadSystemConfiguration(nlohmann::json& Object)
{
    if (Object.contains(AmsConfig::SYSTEM_CONFIGURATION)) {
        orientation_ = Object.at(AmsConfig::SYSTEM_CONFIGURATION).at(AmsConfig::SYSTEM_ORIENTATION).get<std::string>();
        return READ_OK;
    }

    return READ_FAIL;
}

/**
 * The low memory threshold under which the system will kill background processes
 */
int AmsConfigurationParameter::GetMemThreshold(const std::string &key)
{
    auto threshold = memThreshold_.find(key);
    if (threshold == memThreshold_.end()) {
        HILOG_ERROR("%{public}s, threshold[%{public}s] find failed", __func__, key.c_str());
        return EXPERIENCE_MEM_THRESHOLD;
    }

    return std::stoi(threshold->second);
}

}  // namespace AAFwk
}  // namespace OHOS
