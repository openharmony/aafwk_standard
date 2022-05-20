/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "app_context.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppContext::AppContext()
{}
AppContext::~AppContext()
{}

/**
 * @brief Obtains information about the current ability.
 * The returned information includes the class name, bundle name, and other information about the current ability.
 *
 * @return Returns the AbilityInfo object for the current ability.
 */
const std::shared_ptr<AbilityInfo> AppContext::GetAbilityInfo()
{
    return nullptr;
}

/**
 * @brief Starts a new ability.
 * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
 * to start a specific ability. The system locates the target ability from installed abilities based on the value
 * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
 *
 * @param want Indicates the Want containing information about the target ability to start.
 *
 * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
 * template is started. You can define the request code to identify the results returned by abilities. The value
 * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
 * template.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode AppContext::StartAbility(const AAFwk::Want &want, int requestCode)
{
    return ERR_INVALID_VALUE;
}

/**
 * @brief Starts a new ability with special ability start setting.
 *
 * @param want Indicates the Want containing information about the target ability to start.
 * @param requestCode Indicates the request code returned after the ability is started. You can define the request code
 * to identify the results returned by abilities. The value ranges from 0 to 65535.
 * @param abilityStartSetting Indicates the special start setting used in starting ability.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode AppContext::StartAbility(const Want &want, int requestCode,
    const AbilityStartSetting &abilityStartSetting)
{
    return ERR_INVALID_VALUE;
}

/**
 * @brief Destroys another ability you had previously started by calling Ability.startAbilityForResult
 * (ohos.aafwk.content.Want, int, ohos.aafwk.ability.startsetting.AbilityStartSetting) with the same requestCode passed.
 *
 * @param requestCode Indicates the request code passed for starting the ability.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode AppContext::TerminateAbility(int requestCode)
{
    return ERR_INVALID_VALUE;
}

/**
 * @brief Destroys the current ability.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode AppContext::TerminateAbility()
{
    return ERR_INVALID_VALUE;
}

/**
 * @brief
 * Destroys this Service ability if the number of times it has been started equals the number represented by the
 * given {@code startId}. This method is the same as calling {@link #terminateAbility} to destroy this Service
 * ability, except that this method helps you avoid destroying it if a client has requested a Service
 * ability startup in {@link ohos.aafwk.ability.Ability#onCommand} but you are unaware of it.
 *
 * @param startId Indicates the number of startup times of this Service ability passed to
 *                {@link ohos.aafwk.ability.Ability#onCommand}. The {@code startId} is
 *                incremented by 1 every time this ability is started. For example,
 *                if this ability has been started for six times, the value of {@code startId} is {@code 6}.
 *
 * @return Returns {@code true} if the {@code startId} matches the number of startup times
 *         and this Service ability will be destroyed; returns {@code false} otherwise.
 */
bool AppContext::TerminateAbilityResult(int startId)
{
    return false;
}

/**
 * @brief Obtains the bundle name of the ability that called the current ability.
 * You can use the obtained bundle name to check whether the calling ability is allowed to receive the data you will
 * send. If you did not use Ability.startAbilityForResult(ohos.aafwk.content.Want, int,
 * ohos.aafwk.ability.startsetting.AbilityStartSetting) to start the calling ability, null is returned.
 *
 * @return Returns the bundle name of the calling ability; returns null if no calling ability is available.
 */
std::string AppContext::GetCallingBundle()
{
    return "";
}

/**
 * @brief Connects the current ability to an ability
 *
 * @param want Indicates the want containing information about the ability to connect
 *
 * @param conn Indicates the callback object when the target ability is connected.
 *
 * @return True means success and false means failure
 */
bool AppContext::ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn)
{
    return false;
}

/**
 * @brief Disconnects the current ability from an ability
 *
 * @param conn Indicates the IAbilityConnection callback object passed by connectAbility after the connection
 *              is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode AppContext::DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn)
{
    return ERR_INVALID_VALUE;
}

/**
 * @brief Destroys another ability that uses the AbilityInfo.AbilityType.SERVICE template.
 * The current ability using either the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE
 * template can call this method to destroy another ability that uses the AbilityInfo.AbilityType.SERVICE
 * template. The current ability itself can be destroyed by calling the terminateAbility() method.
 *
 * @param want Indicates the Want containing information about the ability to destroy.
 *
 * @return Returns true if the ability is destroyed successfully; returns false otherwise.
 */
bool AppContext::StopAbility(const AAFwk::Want &want)
{
    return false;
}

sptr<IRemoteObject> AppContext::GetToken()
{
    return nullptr;
}

/**
 * @brief Starts multiple abilities.
 *
 * @param wants Indicates the Want containing information array about the target ability to start.
 */
void AppContext::StartAbilities(const std::vector<AAFwk::Want> &wants)
{}

/**
 * @brief Obtains the unique ID of the mission containing this ability.
 *
 * @return Returns the unique mission ID.
 */
int AppContext::GetMissionId()
{
    return -1;
}
}  // namespace AppExecFwk
}  // namespace OHOS
