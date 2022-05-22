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

#include "ability_info.h"
#include "application_info.h"
#include "want.h"

namespace OHOS {
namespace AAFWK {
enum HiSysEventType {
    FAULT = 1,
    STATISTIC = 2,
    SECURITY = 3,
    BEHAVIOR = 4,
};
class EventReport {
public:
    static void AppEvent(const std::shared_ptr<AppExecFwk::ApplicationInfo> &applicationInfo,
    const std::string &pid, const std::string &eventName, HiSysEventType type);
    template<typename... Types>
    static void EventWrite(const std::string &eventName,
        HiSysEventType type, Types... keyValues);
};
}  // namespace AAFWK
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_INNER_EVENT_REPORT_H
