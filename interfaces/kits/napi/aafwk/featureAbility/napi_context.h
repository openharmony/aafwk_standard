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

#ifndef OHOS_APPEXECFWK_NAPI_CONTEXT_H
#define OHOS_APPEXECFWK_NAPI_CONTEXT_H
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "ability.h"
#include "../inner/napi_common/napi_common.h"

using Ability = OHOS::AppExecFwk::Ability;

namespace OHOS {
namespace AppExecFwk {

struct CallAbilityPermissionParam {
    std::vector<std::string> permission_list;
    int requestCode = 0;
    int syncOption = false;
};

struct OnRequestPermissionsFromUserResultCallback {
    int requestCode = 0;
    std::vector<std::string> permissions;
    std::vector<int> grantResults;
    CallbackInfo cb;
};

/**
 * @brief Context NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value ContextPermissionInit(napi_env env, napi_value exports);

/**
 * @brief The interface of onRequestPermissionsFromUserResult provided for ACE to call back to JS.
 *
 * @param requestCode Indicates the request code returned after the ability is started.
 * @param permissions Indicates list of permission.
 * @param grantResults Indicates List of authorization results.
 * @param callbackInfo The environment and call back info that the Node-API call is invoked under.
 */
void CallOnRequestPermissionsFromUserResult(int requestCode, const std::vector<std::string> &permissions,
    const std::vector<int> &grantResults, OHOS::AppExecFwk::CallbackInfo callbackInfo);

napi_value ContextConstructor(napi_env env, napi_callback_info info);

}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_NAPI_CONTEXT_H */
