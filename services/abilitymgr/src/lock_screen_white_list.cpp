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

#include "lock_screen_white_list.h"
#include <sys/stat.h>
#include <sys/types.h>

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
using json = nlohmann::json;

bool LockScreenWhiteList::SetWhiteListInfo(const std::string &bundleName, bool isAllow)
{
    HILOG_INFO("bundleName %{public}s, isAllow %{public}d",
        bundleName.c_str(),
        static_cast<int>(isAllow));
    nlohmann::json jsonFile;
    std::ifstream inFile;
    inFile.open(AmsWhiteList::AMS_WHITE_LIST_FILE_PATH, std::ios::in);
    if (!inFile.is_open()) {
        HILOG_INFO("no such file...");
        return false;
    } else {
        inFile.seekg(0, std::ios::end);
        int len = static_cast<int>(inFile.tellg());
        if (len == 0) {
            HILOG_INFO("file is null");
        } else {
            inFile.seekg(0, std::ios::beg);
            HILOG_INFO("file is not null");
            inFile >> jsonFile;
            if (jsonFile.is_discarded()) {
                HILOG_INFO("json discarded error ...");
                inFile.close();
                return false;
            }
        }
        inFile.close();
    }

    std::ofstream outFile(AmsWhiteList::AMS_WHITE_LIST_FILE_PATH, std::ios::out);
    bool isOpen = outFile.good();
    if (isOpen) {
        HILOG_INFO("open file succeed");
        HILOG_INFO("bundleName = %{public}s is ", bundleName.c_str());
        if (isAllow) {
            HILOG_INFO("add json");
            jsonFile[bundleName][AmsWhiteList::ISAWAKEN_SCREEN] = true;
        } else {
            HILOG_INFO("delete json");
            jsonFile.erase(bundleName);
        }
        outFile << std::setw(AmsWhiteList::DUMP_INDENT) << jsonFile << std::endl;
    } else {
        HILOG_INFO("open file fail");
        return false;
    }
    jsonFile.clear();
    outFile.close();
    return true;
}

bool LockScreenWhiteList::GetWhiteListInfo(nlohmann::json &jsonFile)
{
    HILOG_INFO("%{public}s", __func__);
    std::ifstream inFile;
    inFile.open(AmsWhiteList::AMS_WHITE_LIST_FILE_PATH, std::ios::in);
    if (!inFile.is_open()) {
        HILOG_INFO("read Permit list error ...");
        return false;
    }
    inFile.seekg(0, std::ios::end);
    int len = static_cast<int>(inFile.tellg());
    if (len == 0) {
        HILOG_INFO("file is null");
        return false;
    } else {
        inFile.seekg(0, std::ios::beg);
        HILOG_INFO("file is not null");
        inFile >> jsonFile;
        if (jsonFile.is_discarded()) {
            HILOG_INFO("json discarded error ...");
            inFile.close();
            return false;
        }
    }
    inFile.close();
    return true;
}

bool LockScreenWhiteList::FindBundleNameOnWhiteList(const std::string &bundleName, bool &isAwakenScreen)
{
    nlohmann::json jsonFile;
    if (!GetWhiteListInfo(jsonFile)) {
        HILOG_INFO("fail...");
        return false;
    }
    if (jsonFile.contains(bundleName)) {
        HILOG_INFO("json info contains bundleName...");
        isAwakenScreen = jsonFile.at(bundleName).at(AmsWhiteList::ISAWAKEN_SCREEN).get<bool>();
        jsonFile.clear();
        return true;
    } else {
        HILOG_INFO("json info not contains bundleName...");
        jsonFile.clear();
        return false;
    }
}

bool LockScreenWhiteList::IsExistFile(const std::string &path)
{
    HILOG_INFO("%{public}s", __func__);
    if (path.empty()) {
        return false;
    }
    struct stat buf = {};
    if (stat(path.c_str(), &buf) != 0) {
        return false;
    }
    HILOG_INFO("%{public}s  :file exists", __func__);
    return S_ISREG(buf.st_mode);
}
}  // namespace AAFwk
}  // namespace OHOS
