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

#ifndef OHOS_AAFWK_STOP_USER_CALLBACK_STUB_H
#define OHOS_AAFWK_STOP_USER_CALLBACK_STUB_H

#include <iremote_object.h>
#include <iremote_stub.h>
#include <vector>

#include "stop_user_callback.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class StopUserCallbackStub
 * StopUserCallbackStub.
 */
class StopUserCallbackStub : public IRemoteStub<IStopUserCallback> {
public:
    StopUserCallbackStub();
    virtual ~StopUserCallbackStub() = default;

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(StopUserCallbackStub);

    int OnStopUserDoneInner(MessageParcel &data, MessageParcel &reply);

    using StopUserCallbackFunc = int (StopUserCallbackStub::*)(MessageParcel &data, MessageParcel &reply);
    std::vector<StopUserCallbackFunc> vecMemberFunc_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_STOP_USER_CALLBACK_STUB_H
