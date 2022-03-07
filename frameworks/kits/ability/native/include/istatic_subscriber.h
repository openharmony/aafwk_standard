/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_APPEXECFWK_I_STATIC_SUBSCRIBER_H
#define OHOS_APPEXECFWK_I_STATIC_SUBSCRIBER_H

#include <string_ex.h>
#include <iremote_broker.h>
#include "common_event_data.h"

namespace OHOS {
namespace AppExecFwk {
using EventFwk::CommonEventData;

class IStaticSubscriber : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.AppExecFwk.IStaticSubscriber");

    /**
     * @brief Called when subscriber received common event.
     *
     * @param data Indicates the common event data..
     */
    virtual ErrCode OnReceiveEvent(CommonEventData* data) = 0;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_APPEXECFWK_I_STATIC_SUBSCRIBER_H

