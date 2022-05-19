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

#ifndef FORM_ABILITY_H_
#define FORM_ABILITY_H_

#include "ability.h"
#include "appexecfwk_errors.h"
#include "form_info.h"
#include "form_js_info.h"
#include "form_provider_info.h"
#include "form_state_info.h"
#include "hilog_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "nlohmann/json.hpp"
#include "want.h"

struct AsyncDelFormCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    int64_t formId;
    int result;
};

struct AsyncReleaseFormCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    int64_t formId;
    bool isReleaseCache;
    int result;
};

struct AsyncRequestFormCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    int64_t formId;
    int result;
};

struct AsyncNextRefreshTimeFormCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    int64_t formId;
    int32_t time;
    int result;
};

struct AsyncUpdateFormCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    int64_t formId;
    std::shared_ptr<OHOS::AppExecFwk::FormProviderData> formProviderData;
    int result;
};

struct AsyncCastTempFormCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    int64_t formId;
    int result;
};

struct AsyncNotifyVisibleFormsCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::vector<int64_t> formIds;
    int result;
};

struct AsyncNotifyInvisibleFormsCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::vector<int64_t> formIds;
    int result;
};

struct AsyncEnableUpdateFormCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::vector<int64_t> formIds;
    int result;
};

struct AsyncDisableUpdateFormCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::vector<int64_t> formIds;
    int result;
};

struct AsyncCheckFMSReadyCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    bool isFMSReady;
};

struct AsyncDeleteInvalidFormsCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::vector<int64_t> formIds;
    int numFormsDeleted;
    int result;
};

struct AsyncAcquireFormStateCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    OHOS::AAFwk::Want want;
    OHOS::AppExecFwk::FormStateInfo stateInfo;
    int result;
};

struct AsyncFormUninstallObserverCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    int result;
};

struct AsyncNotifyFormsVisibleCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::vector<int64_t> formIds;
    bool isVisible;
    int result;
};

struct AsyncNotifyFormsEnableUpdateCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::vector<int64_t> formIds;
    bool isEnableUpdate;
    int result;
};

struct AsyncGetFormsInfoCallbackInfo {
    napi_env env;
    OHOS::AppExecFwk::Ability *ability;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::vector<OHOS::AppExecFwk::FormInfo> formInfos;
    std::string bundleName;
    std::string moduleName;
    int result;
};

struct AsyncErrMsgCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    napi_value callbackValue;
    int code;
    int type;
};

napi_value NAPI_DeleteForm(napi_env env, napi_callback_info info);
napi_value NAPI_ReleaseForm(napi_env env, napi_callback_info info);
napi_value NAPI_RequestForm(napi_env env, napi_callback_info info);
napi_value NAPI_SetFormNextRefreshTime(napi_env env, napi_callback_info info);
napi_value NAPI_UpdateForm(napi_env env, napi_callback_info info);
napi_value NAPI_CastTempForm(napi_env env, napi_callback_info info);
napi_value NAPI_NotifyVisibleForms(napi_env env, napi_callback_info info);
napi_value NAPI_NotifyInvisibleForms(napi_env env, napi_callback_info info);
napi_value NAPI_EnableFormsUpdate(napi_env env, napi_callback_info info);
napi_value NAPI_DisableFormsUpdate(napi_env env, napi_callback_info info);
napi_value NAPI_CheckFMSReady(napi_env env, napi_callback_info info);
napi_value NAPI_DeleteInvalidForms(napi_env env, napi_callback_info info);
napi_value NAPI_AcquireFormState(napi_env env, napi_callback_info info);
napi_value NAPI_RegisterFormUninstallObserver(napi_env env, napi_callback_info info);
napi_value NAPI_UnregisterFormUninstallObserver(napi_env env, napi_callback_info info);
napi_value NAPI_NotifyFormsVisible(napi_env env, napi_callback_info info);
napi_value NAPI_NotifyFormsEnableUpdate(napi_env env, napi_callback_info info);
napi_value NAPI_GetAllFormsInfo(napi_env env, napi_callback_info info);
napi_value NAPI_GetFormsInfo(napi_env env, napi_callback_info info);

#endif /* FORM_ABILITY_H_ */
