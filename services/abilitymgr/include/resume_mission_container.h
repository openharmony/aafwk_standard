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

#ifndef OHOS_AAFWK_RESUME_MISSION_CONTAINER_H
#define OHOS_AAFWK_RESUME_MISSION_CONTAINER_H

#include "mission_record.h"
#include "ability_event_handler.h"

namespace OHOS {
namespace AAFwk {
class ResumeMissionContainer {
public:
    explicit ResumeMissionContainer(const std::shared_ptr<AbilityEventHandler> &handler);
    virtual ~ResumeMissionContainer() = default;

    void Save(const std::shared_ptr<MissionRecord> &mission);
    void Resume(const std::shared_ptr<MissionRecord> &mission);
    bool IsResume(int missionId);
    void Remove(int missionId);

private:
    const std::string taskName_ = "ResumeMission#";
    std::map<int, std::shared_ptr<MissionRecord>> missionMaps_;
    std::shared_ptr<AbilityEventHandler> handler_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_RESUME_MISSION_CONTAINER_H