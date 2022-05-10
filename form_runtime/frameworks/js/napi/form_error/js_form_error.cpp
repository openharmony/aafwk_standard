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
#include "js_form_error.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "napi_form_util.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsFormError(NativeEngine &engine)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("ERR_COMMON", CreateJsValue(engine, ERR_COMMON));
    object->SetProperty("ERR_PERMISSION_DENY", CreateJsValue(engine, ERR_PERMISSION_DENY));
    object->SetProperty("ERR_GET_INFO_FAILED", CreateJsValue(engine, ERR_GET_INFO_FAILED));
    object->SetProperty("ERR_GET_BUNDLE_FAILED", CreateJsValue(engine, ERR_GET_BUNDLE_FAILED));
    object->SetProperty("ERR_GET_LAYOUT_FAILED", CreateJsValue(engine, ERR_GET_LAYOUT_FAILED));
    object->SetProperty("ERR_ADD_INVALID_PARAM", CreateJsValue(engine, ERR_ADD_INVALID_PARAM));
    object->SetProperty("ERR_CFG_NOT_MATCH_ID", CreateJsValue(engine, ERR_CFG_NOT_MATCH_ID));
    object->SetProperty("ERR_NOT_EXIST_ID", CreateJsValue(engine, ERR_NOT_EXIST_ID));
    object->SetProperty("ERR_BIND_PROVIDER_FAILED", CreateJsValue(engine, ERR_BIND_PROVIDER_FAILED));
    object->SetProperty("ERR_MAX_SYSTEM_FORMS", CreateJsValue(engine, ERR_MAX_SYSTEM_FORMS));
    object->SetProperty("ERR_MAX_INSTANCES_PER_FORM", CreateJsValue(engine, ERR_MAX_INSTANCES_PER_FORM));
    object->SetProperty("ERR_OPERATION_FORM_NOT_SELF", CreateJsValue(engine, ERR_OPERATION_FORM_NOT_SELF));
    object->SetProperty("ERR_PROVIDER_DEL_FAIL", CreateJsValue(engine, ERR_PROVIDER_DEL_FAIL));
    object->SetProperty("ERR_MAX_FORMS_PER_CLIENT", CreateJsValue(engine, ERR_MAX_FORMS_PER_CLIENT));
    object->SetProperty("ERR_MAX_SYSTEM_TEMP_FORMS", CreateJsValue(engine, ERR_MAX_SYSTEM_TEMP_FORMS));
    object->SetProperty("ERR_FORM_NO_SUCH_MODULE", CreateJsValue(engine, ERR_FORM_NO_SUCH_MODULE));
    object->SetProperty("ERR_FORM_NO_SUCH_ABILITY", CreateJsValue(engine, ERR_FORM_NO_SUCH_ABILITY));
    object->SetProperty("ERR_FORM_NO_SUCH_DIMENSION", CreateJsValue(engine, ERR_FORM_NO_SUCH_DIMENSION));
    object->SetProperty("ERR_FORM_FA_NOT_INSTALLED", CreateJsValue(engine, ERR_FORM_FA_NOT_INSTALLED));
    object->SetProperty("ERR_SYSTEM_RESPONSES_FAILED", CreateJsValue(engine, ERR_SYSTEM_RESPONSES_FAILED));
    object->SetProperty("ERR_FORM_DUPLICATE_ADDED", CreateJsValue(engine, ERR_FORM_DUPLICATE_ADDED));
    object->SetProperty("ERR_IN_RECOVERY", CreateJsValue(engine, ERR_IN_RECOVERY));
    return objValue;
}

NativeValue* FormErrorInit(NativeEngine *engine, NativeValue *exportObj)
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
    object->SetProperty("FormError", CreateJsFormError(*engine));

    HILOG_INFO("%{public}s called end.", __func__);
    return exportObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
