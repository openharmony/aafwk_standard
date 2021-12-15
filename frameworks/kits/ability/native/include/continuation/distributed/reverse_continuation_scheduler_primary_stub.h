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
#ifndef OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_PRIMARY_STUB_H
#define OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_PRIMARY_STUB_H

#include "iremote_stub.h"
#include "reverse_continuation_scheduler_primary_interface.h"

namespace OHOS {
namespace AppExecFwk {
class ReverseContinuationSchedulerPrimaryStub : public IRemoteStub<IReverseContinuationSchedulerPrimary> {
public:
    ReverseContinuationSchedulerPrimaryStub();
    virtual ~ReverseContinuationSchedulerPrimaryStub();

    /**
     * @brief Sets an entry for receiving requests.
     *
     * @param code Indicates the service request code sent from the peer end.
     * @param data Indicates the MessageParcel object sent from the peer end.
     * @param reply Indicates the response message object sent from the remote service. The local service writes the
     * response data to the MessageParcel object.
     * @param option Indicates whether the operation is synchronous or asynchronous.
     * @return ERR_NONE if success, otherwise false.
     */
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int NotifyReplicaTerminatedInner(MessageParcel &data, MessageParcel &reply);
    int ContinuationBackInner(MessageParcel &data, MessageParcel &reply);

    static const std::string DESCRIPTOR;

private:
    using RequestFuncType = int (ReverseContinuationSchedulerPrimaryStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_PRIMARY_STUB_H */