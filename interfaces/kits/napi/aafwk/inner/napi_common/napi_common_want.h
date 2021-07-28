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

#ifndef OHOS_APPEXECFWK_NAPI_COMMON_WANT_H
#define OHOS_APPEXECFWK_NAPI_COMMON_WANT_H

#include <vector>
#include <map>
#include <string>
#include "want.h"
#include "napi_common_data.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {

napi_value WrapElementName(napi_env env, const ElementName &elementName);
bool UnwrapElementName(napi_env env, napi_value param, ElementName &elementName);

napi_value WrapWantParams(napi_env env, const AAFwk::WantParams &wantParams);
bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams);

napi_value WrapWant(napi_env env, const Want &want);
bool UnwrapWant(napi_env env, napi_value param, Want &want);

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APPEXECFWK_NAPI_COMMON_WANT_H
