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

#include "application_info.h"
#include "want.h"
#include "errors.h"

namespace OHOS {
namespace AAFWK {
namespace {
// event type
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
const std::string DO_FOREGROUND_ABILITY = "DO_FOREGROUND_ABILITY";
const std::string DO_BACKGROUND_ABILITY = "DO_BACKGROUND_ABILITY";
const std::string ABILITY_ONSTART = "ABILITY_ONSTART";
const std::string ABILITY_ONSTOP = "ABILITY_ONSTOP";
const std::string ABILITY_ONFOREGROUND = "ABILITY_ONFOREGROUND";
const std::string ABILITY_ONBACKGROUND = "ABILITY_ONBACKGROUND";
const std::string ABILITY_WINDOWSTAGE_CREATE = "ABILITY_WINDOWSTAGE_CREATE";
const std::string ABILITY_WINDOWSTAGE_DESTORY = "ABILITY_WINDOWSTAGE_DESTORY";
// serviceExtensionAbility behavior event
const std::string START_SERVICE_EXTENSION = "START_SERVICE_EXTENSION";
const std::string STOP_SERVICE_EXTENSION = "STOP_SERVICE_EXTENSION";
const std::string CONNECT_SERVICE_EXTENSION = "CONNECT_SERVICE_EXTENSION";
const std::string DISCONNECT_SERVICE_EXTENSION = "DISCONNECT_SERVICE_EXTENSION";
const std::string SERVICE_EXTENSION_ONSTART = "SERVICE_EXTENSION_ONSTART";
const std::string SERVICE_EXTENSION_ONSTOP = "SERVICE_EXTENSION_ONSTOP";
const std::string SERVICE_EXTENSION_ONCONNECT = "SERVICE_EXTENSION_ONCONNECT";
const std::string SERVICE_EXTENSION_ONDISCONNECT = "SERVICE_EXTENSION_ONDISCONNECT";
const std::string SERVICE_EXTENSION_ONREQUEST = "SERVICE_EXTENSION_ONREQUEST";
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
const std::string FORM_LIFECYCLE_ONCREATE = "FORM_LIFECYCLE_ONCREATE";
const std::string FORM_LIFECYCLE_ONDESTROY = "FORM_LIFECYCLE_ONDESTROY";
const std::string FORM_LIFECYCLE_ONUPDATE = "FORM_LIFECYCLE_ONUPDATE";
const std::string FORM_LIFECYCLE_EVENT = "FORM_LIFECYCLE_EVENT";
const std::string FORM_LIFECYCLE_ONCASTTEMPFORM = "FORM_LIFECYCLE_ONCASTTEMPFORM";
const std::string FORM_LIFECYCLE_ONACQUIREFORMSTATE = "FORM_LIFECYCLE_ONACQUIREFORMSTATE";
// app behavior event
const std::string APP_ATTACH = "APP_ATTACH";
const std::string APP_LAUNCH = "APP_LAUNCH";
const std::string APP_FOREGROUND = "APP_FOREGROUND";
const std::string APP_BACKGROUND = "APP_BACKGROUND";
const std::string APP_TERMINATE = "APP_TERMINATE";
}

enum HiSysEventType {
    FAULT = 1,
    STATISTIC = 2,
    SECURITY = 3,
    BEHAVIOR = 4,
};

struct EventInfo {
    int32_t pid = -1;
    int32_t userId = -1;
    int64_t timeStamp = 0;
    int64_t formId = -1;
    uint32_t versionCode = 0;
    std::string versionName;
    std::string bundleName;
    std::string abilityName;

    // olny used in fault event
    ErrCode errCode = ERR_OK;

    void Reset()
    {
        pid =-1;
        userId = -1;
        timeStamp = 0;
        formId =-1;
        versionCode = 0;
        versionName.clear();
        bundleName.clear();
        abilityName.clear();
        errCode = ERR_OK;
    }
};
class EventReport {
public:
    static void SendAppEvent(const std::string &eventName, HiSysEventType type, const EventInfo& eventInfo);
    static void SendAbilityEvent(const std::string &eventName, HiSysEventType type, const EventInfo& eventInfo);
    static void SendExtensionEvent(const std::string &eventName, HiSysEventType type, const EventInfo& eventInfo);
    static void SendFormEvent(const std::string &eventName, HiSysEventType type, const EventInfo& eventInfo);
    template<typename... Types>
    static void EventWrite(const std::string &eventName,
        HiSysEventType type, Types... keyValues);
};
}  // namespace AAFWK
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_INNER_EVENT_REPORT_H
