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
#ifndef FOUNDATION_APPEXECFWK_OHOS_REQUEST_CALLBACK_H
#define FOUNDATION_APPEXECFWK_OHOS_REQUEST_CALLBACK_H
namespace OHOS {
namespace AppExecFwk {
class RequestCallback {
public:
    RequestCallback() = default;
    virtual ~RequestCallback() = default;
    /**
     * @brief Called when the Device+ control center is successfully connected.
     *
     * <p>If this callback is invoked for the {@link IContinuationRegisterManager#register} method,
     * the return value is the registration token assigned to the particular ability after it is successfully
     * registered with the Device+ control center.
     *
     * @param result Indicates the result returned if the connection is successful. {@code -1} indicates that
     * the connection fails.
     * @return none
     */
    virtual void OnResult(int result) = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_REQUEST_CALLBACK_H
