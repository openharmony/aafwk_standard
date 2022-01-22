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

#include <singleton.h>

#include "appexecfwk_errors.h"
#include "bundle_info.h"
#include "form_info.h"

namespace OHOS {
namespace AppExecFwk {
class FormInfoMgr final : public DelayedRefSingleton<FormInfoMgr> {
DECLARE_DELAYED_REF_SINGLETON(FormInfoMgr)

public:
    DISALLOW_COPY_AND_MOVE(FormInfoMgr);

    static ErrCode LoadFormConfigInfoByBundleName(const std::string &bundleName, std::vector<FormInfo> &formInfos);

private:
    static ErrCode LoadAbilityFormConfigInfo(const BundleInfo &bundleInfo, std::vector<FormInfo> &formInfos);

    static ErrCode LoadStageFormConfigInfo(const BundleInfo &bundleInfo, std::vector<FormInfo> &formInfos);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_INFO_MGR_H
