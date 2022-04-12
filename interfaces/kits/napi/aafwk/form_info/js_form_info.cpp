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
#include "js_form_info.h"

#include "form_state_info.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsFormState(NativeEngine &engine)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("UNKNOWN", CreateJsValue(engine, AppExecFwk::FormState::UNKNOWN));
    object->SetProperty("DEFAULT", CreateJsValue(engine, AppExecFwk::FormState::DEFAULT));
    object->SetProperty("READY", CreateJsValue(engine, AppExecFwk::FormState::READY));
    return objValue;
}

NativeValue* FormInfoInit(NativeEngine *engine, NativeValue *exportObj)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("%{public}s engine or exportObj nullptr.", __func__);
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("%{public}s convertNativeValueTo result is nullptr.", __func__);
        return nullptr;
    }
    object->SetProperty("FormState", CreateJsFormState(*engine));

    HILOG_INFO("%{public}s called end.", __func__);
    return exportObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
