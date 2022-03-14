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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_TEST_OBSERVER_PROXY_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_TEST_OBSERVER_PROXY_H

#include "itest_observer.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AAFwk {
class TestObserverProxy : public IRemoteProxy<ITestObserver> {
public:
    explicit TestObserverProxy(const sptr<IRemoteObject> &object);
    virtual ~TestObserverProxy() override;
    virtual void TestStatus(const std::string &msg, const int &resultCode) override;
    virtual void TestFinished(const std::string &msg, const int &resultCode) override;
    virtual ShellCommandResult ExecuteShellCommand(
        const std::string &cmd, const int64_t timeoutSec) override;

private:
    static inline BrokerDelegator<TestObserverProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_TEST_OBSERVER_PROXY_H