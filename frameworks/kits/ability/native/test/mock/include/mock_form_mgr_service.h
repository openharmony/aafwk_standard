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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_MOCK_FORM_MGR_SERVICE_H
#define FOUNDATION_APPEXECFWK_SERVICES_MOCK_FORM_MGR_SERVICE_H

#include <codecvt>
#include <memory>
#include <singleton.h>
#include <thread_ex.h>
#include <unordered_map>
#include <vector>

#include "event_handler.h"
#include "form_constants.h"
#include "form_mgr_stub.h"
#include "form_js_info.h"
#include "form_provider_data.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class MockFormMgrService.
 * The mockFormMgrService for ability test.
 */
class MockFormMgrService : public IFormMgr {
public:
    MockFormMgrService() {};
    virtual ~MockFormMgrService() {};

    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }

    /**
     * @brief Start envent for the form manager service.
     */
    void OnStart() {}

    /**
     * @brief Stop envent for the form manager service.
     */
    void OnStop() {}

    /**
     * @brief Add form with want, send want to form manager service.
     * @param formId The Id of the forms to add.
     * @param want The want of the form to add.
     * @param callerToken Caller ability token.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AddForm(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken, FormJsInfo &formInfo)
    {
        if (formId == 300L) {
            formInfo.formId = formId;
        }
        return 0;
    }

    /**
     * @brief Delete forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to delete.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }

    /**
     * @brief Release forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to release.
     * @param callerToken Caller ability token.
     * @param delCache Delete Cache or not.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache)
    {
        return 0;
    }

    /**
     * @brief Update form with formId, send formId to form manager service.
     * @param formId The Id of the form to update.
     * @param bundleName Provider ability bundleName.
     * @param formBindingData Form binding data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int UpdateForm(const int64_t formId, const std::string& bundleName, const FormProviderData& formBindingData)
    {
        if (formId == 300L) {
            return -1;
        }
        return 0;
    }

    /**
     * @brief Request form with formId and want, send formId and want to form manager service.
     * @param formId The Id of the form to update.
     * @param callerToken Caller ability token.
     * @param want The want of the form to add.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want)
    {
        return 0;
    }

    /**
     * @brief Form visible/invisible notify, send formIds to form manager service.
     * @param formIds The Id list of the forms to notify.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int NotifyWhetherVisibleForms(
        const std::vector<int64_t> &formIds,
        const sptr<IRemoteObject> &callerToken,
        const int32_t formVisibleType)
    {
        if (formIds.size() == 1) {
            return 0;
        }
        return 1;
    }

    /**
     * @brief temp form to normal form.
     * @param formId The Id of the form.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
    {
        if (formId == 175L)return 0;
        return 1;
    }

    /**
     * @brief Dump all of form storage infos.
     * @param formInfos All of form storage infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpStorageFormInfos(std::string& formInfos)
    {
        return 0;
    }

    /**
     * @brief Dump form info by a bundle name.
     * @param bundleName The bundle name of form provider.
     * @param formInfos Form infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos)
    {
        return 0;
    }

    /**
     * @brief Dump form info by a bundle name.
     * @param formId The id of the form.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo)
    {
        return 0;
    }

    /**
     * @brief Set next refresh time.
     * @param formId The Id of the form.
     * @param nextTime The next refresh time.
     * @return Returns ERR_OK on success, others on failure.
     */
    int SetNextRefreshTime(const int64_t formId, const int64_t nextTime)
    {
        if (formId >= 1) {
            return ERR_OK;
        } else {
            return ERR_INVALID_VALUE;
        }
    }

    /**
     * @brief Lifecycle Update.
     * @param formIds The Id list of the forms to notify.
     * @param callerToken Caller ability token.
     * @param updateType The update type.
     * @return Returns ERR_OK on success, others on failure.
     */
    int LifecycleUpdate(
        const std::vector<int64_t> &formIds,
        const sptr<IRemoteObject> &callerToken,
        const int32_t updateType)
    {
        if (formIds.size() == 0) {
            return ERR_INVALID_VALUE;
        }
        return ERR_OK;
    }

    /**
     * @brief Process js message event.
     * @param formId Indicates the unique id of form.
     * @param want information passed to supplier.
     * @param callerToken Caller ability token.
     * @return Returns true if execute success, false otherwise.
     */
    int MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_MOCK_FORM_MGR_SERVICE_H
