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
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "napi_form_ability.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

/**
 * @brief Constructor of JS form
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[in] info An opaque datatype that is passed to a callback function
 *
 * @return This is an opaque pointer that is used to represent a JavaScript value
 */
static napi_value JSFormConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    auto formObject = new NapiFormAbility();
    napi_wrap(
        env, thisVar, formObject,
        [](napi_env env, void* data, void* hint) {
            auto formObject = (NapiFormAbility*)data;
            if (formObject != nullptr) {
                delete formObject;
            }
        },
        nullptr, nullptr);

    return thisVar;
}

EXTERN_C_START

/**
 * @brief  For N-API modules registration
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[in] exports  An empty object via the exports parameter as a convenience
 *
 * @return The return value from Init is treated as the exports object for the module
 */
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("acquireForm", NAPI_AcquireForm),
        DECLARE_NAPI_FUNCTION("deleteForm", NAPI_DeleteForm),
        DECLARE_NAPI_FUNCTION("releaseForm", NAPI_ReleaseForm),
        DECLARE_NAPI_FUNCTION("requestForm", NAPI_RequestForm),
        DECLARE_NAPI_FUNCTION("setFormNextRefreshTime", NAPI_SetFormNextRefreshTime),
        DECLARE_NAPI_FUNCTION("updateForm", NAPI_UpdateForm),
        DECLARE_NAPI_FUNCTION("castTempForm", NAPI_CastTempForm),
        DECLARE_NAPI_FUNCTION("notifyVisibleForms", NAPI_NotifyVisibleForms),
        DECLARE_NAPI_FUNCTION("notifyInvisibleForms", NAPI_NotifyInvisibleForms),
        DECLARE_NAPI_FUNCTION("enableFormsUpdate", NAPI_EnableFormsUpdate),
        DECLARE_NAPI_FUNCTION("disableFormsUpdate", NAPI_DisableFormsUpdate),
        DECLARE_NAPI_FUNCTION("checkFMSReady", NAPI_CheckFMSReady),
        DECLARE_NAPI_FUNCTION("getAllFormsInfo", NAPI_GetAllFormsInfo),
        DECLARE_NAPI_FUNCTION("getFormsInfoByApp", NAPI_GetFormsInfoByApp),
        DECLARE_NAPI_FUNCTION("getFormsInfoByModule", NAPI_GetFormsInfoByModule),
    };

    const char* formClassName = "FormObject";
    napi_value result = nullptr;
    napi_define_class(env, 
                      formClassName, 
                      NAPI_AUTO_LENGTH, 
                      JSFormConstructor, 
                      nullptr, 
                      sizeof(properties) / sizeof(properties[0]), 
                      properties, 
                      &result);

    napi_set_named_property(env, exports, formClassName, result);

    return exports;
}

EXTERN_C_END

// Define a Node-API module. 
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "form",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

// Registers a Node-API module.
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}