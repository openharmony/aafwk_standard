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
var Callee = requireNapi("application.Callee")

class Ability {
    constructor() {
        this.callee = new Callee("rpc.application.callee");
        console.log("Ability::constructor callee is " + typeof this.callee + " " + this.callee);
    }
    onCreate(want) {}
    onDestroy() {}
    onWindowStageCreate(windowStage) {}
    onWindowStageDestroy() {}
    onForeground(want) {}
    onBackground() {}
    onWindowStageRestore(windowStage) {}
    onCallRequest() {
        console.log("Ability::onCallRequest callee is " + typeof this.callee + " " + this.callee);
        return this.callee;
    }
    onContinue(wantParams) {}
    onConfigurationUpdated(config) {}
    onNewWant(want) {}
}

export default Ability