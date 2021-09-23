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

#ifndef OHOS_AAFWK_DATAOBS_MANAGER_CLIENT_H
#define OHOS_AAFWK_DATAOBS_MANAGER_CLIENT_H

#include <mutex>

#include "data_ability_observer_interface.h"
#include "dataobs_mgr_errors.h"
#include "dataobs_mgr_interface.h"
#include "uri.h"

#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class DataObsMgrClient
 * DataObsMgrClient is used to access dataobs manager services.
 */
class DataObsMgrClient {
public:
    DataObsMgrClient();
    virtual ~DataObsMgrClient();
    static std::shared_ptr<DataObsMgrClient> GetInstance();

    /**
     * Registers an observer to DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver);

    /**
     * Deregisters an observer used for DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver);

    /**
     * Notifies the registered observers of a change to the data resource specified by Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode NotifyChange(const Uri &uri);

private:
    /**
     * Connect dataobs manager service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode Connect();

    static std::mutex mutex_;
    static std::shared_ptr<DataObsMgrClient> instance_;
    sptr<IRemoteObject> remoteObject_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DATAOBS_MANAGER_CLIENT_H
