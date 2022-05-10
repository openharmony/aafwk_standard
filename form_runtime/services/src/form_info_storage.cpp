/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#include "form_info_storage.h"

#include "hilog_wrapper.h"
#include "json_serializer.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string JSON_KEY_USER_ID = "userId";
const std::string JSON_KEY_FORM_INFO = "formInfos";
} // namespace

FormInfoStorage::FormInfoStorage(int32_t userId, std::vector<AppExecFwk::FormInfo> &formInfos)
{
    this->userId = userId;
    for (const auto &item : formInfos) {
        this->formInfos.push_back(item);
    }
}

void FormInfoStorage::GetAllFormsInfo(int32_t userId, std::vector<AppExecFwk::FormInfo> &formInfos) const
{
    HILOG_INFO("Get all forms infos, current userId is%{public}d, this userId is %{public}d.", userId, this->userId);
    if (this->userId != userId) {
        return;
    }
    for (const auto &item : this->formInfos) {
        formInfos.push_back(item);
    }
}

void FormInfoStorage::GetFormsInfoByModule(int32_t userId, const std::string &moduleName,
    std::vector<AppExecFwk::FormInfo> &formInfos) const
{
    if (this->userId != userId) {
        return;
    }
    for (const auto &item : this->formInfos) {
        if (item.moduleName == moduleName) {
            formInfos.push_back(item);
        }
    }
}

void to_json(nlohmann::json &jsonObject, const FormInfoStorage &formInfoStorage)
{
    jsonObject = nlohmann::json {
        {JSON_KEY_USER_ID, formInfoStorage.userId},
        {JSON_KEY_FORM_INFO, formInfoStorage.formInfos}
    };
}

void from_json(const nlohmann::json &jsonObject, FormInfoStorage &formInfoStorage)
{
    formInfoStorage.userId = jsonObject.at(JSON_KEY_USER_ID).get<int32_t>();
    formInfoStorage.formInfos = jsonObject.at(JSON_KEY_FORM_INFO).get<std::vector<AppExecFwk::FormInfo>>();
}
} // namespace AppExecFwk
} // namespace OHOS
