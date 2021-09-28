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

void AmsConfigurationParameter::Parse()
{
    auto ref = LoadAmsConfiguration(AmsConfig::AMS_CONFIG_FILE_PATH);
    HILOG_INFO("load config ref : %{public}d", ref);
}

bool AmsConfigurationParameter::GetStartLauncherState() const
{
    return canStartLauncher;
}

bool AmsConfigurationParameter::GetStatusBarState() const
{
    return canStartUiStatusBar;
}

bool AmsConfigurationParameter::GetNavigationBarState() const
{
    return canStartUiNavigationBar;
}

bool AmsConfigurationParameter::NonConfigFile() const
{
    return nonConfigFile;
}

int AmsConfigurationParameter::LoadAmsConfiguration(const std::string &filePath)
{
    HILOG_DEBUG("%{public}s", __func__);

    std::ifstream inFile;
    inFile.open(filePath, std::ios::in);
    if (!inFile.is_open()) {
        HILOG_INFO("read ams config error ...");
        nonConfigFile = true;
        return READ_FAIL;
    }

    json amsJson;
    inFile >> amsJson;
    if (amsJson.is_discarded()) {
        HILOG_INFO("json discarded error ...");
        nonConfigFile = true;
        inFile.close();
        return READ_JSON_FAIL;
    }

    if (amsJson.contains(AmsConfig::SERVICE_ITEM_AMS)) {
        canStartLauncher = amsJson.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_LAUNCHER).get<bool>();
        canStartUiStatusBar = amsJson.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_STATUS_BAR).get<bool>();
        canStartUiNavigationBar =
            amsJson.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_NAVIGATION_BAR).get<bool>();
        HILOG_INFO("get ams service config succes!");
    } else {
        HILOG_INFO("json no have service item ...");
        nonConfigFile = true;
        amsJson.clear();
        inFile.close();
        return READ_JSON_FAIL;
    }

    amsJson.clear();
    inFile.close();
    HILOG_INFO("read ams config succes!");
    return READ_OK;
}

}  // namespace AAFwk
}  // namespace OHOS