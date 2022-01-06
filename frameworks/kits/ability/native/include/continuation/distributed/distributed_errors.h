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

#ifndef FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_ERRORS_H
#define FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_ERRORS_H

#include "errors.h"

namespace OHOS {
namespace AppExecFwk {
enum DistributedModule {
    DISTRIBUTED_MODULE_SERVICE = 0x00,
};

// time error offset, used only in this file.
constexpr ErrCode AAFWK_DISTRIBUTED_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_AAFWK, DISTRIBUTED_MODULE_SERVICE);

enum {
    // 2097152
    GET_SYSTEM_ABILITY_SERVICE_FAILED = AAFWK_DISTRIBUTED_SERVICE_ERR_OFFSET,
    // 2097153
    GET_DISTRIBUTED_ABILITY_SERVICE_FAILED,
    // 2097154
    DISTRIBUTED_ABILITY_SERVICE_NOT_CONNECTED,
    // 2097155
    INCOMING_PARAMETER_POINTER_IS_NULL,
    /**
     * Result(2097156) for StartAbility: An error of the Want could not be resolved
     * to app info from BMS or DistributedMS.
     */
    ABILITY_FAILED_RESTORE_DATA,

};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // SERVICES_INCLUDE_TIME_COMMON_H