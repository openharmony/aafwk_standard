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

#include "data_ability_observer_proxy.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {

DataAbilityObserverProxy::DataAbilityObserverProxy(const sptr<IRemoteObject> &remote)
    : IRemoteProxy<IDataAbilityObserver>(remote)
{}
DataAbilityObserverProxy::~DataAbilityObserverProxy()
{}
/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @param uri Indicates the path of the data to operate.
 */
void DataAbilityObserverProxy::OnChange()
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("%{public}s remote is nullptr", __func__);
        return;
    }

    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;

    if (!data.WriteInterfaceToken(DataAbilityObserverProxy::GetDescriptor())) {
        HILOG_ERROR("%{public}s data.WriteInterfaceToken(GetDescriptor()) return false", __func__);
        return;
    }

    int result = remote->SendRequest(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE, data, reply, option);
    if (result == ERR_NONE) {
        HILOG_INFO("%{public}s SendRequest ok, retval is %d", __func__, reply.ReadInt32());
        return;
    } else {
        HILOG_ERROR("%{public}s SendRequest error, result=%{public}d", __func__, result);
        return;
    }
}

}  // namespace AAFwk
}  // namespace OHOS