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

#ifndef OHOS_AAFWK_WANT_RECEIVER_INTERFACE_H
#define OHOS_AAFWK_WANT_RECEIVER_INTERFACE_H

#include <iremote_broker.h>
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace AAFwk {
class IWantReceiver : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.WantReceiver");
    virtual void Send(const int32_t resultCode) = 0;
    virtual void PerformReceive(const Want &want, int resultCode, const std::string &data, const WantParams &extras,
        bool serialized, bool sticky, int sendingUser) = 0;
    enum {
        WANT_RECEIVER_SEND = 0,
        WANT_RECEIVER_PERFORM_RECEIVE,
    };
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_WANT_RECEIVER_INTERFACE_H