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

var Context = requireNapi("application.Context")
var Caller = requireNapi("application.Caller")

class AbilityContext extends Context {
    constructor(obj) {
        super(obj)
        this.abilityInfo = obj.abilityInfo
        this.currentHapModuleInfo = obj.currentHapModuleInfo
        this.configuration = obj.configuration
    }

    onUpdateConfiguration(config) {
        this.configuration = config
    }

    startAbility(want, options, callback) {
        return this.__context_impl__.startAbility(want, options, callback)
    }

    startAbilityWithAccount(want, accountId, options, callback) {
        return this.__context_impl__.startAbilityWithAccount(want, accountId, options, callback)
    }

    async startAbilityByCall(want) {
        if (typeof want !== 'object' || want == null) {
            console.log("AbilityContext::startAbilityByCall input param error");
            return null;
        }

        let callee = await this.__context_impl__.startAbilityByCall(want);
        if (typeof callee === 'object' && callee != null) {
            console.log("AbilityContext::startAbilityByCall");
            return new Caller(callee);
         } else {
            console.log("AbilityContext::startAbilityByCall Obtain remoteObject falied");
            return null;
        }
    }

    startAbilityForResult(want, options, callback) {
        return this.__context_impl__.startAbilityForResult(want, options, callback)
    }

    startAbilityForResultWithAccount(want, accountId, options, callback) {
        return this.__context_impl__.startAbilityForResultWithAccount(want, accountId, options, callback)
    }

    connectAbility(want, options) {
        return this.__context_impl__.connectAbility(want, options);
    }

    connectAbilityWithAccount(want, accountId, options) {
        return this.__context_impl__.connectAbilityWithAccount(want, accountId, options);
    }

    disconnectAbility(connection, callback) {
        return this.__context_impl__.disconnectAbility(connection, callback);
    }

    terminateSelf(callback) {
        return this.__context_impl__.terminateSelf(callback)
    }

    terminateSelfWithResult(abilityResult, callback) {
        return this.__context_impl__.terminateSelfWithResult(abilityResult, callback)
    }

    requestPermissionsFromUser(permissions, resultCallback) {
        return this.__context_impl__.requestPermissionsFromUser(permissions, resultCallback)
    }

    restoreWindowStage(contentStorage) {
        return this.__context_impl__.restoreWindowStage(contentStorage)
    }
}

export default AbilityContext