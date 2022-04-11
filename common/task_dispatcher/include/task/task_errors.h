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

#ifndef OHOS_TASK_ERRORS_H
#define OHOS_TASK_ERRORS_H

#include "errors.h"

namespace OHOS {
enum {
    ABILITY_MODULE_TASK = 0x08,
};

constexpr ErrCode AAFWK_TASK_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, ABILITY_MODULE_TASK);
enum {
    ERR_APPEXECFWK_CHECK_FAILED = AAFWK_TASK_ERR_OFFSET + 1,
    ERR_APPEXECFWK_INTERCEPT_TASK_EXECUTE_SUCCESS
};
}  // namespace OHOS

#endif  // OHOS_TASK_ERRORS_H