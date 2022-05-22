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
const std::string EVENT_KEY_APP_NAME = "NAME";
const std::string EVENT_KEY_VERSION_NAME = "VERSION_NAME";
const std::string EVENT_KEY_VERSION_CODE = "VERSION_CODE";
const std::string EVENT_KEY_PID = "PID";
const std::string EVENT_KEY_TIME_STAMP = "TIME_STAMP";
const std::string TYPE = "TYPE";
}
void EventReport::AppEvent(const std::shared_ptr<AppExecFwk::ApplicationInfo> &applicationInfo,
    const std::string &pid, const std::string &eventName, HiSysEventType type)
{
    std::string name = applicationInfo->name.c_str();
    std::string versionName = applicationInfo->versionName.c_str();
    uint32_t versionCode = applicationInfo->versionCode;
    int32_t timeStamp =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
    EventReport::EventWrite(
        eventName,
        type,
        EVENT_KEY_APP_NAME, name,
        EVENT_KEY_VERSION_NAME, versionName,
        EVENT_KEY_VERSION_CODE, std::to_string(versionCode),
        EVENT_KEY_PID, pid.c_str(),
        EVENT_KEY_TIME_STAMP, std::to_string(timeStamp));
    HILOG_WARN("{eventName}: name: %{public}s, versionName: %{public}s,"
        "versionCode: %{public}d, pid: %{public}s, timeStamp: %{public}d",
        name.c_str(),
        versionName.c_str(),
        versionCode,
        pid.c_str(),
        timeStamp);
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