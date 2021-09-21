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

#ifndef OHOS_AAFWK_DATAOBS_MANAGER_INTERFACE_H
#define OHOS_AAFWK_DATAOBS_MANAGER_INTERFACE_H

#include <vector>

#include <ipc_types.h>
#include <iremote_broker.h>

#include "data_ability_observer_interface.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
using Uri = OHOS::Uri;
const std::string DATAOBS_MANAGER_SERVICE_NAME = "DataObsMgrService";
/**
 * @class IDataObsMgr
 * IDataObsMgr interface is used to access dataobs manager services.
 */
class IDataObsMgr : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.DataObsMgr")

    /**
     * Registers an observer to DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver) = 0;

    /**
     * Deregisters an observer used for DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver) = 0;

    /**
     * Notifies the registered observers of a change to the data resource specified by Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyChange(const Uri &uri) = 0;

    enum {
        // ipc id 1-1000 for kit
        // ipc id for RegisterObserver (1)
        REGISTER_OBSERVER = 1,

        // ipc id for UnregisterObserver (2)
        UNREGISTER_OBSERVER,

        // ipc id for NotifyChange (3)
        NOTIFY_CHANGE,
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DATAOBS_MANAGER_INTERFACE_H
