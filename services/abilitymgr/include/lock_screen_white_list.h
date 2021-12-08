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

#ifndef OHOS_AAFWK_LOCK_SCREEN_WHITE_LIST_H
#define OHOS_AAFWK_LOCK_SCREEN_WHITE_LIST_H

#include <fstream>
#include <nlohmann/json.hpp>

namespace OHOS {
namespace AAFwk {
namespace AmsWhiteList {
const std::string WHITE_LIST_FILE_PATH {"/data/white_list/lock_screen_white_list.json"};
const std::string WHITE_LIST_DIR_PATH {"/data/white_list"};
const std::string ISAWAKEN_SCREEN {"isawaken_screen"};
constexpr int DUMP_INDENT = 4;
}  // namespace AmsWhiteList

class LockScreenWhiteList {
public:
    LockScreenWhiteList() = default;
    virtual ~LockScreenWhiteList() = default;

    bool SetWhiteListInfo(const std::string &bundleName, bool isAllow);
    bool GetWhiteListInfo(nlohmann::json &jsonFile);
    bool FindBundleNameOnWhiteList(const std::string &bundleName, bool &isAwakenScreen);

private:
    bool IsExistFile(const std::string &path);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_LOCK_SCREEN_WHITE_LIST_H