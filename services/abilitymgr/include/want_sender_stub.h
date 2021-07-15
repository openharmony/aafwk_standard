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

#ifndef OHOS_OS_AAFWK_WANT_SENDER_STUB_H
#define OHOS_OS_AAFWK_WANT_SENDER_STUB_H

#include <map>
#include "want_sender_interface.h"

#include <iremote_object.h>
#include <iremote_stub.h>

namespace OHOS {
namespace AAFwk {
class WantSenderStub : public IRemoteStub<IWantSender> {
public:
    WantSenderStub();
    virtual ~WantSenderStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int SendInner(MessageParcel &data, MessageParcel &reply);
    using RequestFuncType = int (WantSenderStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_OS_AAFWK_WANT_SENDER_STUB_H