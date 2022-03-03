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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_HOST_CLIENT_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_HOST_CLIENT_H

#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <set>
#include "form_callback_interface.h"
#include "form_host_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormHostClient
 * The service of the form host.
 */
class FormHostClient : public FormHostStub {
public:
    FormHostClient();
    virtual ~FormHostClient();

    /**
     * @brief Get FormHostClient instance.
     *
     * @return FormHostClient instance.
     */
    static sptr<FormHostClient> GetInstance();

    /**
     * @brief Add form.
     *
     * @param formCallback the host's form callback.
     * @param formId The Id of the form.
     * @return none.
     */
    void AddForm(std::shared_ptr<FormCallbackInterface> formCallback, const int64_t formId);

    /**
     * @brief Remove form.
     *
     * @param formCallback the host's form callback.
     * @param formId The Id of the form.
     * @return none.
     */
    void RemoveForm(std::shared_ptr<FormCallbackInterface> formCallback, const int64_t formId);

    /**
     * @brief Check whether the form exist in the formhosts.
     *
     * @param formId The Id of the form.
     * @return Returns true if contains form; returns false otherwise.
     */
    bool ContainsForm(int64_t formId);

    /**
     * @brief Request to give back a form.
     *
     * @param formJsInfo Form js info.
     * @return none.
     */
    virtual void OnAcquired(const FormJsInfo &formJsInfo);

     /**
     * @brief Update form.
     *
     * @param formJsInfo Form js info.
     * @return none.
     */
    virtual void OnUpdate(const FormJsInfo &formJsInfo);

    /**
     * @brief UnInstall the forms.
     *
     * @param formIds The Id of the forms.
     * @return none.
     */
    virtual void OnUninstall(const std::vector<int64_t> &formIds);

private:
    static std::mutex instanceMutex_;
    static sptr<FormHostClient> instance_;
    mutable std::mutex callbackMutex_;
    std::map<int64_t, std::set<std::shared_ptr<FormCallbackInterface>>> formCallbackMap_;

    DISALLOW_COPY_AND_MOVE(FormHostClient);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_HOST_CLIENT_H
