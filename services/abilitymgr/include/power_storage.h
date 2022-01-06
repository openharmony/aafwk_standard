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

#ifndef OHOS_AAFWK_POWER_STORAGE_H
#define OHOS_AAFWK_POWER_STORAGE_H

#include "ability_record.h"

namespace OHOS {
namespace AAFwk {
struct PowerOffRecord {
    std::weak_ptr<AbilityRecord> ability;
    int32_t missionId;
    int32_t StackId;
    AbilityState state;
};

class PowerStorage {
public:
    PowerStorage() = default;
    virtual ~PowerStorage() = default;

    void SetPowerOffInActiveRecord(const std::shared_ptr<AbilityRecord> &ability);
    std::vector<PowerOffRecord> GetPowerOffInActiveRecord() const;
    void SetPowerOffActiveRecord(const std::shared_ptr<AbilityRecord> &ability);
    std::vector<PowerOffRecord> GetPowerOffActiveRecord() const;
    void UpdatePowerOffRecord(int32_t missionId, const std::shared_ptr<AbilityRecord> &ability);

    void SetPowerOffInActiveRecordLockScreen(const std::shared_ptr<AbilityRecord> &ability);
    std::vector<PowerOffRecord> GetPowerOffInActiveRecordLockScreen() const;
    void SetPowerOffActiveRecordLockScreen(const std::shared_ptr<AbilityRecord> &ability);
    std::vector<PowerOffRecord> GetPowerOffActiveRecordLockScreen() const;
    void Clear(bool isLockScreen);

private:
    std::vector<PowerOffRecord> inActiveRecord_;
    std::vector<PowerOffRecord> activeRecord_;
    std::vector<PowerOffRecord> inActiveRecordLockScreen_;
    std::vector<PowerOffRecord> activeRecordLockScreen_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_POWER_STORAGE_H
