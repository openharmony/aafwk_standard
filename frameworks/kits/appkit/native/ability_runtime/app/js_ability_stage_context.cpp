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

#include "js_ability_stage_context.h"

#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
void JsAbilityStageContext::ConfigurationUpdated(NativeEngine* engine, std::shared_ptr<NativeReference> &jsContext,
    const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    HILOG_INFO("%{public}s called.", __func__);
    if ((!jsContext) || (!config)) {
        HILOG_INFO("jsContext or config is nullptr.");
        return;
    }

    NativeValue* value = jsContext->Get();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(value);
    if (!object) {
        HILOG_INFO("object is nullptr.");
        return;
    }

    NativeValue* method = object->GetProperty("onUpdateConfiguration");
    if (!method) {
        HILOG_ERROR("Failed to get onUpdateConfiguration from object");
        return;
    }

    HILOG_INFO("JsAbilityStageContext call onUpdateConfiguration.");
    NativeValue* argv[] = {CreateJsConfiguration(*engine, *config)};
    engine->CallFunction(value, method, argv, 1);
}

NativeValue* CreateJsAbilityStageContext(NativeEngine& engine, std::shared_ptr<AbilityRuntime::Context> context)
{
    HILOG_INFO("%{public}s called.", __func__);
    NativeValue* objValue = CreateJsBaseContext(engine, context);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    auto configuration = context->GetConfiguration();
    if (configuration) {
        object->SetProperty("config", CreateJsConfiguration(engine, *configuration));
    }

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS