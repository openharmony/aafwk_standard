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

#ifndef OHOS_APPEXECFWK_NAPI_COMMON_CONFIGURATION_H
#define OHOS_APPEXECFWK_NAPI_COMMON_CONFIGURATION_H

#include "napi_common_data.h"
#include "configuration.h"

namespace OHOS {
namespace AppExecFwk {
const std::string DOT_STRING {"."};
EXTERN_C_START

napi_value WrapConfiguration(napi_env env, const AppExecFwk::Configuration &configuration);
bool UnwrapConfiguration(napi_env env, napi_value param, Configuration &config);

EXTERN_C_END
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APPEXECFWK_NAPI_COMMON_CONFIGURATION_H
