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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_RENDER_SCHEDULER_HOST_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_RENDER_SCHEDULER_HOST_H

#include <map>

#include "irender_scheduler.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class RenderSchedulerHost
 * RenderScheduler stub.
 */
class RenderSchedulerHost : public IRemoteStub<IRenderScheduler> {
public:
    RenderSchedulerHost();
    virtual ~RenderSchedulerHost();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleNotifyBrowserFd(MessageParcel &data, MessageParcel &reply);

    using RenderSchedulerFunc = int32_t (RenderSchedulerHost::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, RenderSchedulerFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(RenderSchedulerHost);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_RENDER_SCHEDULER_HOST_H
