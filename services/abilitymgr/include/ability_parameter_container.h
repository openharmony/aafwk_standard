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

#ifndef OHOS_AAFWK_ABILITY_PARAMETER_CONTAINER_H
#define OHOS_AAFWK_ABILITY_PARAMETER_CONTAINER_H
#include <mutex>
#include "ability_record.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
class AbilityParameterContainer {
public:
    AbilityParameterContainer() = default;
    virtual ~AbilityParameterContainer() = default;

    void AddParameter(int abilityRecordId, const AbilityRequest &abilityRequest);
    void RemoveParameterByID(int abilityRecordId);
    AbilityRequest GetAbilityRequestFromContainer(int abilityRecordId);
    bool IsExist(int abilityRecordId);

private:
    std::mutex lock_;
    std::map<int, AbilityRequest> parameterContainer_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_PARAMETER_CONTAINER_H
