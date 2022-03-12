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

#ifndef FOUNDATION_AAFWK_STOP_USER_CALLBACK_PROXY_H
#define FOUNDATION_AAFWK_STOP_USER_CALLBACK_PROXY_H

#include <string>
#include "iremote_proxy.h"
#include "stop_user_callback.h"

namespace OHOS {
namespace AAFwk {
/**
 * interface for StopUserCallbackProxy.
 */
class StopUserCallbackProxy : public IRemoteProxy<IStopUserCallback> {
public:
    explicit StopUserCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IStopUserCallback>(impl)
    {}
    ~StopUserCallbackProxy() = default;

    /**
     * @brief OnStopUserDone.
     *
     * @param userId userId.
     * @param errcode errcode.
     */
    virtual void OnStopUserDone(int userId, int errcode) override;
private:
    void SendRequestCommon(int userId, int errcode, IStopUserCallback::StopUserCallbackCmd cmd);

private:
    static inline BrokerDelegator<StopUserCallbackProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_STOP_USER_CALLBACK_PROXY_H