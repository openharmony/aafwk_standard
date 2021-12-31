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
#include "connect_callback_stub.h"

#include "app_log_wrapper.h"
#include "ipc_types.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
ConnectCallbackStub::ConnectCallbackStub()
{
    memberFuncMap_[COMMAND_CONNECT] = &ConnectCallbackStub::ConnectInner;
    memberFuncMap_[COMMAND_DISCONNECT] = &ConnectCallbackStub::DisconnectInner;
}

int ConnectCallbackStub::ConnectInner(MessageParcel &data, MessageParcel &reply)
{
    APP_LOGI("%{public}s called begin", __func__);
    string deviceId = Str16ToStr8(data.ReadString16());
    string deviceType = Str16ToStr8(data.ReadString16());
    Connect(deviceId, deviceType);
    APP_LOGI("%{public}s called end", __func__);
    return OHOS::ERR_NONE;
}

int ConnectCallbackStub::DisconnectInner(MessageParcel &data, MessageParcel &reply)
{
    APP_LOGI("%{public}s called begin", __func__);
    string deviceId = Str16ToStr8(data.ReadString16());
    Disconnect(deviceId);
    APP_LOGI("%{public}s called end", __func__);
    return OHOS::ERR_NONE;
}
/**
 * @brief Remote device sends connection or disconnection request.
 * @param Code indicators code of the connection or disconnection request function.
 * @param data indicators receives the message object.
 * @param reply indicators reply the message object.
 * @return none
 */
int ConnectCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    APP_LOGI("%{public}s called", __func__);
    std::u16string token = data.ReadInterfaceToken();
    if (token.compare(IConnectCallback::GetDescriptor()) != 0) {
        APP_LOGE("%{public}s Descriptor is wrong", __func__);
        return OHOS::ERR_INVALID_REPLY;
    }
    auto localFuncIt = memberFuncMap_.find(code);
    if (localFuncIt != memberFuncMap_.end()) {
        auto memberFunc = localFuncIt->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    APP_LOGI("ConnectCallbackStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS