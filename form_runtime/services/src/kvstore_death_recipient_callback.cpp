/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "kvstore_death_recipient_callback.h"

#include <thread>
#include <unistd.h>

#include "form_db_cache.h"
#include "hilog_wrapper.h"

using namespace OHOS::DistributedKv;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t CHECK_TIMES = 300;
const int32_t CHECK_INTERVAL = 100000;  // 100ms
}  // namespace
KvStoreDeathRecipientCallback::KvStoreDeathRecipientCallback()
{
    HILOG_INFO("create kvstore death recipient callback instance");
}

KvStoreDeathRecipientCallback::~KvStoreDeathRecipientCallback()
{
    HILOG_INFO("destroy kvstore death recipient callback instance");
}

void KvStoreDeathRecipientCallback::OnRemoteDied()
{
    HILOG_INFO("OnRemoteDied, register data change listener begin");

    auto dataStorage = FormDbCache::GetInstance().GetDataStorage();
    if (!dataStorage) {
        HILOG_ERROR("dataStorage is nullptr");
        return;
    }

    std::thread([dataStorage] {
        int32_t times = 0;
        while (times < CHECK_TIMES) {
            times++;
            // init kvStore.
            if (dataStorage && dataStorage->ResetKvStore()) {
                // register data change listener again.
                HILOG_INFO("current times is %{public}d", times);
                break;
            }
            usleep(CHECK_INTERVAL);
        }
    }).detach();

    HILOG_INFO("OnRemoteDied, register data change listener end");
}
}  // namespace AppExecFwk
}  // namespace OHOS