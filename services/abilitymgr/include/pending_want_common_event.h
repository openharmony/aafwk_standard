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

#ifndef PENDING_WANT_COMMON_EVENT_H
#define PENDING_WANT_COMMON_EVENT_H

#include "common_event_data.h"
#include "common_event_subscriber.h"
#include "want.h"
#include "want_params.h"
#include "want_receiver_interface.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::EventFwk;

class PendingWantCommonEvent : public CommonEventSubscriber {
public:
    PendingWantCommonEvent();
    virtual ~PendingWantCommonEvent() = default;

    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

    void SetWantParams(const WantParams &wantParams);
    void SetFinishedReceiver(const sptr<IWantReceiver> &finishedReceiver);

private:
    Want want_;
    WantParams wantParams_;
    sptr<IWantReceiver> finishedReceiver_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // PENDING_WANT_COMMON_EVENT_H