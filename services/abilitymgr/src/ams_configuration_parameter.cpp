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

#include "ams_configuration_parameter.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
using json = nlohmann::json;
static const int experienceMemThreshold = 20;

void AmsConfigurationParameter::Parse()
{
    auto ref = LoadAmsConfiguration(AmsConfig::AMS_CONFIG_FILE_PATH);
    HILOG_INFO("load config ref : %{public}d", ref);
}

bool AmsConfigurationParameter::GetStartLauncherState() const
{
    return canStartLauncher_;
}

bool AmsConfigurationParameter::GetStatusBarState() const
{
    return canStartUiStatusBar_;
}

bool AmsConfigurationParameter::GetNavigationBarState() const
{
    return canStartUiNavigationBar_;
}

bool AmsConfigurationParameter::GetPhoneServiceState() const
{
    return canStartPhoneService_;
}

bool AmsConfigurationParameter::GetStartContactsState() const
{
    return canStartContacts;
}

bool AmsConfigurationParameter::GetStartMmsState() const
{
    return canStartMms;
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

    HILOG_INFO("read ams config succes!");
    return READ_OK;
}

int AmsConfigurationParameter::LoadAppConfigurationForStartUpService(nlohmann::json& Object)
{
    int ret = -1;
    if (Object.contains(AmsConfig::SERVICE_ITEM_AMS)) {
        canStartLauncher_ = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_LAUNCHER).get<bool>();
        canStartUiStatusBar_ = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_STATUS_BAR).get<bool>();
        canStartUiNavigationBar_ =
            Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_NAVIGATION_BAR).get<bool>();
        canStartPhoneService_ =
            Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_PHONE_SERVICE).get<bool>();
        canStartContacts = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_CONTACTS).get<bool>();
        canStartMms = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_MMS).get<bool>();
        missionSaveTime_ = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::MISSION_SAVE_TIME).get<int>();
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
        return experienceMemThreshold;
    }

    return std::stoi(threshold->second);
}

}  // namespace AAFwk
}  // namespace OHOS
