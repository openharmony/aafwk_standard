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

#ifndef OHOS_AAFWK_MULTIAPP_SELECTOR_RETURN_STORAGE_H
#define OHOS_AAFWK_MULTIAPP_SELECTOR_RETURN_STORAGE_H

#include <mutex>
#include "ability_record.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
class WaitMultiAppReturnRecord {
public:
    WaitMultiAppReturnRecord(const sptr<IRemoteObject> &callerToken);
    virtual ~WaitMultiAppReturnRecord() = default;
    sptr<IRemoteObject> GetCallerToken();
    int WaitForMultiAppSelectorReturn();
    void multiAppSelectorReturn(int requestUid);
private:
    std::mutex mutex_;
    sptr<IRemoteObject> callerToken_ = nullptr;
    std::condition_variable waitSelectorCond_ {};
    int requestUid_ = -1;
};

class WaitMultiAppReturnStorage {
public:
    WaitMultiAppReturnStorage() = default;
    virtual ~WaitMultiAppReturnStorage() = default;

    std::shared_ptr<WaitMultiAppReturnRecord> AddRecord(const sptr<IRemoteObject> &callerToken);
    void RemoveRecord(const sptr<IRemoteObject> &callerToken);
    std::shared_ptr<WaitMultiAppReturnRecord> GetRecord(const sptr<IRemoteObject> &callerToken);

private:
    std::mutex mutex_;
    std::vector<std::shared_ptr<WaitMultiAppReturnRecord>> recordsVector_ = {};
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_MULTIAPP_SELECTOR_RETURN_STORAGE_H
