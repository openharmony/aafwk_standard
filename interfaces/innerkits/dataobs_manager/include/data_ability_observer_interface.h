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
#ifndef OHOS_AAFWK_DATA_ABILITY_OBSERVER_INTERFACE_H
#define OHOS_AAFWK_DATA_ABILITY_OBSERVER_INTERFACE_H

#include <iremote_broker.h>
#include "uri.h"

namespace OHOS {
namespace AAFwk {
class IDataAbilityObserver : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.DataAbilityObserver");

    enum {
        DATA_ABILITY_OBSERVER_CHANGE,
    };

    /**
     * @brief Called back to notify that the data being observed has changed.
     *
     * @param uri Indicates the path of the data to operate.
     */
    virtual void OnChange() = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DATA_ABILITY_OBSERVER_INTERFACE_H