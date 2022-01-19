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

#ifndef ABILITY_RUNTIME_ABILITY_CONTEXT_H
#define ABILITY_RUNTIME_ABILITY_CONTEXT_H

#include "foundation/aafwk/standard/frameworks/kits/appkit/native/ability_runtime/context/context.h"

#include "ability_connect_callback.h"
#include "ability_info.h"
#include "native_engine/native_value.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
using RuntimeTask = std::function<void(int, const AAFwk::Want&)>;
class AbilityContext : public Context {
public:
    virtual ~AbilityContext() = default;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
     * template is started. You can define the request code to identify the results returned by abilities. The value
     * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
     * template.
     */
    virtual ErrCode StartAbility(const AAFwk::Want &want, int requestCode) = 0;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and the value of the start options and then starts it. You can specify the ability to
     * start using the two parameters.
     *
     * @param want Indicates the Want containing application side information about the target ability to start.
     * @param startOptions Indicates the StartOptions containing service side information about the target ability to
     * start.
     * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
     * template is started. You can define the request code to identify the results returned by abilities. The value
     * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
     * template.
     */
    virtual ErrCode StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode) = 0;

    virtual ErrCode StartAbilityForResult(const AAFwk::Want &Want, int requestCode, RuntimeTask &&task) = 0;

    virtual ErrCode StartAbilityForResult(const AAFwk::Want &Want, const AAFwk::StartOptions &startOptions,
        int requestCode, RuntimeTask &&task) = 0;

    virtual ErrCode TerminateAbilityWithResult(const AAFwk::Want &want, int resultCode) = 0;

    virtual ErrCode RestoreWindowStage(void* contentStorage) = 0;

    virtual void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData) = 0;

    /**
    * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
    *
    * @param want Indicates the want containing information about the ability to connect
    * @param connectCallback Indicates the callback object when the target ability is connected.
    * @return True means success and false means failure
    */
    virtual bool ConnectAbility(const AAFwk::Want &want,
        const sptr<AbilityConnectCallback> &connectCallback) = 0;

    /**
    * @brief Disconnects the current ability from an ability
    *
    * @param want Indicates the want containing information about the ability to disconnect
    * @param connectCallback Indicates the callback object when the target ability is connected.
    * is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
    */
    virtual void DisconnectAbility(const AAFwk::Want &want,
        const sptr<AbilityConnectCallback> &connectCallback) = 0;

    /**
     * @brief get ability info of the current ability
     *
     * @return the ability info of the current ability
     */
    virtual std::shared_ptr<AppExecFwk::AbilityInfo> GetAbilityInfo() const = 0;

    /**
     * @brief Minimize the current ability.
     *
     */
    virtual void MinimizeAbility() = 0;

    virtual ErrCode TerminateSelf() = 0;

     /**
     * @brief Obtains token.
     *
     * @return Returns the token.
     */
    virtual sptr<IRemoteObject> GetAbilityToken() = 0;

    /**
     * @brief Requests certain permissions from the system.
     * This method is called for permission request. This is an asynchronous method. When it is executed,
     * the Ability.onRequestPermissionsFromUserResult(int, String[], int[]) method will be called back.
     *
     * @param permissions Indicates the list of permissions to be requested. This parameter cannot be null.
     * @param requestCode Indicates the request code to be passed to the Ability.onRequestPermissionsFromUserResult(int,
     * String[], int[]) callback method. This code cannot be a negative number.
     */
    virtual void RequestPermissionsFromUser(const std::vector<std::string> &permissions, int requestCode) = 0;

    /**
     * @brief Get ContentStorage.
     *
     * @return Returns the ContentStorage.
     */
    virtual void* GetContentStorage() = 0;

    using SelfType = AbilityContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || Context::IsContext(contextTypeId);
    }
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_ABILITY_CONTEXT_H