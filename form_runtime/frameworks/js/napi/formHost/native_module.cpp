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
#include "napi/native_api.h"

#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#include "hilog_wrapper.h"
#include "napi/native_node_api.h"
#include "napi_form_host.h"

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
    HILOG_INFO("napi_moudule Init start...");
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("deleteForm", NAPI_DeleteForm),
        DECLARE_NAPI_FUNCTION("releaseForm", NAPI_ReleaseForm),
        DECLARE_NAPI_FUNCTION("requestForm", NAPI_RequestForm),
        DECLARE_NAPI_FUNCTION("castTempForm", NAPI_CastTempForm),
        DECLARE_NAPI_FUNCTION("notifyVisibleForms", NAPI_NotifyVisibleForms),
        DECLARE_NAPI_FUNCTION("notifyInvisibleForms", NAPI_NotifyInvisibleForms),
        DECLARE_NAPI_FUNCTION("enableFormsUpdate", NAPI_EnableFormsUpdate),
        DECLARE_NAPI_FUNCTION("disableFormsUpdate", NAPI_DisableFormsUpdate),
        DECLARE_NAPI_FUNCTION("isSystemReady", NAPI_CheckFMSReady),
        DECLARE_NAPI_FUNCTION("deleteInvalidForms", NAPI_DeleteInvalidForms),
        DECLARE_NAPI_FUNCTION("acquireFormState", NAPI_AcquireFormState),
        DECLARE_NAPI_FUNCTION("on", NAPI_RegisterFormUninstallObserver),
        DECLARE_NAPI_FUNCTION("off", NAPI_UnregisterFormUninstallObserver),
        DECLARE_NAPI_FUNCTION("notifyFormsVisible", NAPI_NotifyFormsVisible),
        DECLARE_NAPI_FUNCTION("notifyFormsEnableUpdate", NAPI_NotifyFormsEnableUpdate),
        DECLARE_NAPI_FUNCTION("getAllFormsInfo", NAPI_GetAllFormsInfo),
        DECLARE_NAPI_FUNCTION("getFormsInfo", NAPI_GetFormsInfo),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));
    HILOG_INFO("napi_moudule Init end...");

    return exports;
}

EXTERN_C_END

// Define a Node-API module.
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "application.formHost",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

// Registers a Node-API module.
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}