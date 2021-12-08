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

#include "wait_multiapp_return_storage.h"
#include "ability_util.h"

namespace OHOS {
namespace AAFwk {
WaitMultiAppReturnRecord::WaitMultiAppReturnRecord(const sptr<IRemoteObject> &callerToken)
{
    callerToken_ = callerToken;
}

sptr<IRemoteObject> WaitMultiAppReturnRecord::GetCallerToken()
{
    return callerToken_;
}

int WaitMultiAppReturnRecord::WaitForMultiAppSelectorReturn()
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mutex_);
    waitSelectorCond_.wait(lock);
    return requestUid_;
}

void WaitMultiAppReturnRecord::multiAppSelectorReturn(int requestUid)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mutex_);
    requestUid_ = requestUid;
    waitSelectorCond_.notify_all();
}

std::shared_ptr<WaitMultiAppReturnRecord> WaitMultiAppReturnStorage::AddRecord(const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_POINTER_AND_RETURN(callerToken, nullptr);
    auto isExist = [&callerToken](const std::shared_ptr<WaitMultiAppReturnRecord> &record) {
        return callerToken == record->GetCallerToken();
    };
    auto iter = std::find_if(recordsVector_.begin(), recordsVector_.end(), isExist);
    if (iter == recordsVector_.end()) {
        std::shared_ptr<WaitMultiAppReturnRecord> record = std::make_shared<WaitMultiAppReturnRecord>(callerToken);
        recordsVector_.emplace_back(record);
        HILOG_INFO("WaitMultiAppReturnStorage::recordsVector_ size %{public}zu", recordsVector_.size());
        return record;
    }
    return *iter;
}

void WaitMultiAppReturnStorage::RemoveRecord(const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_POINTER(callerToken);
    auto isExist = [&callerToken](const std::shared_ptr<WaitMultiAppReturnRecord> &record) {
        return callerToken == record->GetCallerToken();
    };
    auto iter = std::find_if(recordsVector_.begin(), recordsVector_.end(), isExist);
    if (iter != recordsVector_.end()) {
        recordsVector_.erase(iter);
    }
    HILOG_INFO("WaitMultiAppReturnStorage::recordsVector_ size %{public}zu", recordsVector_.size());
}

std::shared_ptr<WaitMultiAppReturnRecord> WaitMultiAppReturnStorage::GetRecord(const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_POINTER_AND_RETURN(callerToken, nullptr);
    auto isExist = [&callerToken](const std::shared_ptr<WaitMultiAppReturnRecord> &record) {
        return callerToken == record->GetCallerToken();
    };
    auto iter = std::find_if(recordsVector_.begin(), recordsVector_.end(), isExist);
    if (iter != recordsVector_.end()) {
        return *iter;
    }
    return nullptr;
}
}  // namespace AAFwk
}  // namespace OHOS
