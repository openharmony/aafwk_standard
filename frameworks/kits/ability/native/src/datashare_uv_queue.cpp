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

#include "datashare_uv_queue.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr int WAIT_TIME_OUT = 3;
DataShareUvQueue::DataShareUvQueue(napi_env env)
    : env_(env)
{
    napi_get_uv_event_loop(env, &loop_);
}

void DataShareUvQueue::SyncCall(NapiVoidFunc func)
{
    HILOG_INFO("%{public}s begin.", __func__);
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }
    work->data = new UvEntry {env_, std::move(func), false, false, false, {}, {}};
    uv_queue_work(
        loop_, work, [](uv_work_t* work) {},
        [](uv_work_t* work, int uvstatus) {
            auto *entry = static_cast<UvEntry*>(work->data);
            if (entry->func) {
                entry->func();
            }
            std::unique_lock<std::mutex> lock(entry->mutex);
            entry->done = true;
            entry->condition.notify_all();
            if (entry->purge) {
                delete entry;
                delete work;
                entry = nullptr;
                work = nullptr;
            }
        });

    if (work == nullptr || work->data == nullptr) {
        HILOG_ERROR("%{public}s invalid work or work->data.", __func__);
        return;
    }

    auto *uvEntry = static_cast<UvEntry*>(work->data);
    {
        std::unique_lock<std::mutex> lock(uvEntry->mutex);
        while (!uvEntry->done) {
            HILOG_INFO("%{public}s Wait uv_queue_work to complete.", __func__);
            if (uvEntry->condition.wait_for(lock, std::chrono::seconds(WAIT_TIME_OUT)) == std::cv_status::timeout) {
                HILOG_INFO("%{public}s Wait uv_queue_work timeout.", __func__);
                uvEntry->isTimeout = true;
                break;
            }
        }
    }

    if (uvEntry->isTimeout && !uv_cancel((uv_req_t*)&work)) {
        HILOG_ERROR("%{public}s uv_cancel failed.", __func__);
        uvEntry->purge = true;
    } else {
        delete uvEntry;
        delete work;
    }
    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AbilityRuntime
} // namespace OHOS