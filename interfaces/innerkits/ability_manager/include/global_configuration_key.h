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

#ifndef OHOS_AAFWK_INTERFACES_INNERKITS_GLOBAL_CONFIGURATION_KEY_H
#define OHOS_AAFWK_INTERFACES_INNERKITS_GLOBAL_CONFIGURATION_KEY_H

#include <string>

namespace OHOS {
namespace AAFwk {
namespace GlobalConfigurationKey {
    /* For the time being, there is no uniform standard */
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!@ */
    /* Must be synchronized with the keystore(SystemConfigurationKeyStore)in the configuration */
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    static const std::string SYSTEM_LANGUAGE {"ohos.system.language"};
    static const std::string SYSTEM_COLORMODE {"ohos.system.colorMode"};
} // namespace GlobalConfigurationKey
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_AAFWK_INTERFACES_INNERKITS_GLOBAL_CONFIGURATION_KEY_H