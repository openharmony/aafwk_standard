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
#include "mock_form_host_client.h"
#include <gtest/gtest.h>
#include "appexecfwk_errors.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * Request to give back a Form.
 *
 * @param formInfo, Form info.
 * @return none.
 */
void MockFormHostClient::OnAcquired(const FormJsInfo &formInfo)
{
    HILOG_DEBUG("MockFormHostClient OnAcquired");

    int64_t formId = formInfo.formId;
    ASSERT_FALSE(formId == 0);
    PostVoid();
}

/**
* Form is updated.
*
* @param bundleName, Provider ability bundleName.
* @return none.
*/
void MockFormHostClient::OnUpdate(const FormJsInfo &formInfo)
{
    HILOG_DEBUG("MockFormHostClient OnUpdate");
    PostVoid();
}

/**
 * Form provider is uninstalled
 *
 * @param formIds, The Id list of the forms.
 * @return none.
 */
void  MockFormHostClient::OnUninstall(const std::vector<int64_t> &formIds)
{
    HILOG_DEBUG("MockFormHostClient OnUnInstall");
    PostVoid();
}

void MockFormHostClient::OnAcquireState(AppExecFwk::FormState state, const AAFwk::Want &want)
{
    HILOG_DEBUG("MockFormHostClient OnAcquireState");
    PostVoid();
}
}  // namespace AppExecFwk
}  // namespace OHOS
