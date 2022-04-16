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
    /**
     * A constructor used to create a TestObserverProxy instance with the input parameter object passed.
     * @param object Indicates remote object.
     */
    explicit TestObserverProxy(const sptr<IRemoteObject> &object);

    /**
     * Deconstructor used to deconstruct.
     */
    virtual ~TestObserverProxy() override;

    /**
     * Outputs test status.
     *
     * @param msg, Indicates the status information.
     * @param resultCode, Indicates the result code.
     */
    virtual void TestStatus(const std::string &msg, const int64_t &resultCode) override;

    /**
     * Outputs information and result code that the test has finished.
     *
     * @param msg, Indicates the status information.
     * @param resultCode, Indicates the result code.
     */
    virtual void TestFinished(const std::string &msg, const int64_t &resultCode) override;

    /**
     * Executes the specified shell command.
     *
     * @param cmd, Indicates the specified shell command.
     * @param timeoutSec, Indicates the specified time out time, in seconds.
     * @return the result of the specified shell command.
     */
    virtual ShellCommandResult ExecuteShellCommand(const std::string &cmd, const int64_t timeoutSec) override;

private:
    static inline BrokerDelegator<TestObserverProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_TEST_OBSERVER_PROXY_H