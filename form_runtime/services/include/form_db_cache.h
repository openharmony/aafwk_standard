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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_DB_CACHE_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_DB_CACHE_H

#include <mutex>
#include <set>
#include <singleton.h>
#include <vector>

#include "appexecfwk_errors.h"
#include "form_id_key.h"
#include "form_record.h"
#include "form_storage_mgr.h"

namespace OHOS {
namespace AppExecFwk {
class FormDbCache final : public DelayedRefSingleton<FormDbCache> {
DECLARE_DELAYED_REF_SINGLETON(FormDbCache)
public:
    DISALLOW_COPY_AND_MOVE(FormDbCache);

    /**
     * @brief Load form data from DB to DbCache when starting.
     * @return Void.
     */
    void Start();

    /**
     * @brief Get all form data from DbCache.
     * @param formDBInfos Storage all DbCache.
     * @return Void
     */
    void GetAllFormInfo(std::vector<FormDBInfo> &formDBInfos);

    /**
     * @brief Save or update form data to DbCache and DB.
     * @param formDBInfo Form data.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SaveFormInfo(const FormDBInfo &formDBInfo);

    /**
     * @brief Save or update form data to DbCache and DB.
     * @param formDBInfo Form data.
     * @return Returns ERR_OK on success, others on failure.(NoLock)
     */
    ErrCode SaveFormInfoNolock(const FormDBInfo &formDBInfo);

    /**
     * @brief Delete form data in DbCache and DB with formId.
     * @param formId form data Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DeleteFormInfo(int64_t formId);

    /**
     * @brief Get record from DB cache with formId
     * @param formId Form data Id
     * @param record Form data
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetDBRecord(const int64_t formId, FormRecord &record) const;

    /**
     * @brief Get record from DB cache with formId
     * @param formId Form data Id
     * @param record Form db data
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetDBRecord(const int64_t formId, FormDBInfo &record) const;

    /**
     * @brief Use record save or update DB data and DB cache with formId
     * @param formId Form data Id
     * @param record Form data
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UpdateDBRecord(const int64_t formId, const FormRecord &record) const;

    /**
     * @brief Delete form data in DbCache and DB with formId.
     * @param formId form data Id.
     * @param removedDBForms Removed db form infos
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DeleteFormInfoByBundleName(const std::string &bundleName, const int32_t userId,
        std::vector<FormDBInfo> &removedDBForms);

    /**
     * @brief Get no host db record.
     * @param uid The caller uid.
     * @param noHostFormDBList no host db record list.
     * @param foundFormsMap Form Id list.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetNoHostDBForms(const int uid, std::map<FormIdKey, std::set<int64_t>> &noHostFormDBList,
        std::map<int64_t, bool> &foundFormsMap);

    /**
     * @brief Get match count by bundleName and moduleName.
     * @param bundleName BundleName.
     * @param moduleName ModuleName.
     * @return Returns match count.
     */
    int GetMatchCount(const std::string &bundleName, const std::string &moduleName);

    /**
     * @brief Get data storage.
     * @return Returns data storage.
     */
    std::shared_ptr<FormStorageMgr> GetDataStorage() const;
	
	/**
     * @brief delete forms bu userId.
     * @param userId user ID.
     */
    void DeleteDBFormsByUserId(const int32_t userId);

    /**
     * @brief handle get no host invalid DB forms.
     * @param userId User ID.
     * @param callingUid The UID of the proxy.
     * @param matchedFormIds The set of the valid forms.
     * @param noHostDBFormsMap The map of the no host forms.
     * @param foundFormsMap The map of the found forms.
     */
    void GetNoHostInvalidDBForms(int32_t userId, int32_t callingUid, std::set<int64_t> &matchedFormIds,
                                 std::map<FormIdKey, std::set<int64_t>> &noHostDBFormsMap,
                                 std::map<int64_t, bool> &foundFormsMap);

    /**
     * @brief handle delete no host DB forms.
     * @param callingUid The UID of the proxy.
     * @param noHostDBFormsMap The map of the no host forms.
     * @param foundFormsMap The map of the found forms.
     */
    void BatchDeleteNoHostDBForms(int32_t callingUid, std::map<FormIdKey, std::set<int64_t>> &noHostDBFormsMap,
                                  std::map<int64_t, bool> &foundFormsMap);

    /**
     * @brief handle delete invalid DB forms.
     * @param userId User ID.
     * @param callingUid The UID of the proxy.
     * @param matchedFormIds The set of the valid forms.
     * @param removedFormsMap The map of the removed invalid forms.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DeleteInvalidDBForms(int32_t userId, int32_t callingUid, std::set<int64_t> &matchedFormIds,
                                 std::map<int64_t, bool> &removedFormsMap);
private:
    std::shared_ptr<FormStorageMgr> dataStorage_;
    mutable std::mutex formDBInfosMutex_;
    std::vector<FormDBInfo> formDBInfos_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_DB_CACHE_H
