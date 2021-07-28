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

#ifndef OHOS_AAFWK_LOCK_MISSION_CONTAINER_H
#define OHOS_AAFWK_LOCK_MISSION_CONTAINER_H

#include <list>

#include "mission_record.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class LockMissionContainer
 * LockMissionContainer records locked mission record.
 */
class LockMissionContainer {
public:
    LockMissionContainer() = default;
    virtual ~LockMissionContainer() = default;

    enum LockMissionState { LOCK_MISSION_STATE_NONE = 0, LOCK_MISSION_STATE_LOCKED, LOCK_MISSION_STATE_PINNED };

    bool IsLockedMissionState() const;
    bool SetLockedMission(const std::shared_ptr<MissionRecord> &mission, int lockUid, bool isSystemApp);
    bool ReleaseLockedMission(const std::shared_ptr<MissionRecord> &mission, int callerUid, bool isForce);
    bool IsSameLockedMission(std::string bundleName) const;
    std::shared_ptr<MissionRecord> GetLockMission();
    int GetLockedMissionState() const;

private:
    void Clear();

private:
    int lockerUid_ = -1;
    LockMissionState lockState_ = LOCK_MISSION_STATE_NONE;
    std::weak_ptr<MissionRecord> lockMission_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_LOCK_MISSION_CONTAINER_H