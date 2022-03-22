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

#ifndef OHOS_AAFWK_AMS_CONFIGURATION_PARAMETER_H
#define OHOS_AAFWK_AMS_CONFIGURATION_PARAMETER_H

#include <fstream>
#include <nlohmann/json.hpp>

namespace OHOS {
namespace AAFwk {
namespace AmsConfig {
namespace MemThreshold {
const std::string HOME_APP("home_application");
}
const std::string AMS_CONFIG_FILE_PATH {"/system/etc/ams_service_config.json"};
const std::string SERVICE_ITEM_AMS {"service_startup_config"};
const std::string STARTUP_LAUNCHER {"startup_launcher"};
const std::string STARTUP_SETTINGS_DATA {"startup_settings_data"};
const std::string STARTUP_SCREEN_LOCK {"startup_screen_lock"};
const std::string STARTUP_STATUS_BAR {"startup_system_ui_status_bar"};
const std::string STARTUP_NAVIGATION_BAR {"startup_system_ui_navigation_bar"};
const std::string STARTUP_PHONE_SERVICE {"startup_phone_service"};
const std::string MISSION_SAVE_TIME {"mission_save_time"};
const std::string APP_NOT_RESPONSE_PROCESS_TIMEOUT_TIME {"app_not_response_process_timeout_time"};
const std::string AMS_TIMEOUT_TIME {"ams_timeout_time"};
const std::string SYSTEM_CONFIGURATION {"system_configuration"};
const std::string SYSTEM_ORIENTATION {"system_orientation"};
const std::string STARTUP_MMS {"startup_mms"};
const std::string USE_NEW_MISSION {"use_new_mission"};
const std::string ROOT_LAUNCHER_RESTART_MAX {"root_launcher_restart_max"};
}  // namespace AmsConfig

enum class SatrtUiMode { STATUSBAR = 1, NAVIGATIONBAR = 2, STARTUIBOTH = 3 };

class AmsConfigurationParameter final {
public:
    AmsConfigurationParameter() = default;
    ~AmsConfigurationParameter() = default;
    /**
     * return true : ams no config file
     * return false : ams have config file
     */
    bool NonConfigFile() const;
    /**
     * return true : ams can start launcher
     * return false : ams do not start launcher
     */
    bool GetStartLauncherState() const;
    /**
     * return true : ams can start settings data
     * return false : ams do not start settings data
     */
    bool GetStartSettingsDataState() const;
    /**
     * return true : ams can start screen lock
     * return false : ams do not start screen lock
     */
    bool GetStartScreenLockState() const;
    /**
     * return true : ams can start system ui status bar
     * return false : ams do not start system ui status bar
     */
    bool GetStatusBarState() const;
    /**
     * return true : ams can start system ui navigation bar
     * return false : ams do not start system ui navigation bar
     */
    bool GetNavigationBarState() const;
    /**
     * return true : ams can start phone service
     * return false : ams do not start phone service
     */
    bool GetPhoneServiceState() const;
    /**
     * return true : ams can start mms
     * return false : ams do not start mms
     */
    bool GetStartMmsState() const;
    /**
     * Get profile information
     */
    void Parse();
    /**
     * The low memory threshold under which the system will kill background processes
     */
    int GetMemThreshold(const std::string &key);
    /**
     * Get the save time of the current content
     */
    int GetMissionSaveTime() const;
    /**
     * Get current system direction parameters, Temporary method.
     */
    std::string GetOrientation() const;
    /**
     * check if use new mission.
     *
     * return true if use mission list, false if use mission stack.
     */
    bool IsUseNewMission() const;
    /**
     * Get the max number of restart.
     */
    int GetMaxRestartNum() const;
    /**
     * get the application not response process timeout time.
     */
    int GetANRTimeOutTime() const;
    /**
     * get ability manager service not response process timeout time.
     */
    int GetAMSTimeOutTime() const;

    enum { READ_OK = 0, READ_FAIL = 1, READ_JSON_FAIL = 2 };

private:
    /**
     * Read the configuration file of ams
     *
     */
    int LoadAmsConfiguration(const std::string &filePath);
    int LoadAppConfigurationForStartUpService(nlohmann::json& Object);
    int LoadAppConfigurationForMemoryThreshold(nlohmann::json& Object);
    int LoadSystemConfiguration(nlohmann::json& Object);

private:
    bool nonConfigFile_ {false};
    bool canStartLauncher_ {false};
    bool canStartSettingsData_ {false};
    bool canStartScreenLock_ {false};
    bool canStartUiStatusBar_ {false};
    bool canStartUiNavigationBar_ {false};
    bool canStartPhoneService_ {false};
    bool canStartMms {false};
    bool useNewMission_ {false};
    int maxRestartNum_ = 0;
    std::string orientation_ {""};
    int missionSaveTime_ {12 * 60 * 60 * 1000};
    int anrTime_ {5000};
    int amsTime_ {5000};
    std::map<std::string, std::string> memThreshold_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_AMS_CONFIGURATION_PARAMETER_H
