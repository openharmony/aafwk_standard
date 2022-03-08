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

#ifndef OHOS_APPEXECFWK_STATIC_SUBSCRIBER_STUB_IMP_H
#define OHOS_APPEXECFWK_STATIC_SUBSCRIBER_STUB_IMP_H

#include <memory>
#include "js_static_subscriber_extension.h"
#include "static_subscriber_stub.h"

namespace OHOS {
namespace AppExecFwk {
using AbilityRuntime::JsStaticSubscriberExtension;
class StaticSubscriberStubImp : public StaticSubscriberStub {
public:
    explicit StaticSubscriberStubImp(const std::shared_ptr<JsStaticSubscriberExtension>& extension)
        : extension_(extension) {}

    virtual ~StaticSubscriberStubImp() {}

    ErrCode OnReceiveEvent(CommonEventData* data) override;
private:
    std::weak_ptr<JsStaticSubscriberExtension> extension_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_APPEXECFWK_STATIC_SUBSCRIBER_STUB_IMP_H

