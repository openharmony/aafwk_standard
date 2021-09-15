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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_POST_EVENT_TIMEOUT_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_POST_EVENT_TIMEOUT_H

#include <memory>
#include <atomic>
#include <mutex>
#include <string>

namespace OHOS {
namespace AppExecFwk {

class AbilityHandler;
class AbilityPostEventTimeout : public std::enable_shared_from_this<AbilityPostEventTimeout> {
private:
    // default delaytime is 5000ms
    static const int64_t defalutDelayTime;

public:
    AbilityPostEventTimeout(std::string str, std::shared_ptr<AbilityHandler> &eventHandler);
    ~AbilityPostEventTimeout();

    void TimingBegin(int64_t delaytime = defalutDelayTime);
    void TimeEnd();

protected:
    void TimeOutProc();

private:
    std::string task_;
    std::shared_ptr<AbilityHandler> handler_;
    std::mutex mtx_;
    bool taskExec_;

    static std::atomic<uint32_t> allocationId_;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_POST_EVENT_TIMEOUT_H