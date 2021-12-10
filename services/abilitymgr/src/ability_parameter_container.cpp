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

#include "ability_parameter_container.h"

namespace OHOS {
namespace AAFwk {
void AbilityParameterContainer::AddParameter(int abilityRecordId, const AbilityRequest &abilityRequest)
{
    std::lock_guard<std::mutex> guard(lock_);
    parameterContainer_.emplace(abilityRecordId, abilityRequest);
}

void AbilityParameterContainer::RemoveParameterByID(int abilityRecordId)
{
    std::lock_guard<std::mutex> guard(lock_);
    parameterContainer_.erase(abilityRecordId);
    HILOG_INFO("parameterContainer_ size %{public}zu", parameterContainer_.size());
}

AbilityRequest AbilityParameterContainer::GetAbilityRequestFromContainer(int abilityRecordId)
{
    std::lock_guard<std::mutex> guard(lock_);
    AbilityRequest abilityRequest;
    auto iter = parameterContainer_.find(abilityRecordId);
    if (iter != parameterContainer_.end()) {
        return iter->second;
    }
    return abilityRequest;
}

bool AbilityParameterContainer::IsExist(int abilityRecordId)
{
    std::lock_guard<std::mutex> guard(lock_);
    return (parameterContainer_.find(abilityRecordId) != parameterContainer_.end());
}
}  // namespace AAFwk
}  // namespace OHOS
