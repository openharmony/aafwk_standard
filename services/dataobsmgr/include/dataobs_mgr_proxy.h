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

#ifndef OHOS_AAFWK_DATAOBS_MANAGER_PROXY_H
#define OHOS_AAFWK_DATAOBS_MANAGER_PROXY_H

#include "dataobs_mgr_interface.h"
#include "hilog_wrapper.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class DataObsManagerProxy
 * DataObsManagerProxy.
 */
class DataObsManagerProxy : public IRemoteProxy<IDataObsMgr> {
public:
    explicit DataObsManagerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IDataObsMgr>(impl)
    {}

    virtual ~DataObsManagerProxy()
    {}

    /**
     * Registers an observer to DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns ERR_OK on success, others on failure.
     */

    virtual int RegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver);

    /**
     * Deregisters an observer used for DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver);

    /**
     * Notifies the registered observers of a change to the data resource specified by Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyChange(const Uri &uri);

private:
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<DataObsManagerProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DATAOBS_MANAGER_PROXY_H
