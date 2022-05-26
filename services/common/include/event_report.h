/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_INNER_EVENT_REPORT_H
#define FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_INNER_EVENT_REPORT_H

#include <string>
#include <unordered_map>

#include "errors.h"
#include "hisysevent.h"
#include "extension_ability_info.h"

using HiSysEventType = OHOS::HiviewDFX::HiSysEvent::EventType;
using HiSysEvent = OHOS::HiviewDFX::HiSysEvent;

namespace OHOS {
namespace AAFWK {
namespace {
// fault event
const std::string START_ABILITY_ERROR = "START_ABILITY_ERROR";
const std::string TERMINATE_ABILITY_ERROR = "TERMINATE_ABILITY_ERROR";
const std::string START_EXTENSION_ERROR = "START_EXTENSION_ERROR";
const std::string STOP_EXTENSION_ERROR = "STOP_EXTENSION_ERROR";
const std::string CONNECT_SERVICE_ERROR = "CONNECT_SERVICE_ERROR";
const std::string DISCONNECT_SERVICE_ERROR = "DISCONNECT_SERVICE_ERROR";
// ability behavior event
const std::string START_ABILITY = "START_ABILITY";
const std::string TERMINATE_ABILITY = "TERMINATE_ABILITY";
const std::string CLOSE_ABILITY = "CLOSE_ABILITY";
const std::string DO_FOREGROUND_ABILITY = "DO_FOREGROUND_ABILITY";
const std::string DO_BACKGROUND_ABILITY = "DO_BACKGROUND_ABILITY";
// serviceExtensionAbility behavior event
const std::string START_SERVICE = "START_SERVICE";
const std::string STOP_SERVICE = "STOP_SERVICE";
const std::string CONNECT_SERVICE = "CONNECT_SERVICE";
const std::string DISCONNECT_SERVICE = "DISCONNECT_SERVICE";
// form behavior event
const std::string ADD_FORM = "ADD_FORM";
const std::string REQUEST_FORM = "REQUEST_FORM";
const std::string DELETE_FORM = "DELETE_FORM";
const std::string CASTTEMP_FORM = "CASTTEMP_FORM";
const std::string ACQUIREFORMSTATE_FORM = "ACQUIREFORMSTATE_FORM";
const std::string MESSAGE_EVENT_FORM = "MESSAGE_EVENT_FORM";
const std::string ROUTE_EVENT_FORM = "ROUTE_EVENT_FORM";
const std::string RELEASE_FORM = "RELEASE_FORM";
const std::string DELETE_INVALID_FORM = "DELETE_INVALID_FORM";
const std::string SET_NEXT_REFRESH_TIME_FORM = "SET_NEXT_REFRESH_TIME_FORM";
// app behavior event
const std::string APP_ATTACH = "APP_ATTACH";
const std::string APP_LAUNCH = "APP_LAUNCH";
const std::string APP_FOREGROUND = "APP_FOREGROUND";
const std::string APP_BACKGROUND = "APP_BACKGROUND";
const std::string APP_TERMINATE = "APP_TERMINATE";
}

struct EventInfo {
    int32_t pid = -1;
    int32_t userId = -1;
    int64_t formId = -1;
    int32_t extensionType = -1;
    uint32_t versionCode = 0;
    uint32_t sceneFlag = -1;
    std::string versionName;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string processName;

    // olny used in fault event
    ErrCode errCode = ERR_OK;

    void Reset()
    {
        pid =-1;
        userId = -1;
        formId =-1;
        versionCode = 0;
        sceneFlag = -1;
        versionName.clear();
        bundleName.clear();
        moduleName.clear();
        abilityName.clear();
        processName.clear();
        extensionType = (int32_t)AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
        errCode = ERR_OK;
    }
};
class EventReport {
public:
    static void SendAppEvent(const std::string &eventName, HiSysEventType type,
        const EventInfo& eventInfo);
    static void SendAbilityEvent(const std::string &eventName, HiSysEventType type,
        const EventInfo& eventInfo);
    static void SendExtensionEvent(const std::string &eventName, HiSysEventType type,
        const EventInfo& eventInfo);
    static void SendFormEvent(const std::string &eventName, HiSysEventType type,
        const EventInfo& eventInfo);
};
}  // namespace AAFWK
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_INNER_EVENT_REPORT_H
