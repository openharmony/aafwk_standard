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

#ifndef ABILITY_RUNTIME_ABILITY_CONNECT_CALLBACK_H
#define ABILITY_RUNTIME_ABILITY_CONNECT_CALLBACK_H

#include "element_name.h"

#include "iremote_broker.h"
#include "refbase.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class AbilityConnectCallback
 * AbilityConnectCallback is used to notify caller ability that connect or disconnect is complete.
 */
class AbilityConnectCallback : public RefBase{
public:
    /**
     * @brief Constructor.
     *
     */
    AbilityConnectCallback() = default;

    /**
     * @brief Destructor.
     *
     */
    virtual ~AbilityConnectCallback() = default;
    /**
     * OnConnect, AbilityMs notify caller ability the result of connect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
                           const sptr<IRemoteObject> &remoteObject, int resultCode) = 0;

    /**
     * OnDisconnect, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) = 0;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_ABILITY_CONNECT_CALLBACK_H