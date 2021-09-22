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

#ifndef OHOS_AAFWK_ABILITY_CONFIG_H
#define OHOS_AAFWK_ABILITY_CONFIG_H

#include <string>

namespace OHOS {
namespace AAFwk {
namespace AbilityConfig {
const std::string NAME_ABILITY_MGR_SERVICE = "AbilityManagerService";
const std::string NAME_BUNDLE_MGR_SERVICE = "BundleMgrService";
const std::string SCHEME_DATA_ABILITY = "dataability";
const std::string SYSTEM_UI_BUNDLE_NAME = "com.ohos.systemui";
const std::string SYSTEM_UI_STATUS_BAR = "com.ohos.systemui.statusbar.MainAbility";
const std::string SYSTEM_UI_NAVIGATION_BAR = "com.ohos.systemui.navigationbar.MainAbility";
const std::string SYSTEM_DIALOG_NAME = "com.ohos.systemui.systemdialog.MainAbility";

const std::string SYSTEM_DIALOG_REQUEST_PERMISSIONS = "OHOS_RESULT_PERMISSIONS_LIST_YES";
const std::string SYSTEM_DIALOG_CALLER_BUNDLENAME = "OHOS_RESULT_CALLER_BUNDLERNAME";
const std::string SYSTEM_DIALOG_KEY = "OHOS_RESULT_PERMISSION_KEY";

const std::string DEVICE_MANAGER_BUNDLE_NAME = "com.ohos.devicemanagerui";
const std::string DEVICE_MANAGER_NAME = "com.ohos.devicemanagerui.MainAbility";

const std::string EVENT_SYSTEM_WINDOW_MODE_CHANGED = "common.event.SYSTEM_WINDOW_MODE_CHANGED";
const int EVENT_CODE_SYSTEM_WINDOW_MODE_CHANGED = 1;

const std::string MISSION_NAME_MARK_HEAD = "#";
const std::string MISSION_NAME_SEPARATOR = ":";

const std::string FLOATING_WINDOW_PERMISSION = "ohos.permission.SYSTEM_FLOAT_WINDOW";

}  // namespace AbilityConfig
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_CONFIG_H
