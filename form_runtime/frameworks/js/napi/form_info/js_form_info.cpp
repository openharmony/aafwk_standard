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
#include "js_form_info.h"

#include "form_constants.h"
#include "form_info.h"
#include "form_info_base.h"
#include "form_state_info.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsFormType(NativeEngine &engine)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("JS", CreateJsValue(engine, AppExecFwk::FormType::JS));
    return objValue;
}

NativeValue* CreateJsColorMode(NativeEngine &engine)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("MODE_AUTO", CreateJsValue(engine, AppExecFwk::FormsColorMode::AUTO_MODE));
    object->SetProperty("MODE_DARK", CreateJsValue(engine, AppExecFwk::FormsColorMode::DARK_MODE));
    object->SetProperty("MODE_LIGHT", CreateJsValue(engine, AppExecFwk::FormsColorMode::LIGHT_MODE));
    return objValue;
}

NativeValue* CreateJsFormState(NativeEngine &engine)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("UNKNOWN", CreateJsValue(engine, AppExecFwk::FormState::UNKNOWN));
    object->SetProperty("DEFAULT", CreateJsValue(engine, AppExecFwk::FormState::DEFAULT));
    object->SetProperty("READY", CreateJsValue(engine, AppExecFwk::FormState::READY));
    return objValue;
}

NativeValue* CreateJsFormParam(NativeEngine &engine)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("IDENTITY_KEY", CreateJsValue(engine, AppExecFwk::Constants::PARAM_FORM_IDENTITY_KEY));
    object->SetProperty("DIMENSION_KEY", CreateJsValue(engine, AppExecFwk::Constants::PARAM_FORM_DIMENSION_KEY));
    object->SetProperty("NAME_KEY", CreateJsValue(engine, AppExecFwk::Constants::PARAM_FORM_NAME_KEY));
    object->SetProperty("MODULE_NAME_KEY", CreateJsValue(engine, AppExecFwk::Constants::PARAM_MODULE_NAME_KEY));
    object->SetProperty("WIDTH_KEY", CreateJsValue(engine, AppExecFwk::Constants::PARAM_FORM_WIDTH_KEY));
    object->SetProperty("HEIGHT_KEY", CreateJsValue(engine, AppExecFwk::Constants::PARAM_FORM_HEIGHT_KEY));
    object->SetProperty("TEMPORARY_KEY", CreateJsValue(engine, AppExecFwk::Constants::PARAM_FORM_TEMPORARY_KEY));
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
    object->SetProperty("FormType", CreateJsFormType(*engine));
    object->SetProperty("ColorMode", CreateJsColorMode(*engine));
    object->SetProperty("FormState", CreateJsFormState(*engine));
    object->SetProperty("FormParam", CreateJsFormParam(*engine));

    HILOG_INFO("%{public}s called end.", __func__);
    return exportObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
