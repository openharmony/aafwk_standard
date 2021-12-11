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

#include "data_ability_observer_stub.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
DataAbilityObserverStub::DataAbilityObserverStub()
{
    requestFuncMap_[DATA_ABILITY_OBSERVER_CHANGE] = &DataAbilityObserverStub::OnChangeInner;
}

DataAbilityObserverStub::~DataAbilityObserverStub()
{
    requestFuncMap_.clear();
}

int DataAbilityObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("%{public}s, cmd = %d, flags= %d", __func__, code, option.GetFlags());
    std::u16string descriptor = DataAbilityObserverStub::GetDescriptor();
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
    HILOG_WARN("%{public}s, default case, need check.", __func__);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @param uri Indicates the path of the data to operate.
 *
 * @return Returns 0 on success, others on failure.
 */
int DataAbilityObserverStub::OnChangeInner(MessageParcel &data, MessageParcel &reply)
{
    OnChange();
    return ERR_NONE;
}

void DataObsCallbackRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_ERROR("recv DataObsCallbackRecipient death notice");

    if (handler_) {
        handler_(remote);
    }
}

DataObsCallbackRecipient::DataObsCallbackRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

DataObsCallbackRecipient::~DataObsCallbackRecipient()
{}
}  // namespace AAFwk
}  // namespace OHOS