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

#include "event_report.h"
#include "hisysevent.h"
#include "hilog_wrapper.h"

#include <chrono>

namespace OHOS {
namespace AAFWK {
namespace {
const std::string EVENT_KEY_PID = "PID";
const std::string EVENT_KEY_UID = "UID";
const std::string EVENT_KEY_RID = "RID";
const std::string EVENT_KEY_ABILITY_NAME = "ABILITY_NAME";
const std::string EVENT_KEY_ABILITY_TYPE = "ABILITY_TYPE";
const std::string EVENT_KEY_MODEL_TYPE = "MODEL_TYPE";
const std::string EVENT_KEY_BUNDLE_NAME = "BUNDLE_NAME";
const std::string EVENT_KEY_DEVICEID = "DEVICEID";
const std::string EVENT_KEY_URI = "URI";
const std::string EVENT_KEY_ACTION = "ACTION";
const std::string EVENT_KEY_APP_NAME = "NAME";
const std::string EVENT_KEY_VERSION_NAME = "VERSION_NAME";
const std::string EVENT_KEY_VERSION_CODE = "VERSION_CODE";
const std::string EVENT_KEY_TIME_STAMP = "TIME_STAMP";
const std::string EVENT_KEY_FORM_ID = "FORM_ID";
const std::string TYPE = "TYPE";
}
void EventReport::SendHiSysEvent(const std::string &abilityName, const std::string &eventName,
    HiSysEventType type)
{
    EventReport::EventWrite(
        eventName,
        type,
        EVENT_KEY_ABILITY_NAME, abilityName);
    HILOG_WARN("{eventName}: abilityName: %{public}s",
        abilityName.c_str());
}
template<typename... Types>
void EventReport::EventWrite(
    const std::string &eventName,
    HiSysEventType type,
    Types... keyValues)
{
    OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK,
        eventName,
        static_cast<OHOS::HiviewDFX::HiSysEvent::EventType>(type),
        keyValues...);
}
}  // namespace AAFWK
}  // namespace OHOS