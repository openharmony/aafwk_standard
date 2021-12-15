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
#ifndef FOUNDATION_APPEXECFWK_OHOS_CONNECT_CALLBACK_STUB_H
#define FOUNDATION_APPEXECFWK_OHOS_CONNECT_CALLBACK_STUB_H
#include <memory>
#include <map>
#include "connect_callback_interface.h"
#include "iremote_stub.h"

using string = std::string;

namespace OHOS {
namespace AppExecFwk {
class ConnectCallbackStub : public IRemoteStub<IConnectCallback> {
public:
    ConnectCallbackStub();
    virtual ~ConnectCallbackStub() = default;
    /**
     * @brief Remote device sends connection or disconnection request.
     * @param Code indicators code of the connection or disconnection request function.
     * @param data indicators receives the message object.
     * @param reply indicators reply the message object.
     * @return none
     */
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int ConnectInner(MessageParcel &data, MessageParcel &reply);
    int DisconnectInner(MessageParcel &data, MessageParcel &reply);
    using ConnectCallbackFunc = int (ConnectCallbackStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<int, ConnectCallbackFunc> memberFuncMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_CONNECT_CALLBACK_STUB_H