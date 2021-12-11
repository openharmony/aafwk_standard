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

#ifndef FOUNDATION_APPEXECFWK_OHOS_REMOTE_REGISTER_SERVICE_STUB_H
#define FOUNDATION_APPEXECFWK_OHOS_REMOTE_REGISTER_SERVICE_STUB_H

#include "remote_register_service_interface.h"
#include <iremote_object.h>
#include <iremote_stub.h>
#include <map>
#include "message_parcel.h"

namespace OHOS {
namespace AppExecFwk {
class RemoteRegisterServiceStub : public IRemoteStub<IRemoteRegisterService> {
public:
    RemoteRegisterServiceStub();
    virtual ~RemoteRegisterServiceStub();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int RegisterInner(MessageParcel &data, MessageParcel &reply);
    int UnregisterInner(MessageParcel &data, MessageParcel &reply);
    int UpdateConnectStatusInner(MessageParcel &data, MessageParcel &reply);
    int ShowDeviceListInner(MessageParcel &data, MessageParcel &reply);

    using RequestRegisterFuncType = int (RemoteRegisterServiceStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, RequestRegisterFuncType> requestFuncMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_REMOTE_REGISTER_SERVICE_STUB_H
