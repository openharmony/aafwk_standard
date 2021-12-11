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

#include "dataobs_mgr_proxy.h"

#include "errors.h"
#include "string_ex.h"

#include "data_ability_observer_proxy.h"
#include "data_ability_observer_stub.h"
#include "dataobs_mgr_errors.h"

namespace OHOS {
namespace AAFwk {
bool DataObsManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(DataObsManagerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

int DataObsManagerProxy::RegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return DATAOBS_PROXY_INNER_ERR;
    }
    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("register observer fail, uri error");
        return ERR_INVALID_VALUE;
    }
    if (dataObserver == nullptr) {
        HILOG_ERROR("register observer fail, dataObserver is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteParcelable(dataObserver->AsObject())) {
        HILOG_ERROR("register observer fail, dataObserver error");
        return ERR_INVALID_VALUE;
    }

    error = Remote()->SendRequest(IDataObsMgr::REGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("register observer fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int DataObsManagerProxy::UnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return DATAOBS_PROXY_INNER_ERR;
    }
    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("unregister observer fail, uri error");
        return ERR_INVALID_VALUE;
    }
    if (dataObserver == nullptr) {
        HILOG_ERROR("unregister observer fail, dataObserver is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteParcelable(dataObserver->AsObject())) {
        HILOG_ERROR("unregister observer fail, dataObserver error");
        return ERR_INVALID_VALUE;
    }

    error = Remote()->SendRequest(IDataObsMgr::UNREGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("unregister observer fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int DataObsManagerProxy::NotifyChange(const Uri &uri)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return DATAOBS_PROXY_INNER_ERR;
    }
    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("notifyChange fail, uri error");
        return ERR_INVALID_VALUE;
    }

    error = Remote()->SendRequest(IDataObsMgr::NOTIFY_CHANGE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("notifyChange fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}
}  // namespace AAFwk
}  // namespace OHOS
