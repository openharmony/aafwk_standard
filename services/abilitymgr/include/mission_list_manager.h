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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H

#include <list>
#include <memory>

#include "mission_list.h"

namespace OHOS {
namespace AbilityRuntime {
class MissionListManager {
public:
private:
    std::list<std::shared_ptr<AbilityRuntime::MissionList>> currentMissionLists_;
    // only manager the ability of standard in the default list
    std::shared_ptr<AbilityRuntime::MissionList> defaultStandardList_;
    // only manager the ability of singleton in the default list for the fast search
    std::shared_ptr<AbilityRuntime::MissionList> defaultSingleList_;
    std::shared_ptr<AbilityRuntime::MissionList> launcherList_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H