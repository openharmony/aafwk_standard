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
DataShareUvQueue::DataShareUvQueue(napi_env env)
    : env_(env)
{
    napi_get_uv_event_loop(env, &loop_);
}

void DataShareUvQueue::CallFunction(NapiArgsGenerator genArgs)
{
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("no memory for uv_work_t");
        return;
    }
    work->data = new UvEntry{env_, std::move(genArgs)};
    uv_queue_work(
        loop_, work, [](uv_work_t* work) {},
        [](uv_work_t* work, int uvstatus) {
            auto *entry = static_cast<UvEntry*>(work->data);
            if (entry->args) {
                entry->args();
            }
            delete work;
            work = nullptr;
        });
}
} // namespace AbilityRuntime
} // namespace OHOS