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

#include "dataobs_mgr_stub.h"

#include "errors.h"
#include "string_ex.h"

#include "data_ability_observer_proxy.h"
#include "data_ability_observer_stub.h"
#include "dataobs_mgr_errors.h"

namespace OHOS {
namespace AAFwk {
using Uri = OHOS::Uri;
DataObsManagerStub::DataObsManagerStub()
{
    requestFuncMap_[REGISTER_OBSERVER] = &DataObsManagerStub::RegisterObserverInner;
    requestFuncMap_[UNREGISTER_OBSERVER] = &DataObsManagerStub::UnregisterObserverInner;
    requestFuncMap_[NOTIFY_CHANGE] = &DataObsManagerStub::NotifyChangeInner;
}

DataObsManagerStub::~DataObsManagerStub()
{
    requestFuncMap_.clear();
}

int DataObsManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("DataObsManagerStub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
    std::u16string descriptor = DataObsManagerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    HILOG_WARN("DataObsManagerStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int DataObsManagerStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("DataObsManagerStub: uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto observer = iface_cast<IDataAbilityObserver>(data.ReadRemoteObject());
    int32_t result = RegisterObserver(*uri, observer);
    reply.WriteInt32(result);
    if (uri != nullptr) {
        delete uri;
    }
    return NO_ERROR;
}

int DataObsManagerStub::UnregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("DataObsManagerStub: uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto observer = iface_cast<IDataAbilityObserver>(data.ReadRemoteObject());
    int32_t result = UnregisterObserver(*uri, observer);
    reply.WriteInt32(result);
    if (uri != nullptr) {
        delete uri;
    }
    return NO_ERROR;
}

int DataObsManagerStub::NotifyChangeInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("DataObsManagerStub: uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t result = NotifyChange(*uri);
    reply.WriteInt32(result);
    if (uri != nullptr) {
        delete uri;
    }
    return NO_ERROR;
}

}  // namespace AAFwk
}  // namespace OHOS
