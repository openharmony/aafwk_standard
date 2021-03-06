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

#ifndef ABILITY_RUNTIME_LOCAL_CALL_CONTAINER_H
#define ABILITY_RUNTIME_LOCAL_CALL_CONTAINER_H

#include "ability_connect_callback_stub.h"
#include "ability_connect_callback_proxy.h"
#include "local_call_record.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
using Want = OHOS::AAFwk::Want;
using AbilityConnectionStub = OHOS::AAFwk::AbilityConnectionStub;
class LocalCallContainer : public AbilityConnectionStub {
public:
    LocalCallContainer() = default;
    virtual ~LocalCallContainer() = default;

    int StartAbilityInner(
        const Want &want, const std::shared_ptr<CallerCallBack> &callback, const sptr<IRemoteObject> &callerToken);

    int Release(const std::shared_ptr<CallerCallBack> &callback);

    void DumpCalls(std::vector<std::string> &info) const;

    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

private:
    bool GetCallLocalreocrd(
        const AppExecFwk::ElementName &elementName, std::shared_ptr<LocalCallRecord> &localCallRecord);
    void OnCallStubDied(const wptr<IRemoteObject> &remote);

private:
    std::map<std::string, std::shared_ptr<LocalCallRecord>> callProxyRecords_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_LOCAL_CALL_CONTAINER_H
