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

var Context = requireNapi("application.Context")
var Caller = requireNapi("application.Caller")

class AbilityContext extends Context {
    constructor(obj) {
        super(obj)
        this.abilityInfo = obj.abilityInfo
        this.currentHapModuleInfo = obj.currentHapModuleInfo
        this.config = obj.config
    }

    onUpdateConfiguration(config) {
        this.config = config
    }

    startAbility(want, options, callback) {
        return this.__context_impl__.startAbility(want, options, callback)
    }

    startAbilityWithAccount(want, accountId, options, callback) {
        return this.__context_impl__.startAbilityWithAccount(want, accountId, options, callback)
    }

    startAbilityByCall(want) {
        return new Promise(async (resolve, reject) => {
            if (typeof want !== 'object' || want == null) {
                console.log("AbilityContext::startAbilityByCall input param error");
                reject(new Error("function input parameter error"));
                return;
            }

            let callee = await this.__context_impl__.startAbilityByCall(want);
            if (callee == null || typeof callee !== 'object') {
                console.log("AbilityContext::startAbilityByCall Obtain remoteObject failed");
                reject(new Error("function request remote error"));
                return;
            }

            resolve(new Caller(callee));
            console.log("AbilityContext::startAbilityByCall success");
            return;
        });
    }

    startAbilityForResult(want, options, callback) {
        return this.__context_impl__.startAbilityForResult(want, options, callback)
    }

    startAbilityForResultWithAccount(want, accountId, options, callback) {
        return this.__context_impl__.startAbilityForResultWithAccount(want, accountId, options, callback)
    }

    startServiceExtensionAbility(want, callback) {
        return this.__context_impl__.startServiceExtensionAbility(want, callback)
    }

    startServiceExtensionAbilityWithAccount(want, accountId, callback) {
        return this.__context_impl__.startServiceExtensionAbilityWithAccount(want, accountId, callback)
    }

    stopServiceExtensionAbility(want, callback) {
        return this.__context_impl__.stopServiceExtensionAbility(want, callback)
    }

    stopServiceExtensionAbilityWithAccount(want, accountId, callback) {
        return this.__context_impl__.stopServiceExtensionAbilityWithAccount(want, accountId, callback)
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

    isTerminating() {
        return this.__context_impl__.isTerminating()
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

    setMissionLabel(label, callback) {
        return this.__context_impl__.setMissionLabel(label, callback)
    }

    setMissionIcon(icon, callback) {
        return this.__context_impl__.setMissionIcon(icon, callback)
    }
}

export default AbilityContext