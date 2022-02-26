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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_IRENDER_SCHEDULER_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_IRENDER_SCHEDULER_H

#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class IRenderScheduler
 * Ipc interface of render process to app mgr service.
 */
class IRenderScheduler : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.RenderScheduler");

    /**
     * Notify Browser's fd to render process.
     *
     * @param ipcFd, ipc file descriptior for web browser and render process.
     * @param sharedFd, shared memory file descriptior.
     */
    virtual void NotifyBrowserFd(int32_t ipcFd, int32_t sharedFd) = 0;

    enum class Message {
        NOTIFY_BROWSER_FD = 1,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_IRENDER_SCHEDULER_H
