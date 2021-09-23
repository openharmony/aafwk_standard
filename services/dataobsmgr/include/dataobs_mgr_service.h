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

#ifndef OHOS_AAFWK_DATAOBS_MANAGER_SERVICE_H
#define OHOS_AAFWK_DATAOBS_MANAGER_SERVICE_H

#include <memory>
#include <singleton.h>
#include <thread_ex.h>
#include <unordered_map>

#include "dataobs_mgr_inner.h"
#include "dataobs_mgr_stub.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "system_ability.h"
#include "uri.h"
#include "event_runner.h"
#include "event_handler.h"

namespace OHOS {
namespace AAFwk {
using EventRunner = OHOS::AppExecFwk::EventRunner;
using EventHandler = OHOS::AppExecFwk::EventHandler;
enum class DataObsServiceRunningState { STATE_NOT_START, STATE_RUNNING };

/**
 * @class DataObsMgrService
 * DataObsMgrService provides a facility for dataobserver.
 */
class DataObsMgrService : public SystemAbility,
                          public DataObsManagerStub,
                          public std::enable_shared_from_this<DataObsMgrService> {
    DECLARE_DELAYED_SINGLETON(DataObsMgrService)
    DECLEAR_SYSTEM_ABILITY(DataObsMgrService)
public:
    void OnStart() override;
    void OnStop() override;
    DataObsServiceRunningState QueryServiceState() const;

    virtual int RegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver) override;
    virtual int UnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver) override;
    virtual int NotifyChange(const Uri &uri) override;

    /**
     * GetEventHandler, get the dataobs manager service's handler.
     *
     * @return Returns EventHandler ptr.
     */
    std::shared_ptr<EventHandler> GetEventHandler();

private:
    bool Init();
    std::shared_ptr<EventRunner> eventLoop_;
    std::shared_ptr<EventHandler> handler_;
    DataObsServiceRunningState state_;
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner_;
    const int taskMax_ = 50;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DATAOBS_MANAGER_SERVICE_H
