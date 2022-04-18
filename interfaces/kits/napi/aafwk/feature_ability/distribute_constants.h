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

#ifndef OHOS_AAFWK_DISTRIBUTE_CONSTANTS_H
#define OHOS_AAFWK_DISTRIBUTE_CONSTANTS_H

#include <string>

namespace OHOS {
namespace AppExecFwk {
class DistributeConstants {
public:
    static const std::string START_ABILITY_PARAMS_KEY;
    static const std::string START_ABILITY_URL_KEY;
    static const std::string DEEP_LINK_ACTION_NAME;

    static constexpr int DEVICE_TYPE_DEFAULT = 0;
    static constexpr int DEVICE_TYPE_LOCAL = 1;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DISTRIBUTE_CONSTANTS_H
