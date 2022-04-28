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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_INFO_MGR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_INFO_MGR_H

#include <shared_mutex>
#include <singleton.h>
#include <unordered_map>

#include "appexecfwk_errors.h"
#include "bundle_info.h"
#include "form_info.h"
#include "form_info_storage.h"

namespace OHOS {
namespace AppExecFwk {
class FormInfoHelper {
public:
    static ErrCode LoadFormConfigInfoByBundleName(const std::string &bundleName, std::vector<FormInfo> &formInfos);

private:
    static ErrCode LoadAbilityFormConfigInfo(const BundleInfo &bundleInfo, std::vector<FormInfo> &formInfos);

    static ErrCode LoadStageFormConfigInfo(const BundleInfo &bundleInfo, std::vector<FormInfo> &formInfos);
};

class BundleFormInfo {
public:
    explicit BundleFormInfo(std::string bundleName);

    ErrCode InitFromJson(const std::string &formInfoStoragesJson);

    ErrCode Update();

    ErrCode Remove();

    bool Empty();

    ErrCode GetAllFormsInfo(std::vector<FormInfo> &formInfos);

    ErrCode GetFormsInfoByModule(const std::string &moduleName, std::vector<FormInfo> &formInfos);

private:
    std::string bundleName_ {};
    mutable std::shared_timed_mutex formInfosMutex_ {};
    std::vector<AAFwk::FormInfoStorage> formInfoStorages_ {};
};

class FormInfoMgr final : public DelayedRefSingleton<FormInfoMgr> {
DECLARE_DELAYED_REF_SINGLETON(FormInfoMgr)

public:
    DISALLOW_COPY_AND_MOVE(FormInfoMgr);

    ErrCode Start();

    ErrCode Update(const std::string &bundleName);

    ErrCode Remove(const std::string &bundleName);

    ErrCode GetAllFormsInfo(std::vector<FormInfo> &formInfos);

    ErrCode GetFormsInfoByBundle(const std::string &bundleName, std::vector<FormInfo> &formInfos);

    ErrCode GetFormsInfoByModule(const std::string &bundleName, const std::string &moduleName,
                                 std::vector<FormInfo> &formInfos);

private:
    std::shared_ptr<BundleFormInfo> GetOrCreateBundleFromInfo(const std::string &bundleName);
    bool IsCaller(std::string bundleName);
    bool CheckBundlePermission();

    mutable std::shared_timed_mutex bundleFormInfoMapMutex_ {};
    std::unordered_map<std::string, std::shared_ptr<BundleFormInfo>> bundleFormInfoMap_ {};
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_INFO_MGR_H
