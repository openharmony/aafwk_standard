
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

#ifndef MOCK_FOUNDATION_AAFWK_MOCK_DATAOBS_MGR_SERVICE_H
#define MOCK_FOUNDATION_AAFWK_MOCK_DATAOBS_MGR_SERVICE_H

#include <gmock/gmock.h>
#define protected public
#define private public
#include "dataobs_mgr_stub.h"

namespace OHOS {
namespace AAFwk {
class MockDataObsMgrService : public DataObsManagerStub {
public:
    MockDataObsMgrService() = default;
    virtual ~MockDataObsMgrService() = default;

    MOCK_METHOD2(RegisterObserverCall, int(const Uri &, const sptr<IDataAbilityObserver> &));
    MOCK_METHOD2(UnregisterObserverCall, int(const Uri &, const sptr<IDataAbilityObserver> &));
    MOCK_METHOD1(NotifyChangeCall, int(const Uri &));

    int RegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
    {
        RegisterObserverCall(uri, dataObserver);
        return 1;
    }
    int UnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
    {
        UnregisterObserverCall(uri, dataObserver);
        return 1;
    }
    int NotifyChange(const Uri &uri)
    {
        NotifyChangeCall(uri);
        return 1;
    }

    void OnStart()
    {
        if (state_ == DataObsServiceRunningState::STATE_RUNNING) {
            HILOG_INFO("Dataobs Manager Service has already started.");
            return;
        }
        HILOG_INFO("Dataobs Manager Service started.");
        if (!Init()) {
            HILOG_ERROR("failed to init service.");
            return;
        }
        state_ = DataObsServiceRunningState::STATE_RUNNING;
        eventLoop_->Run();

        HILOG_INFO("Ability Manager Service start success.");
    }
    void OnStop()
    {
        HILOG_INFO("stop service");
        eventLoop_.reset();
        handler_.reset();
        state_ = DataObsServiceRunningState::STATE_NOT_START;
    }
    /**
     * GetEventHandler, get the dataobs manager service's handler.
     * @return Returns EventHandler ptr.
     */
    std::shared_ptr<EventHandler> GetEventHandler()
    {
        return handler_;
    }

private:
    bool Init()
    {
        eventLoop_ = AppExecFwk::EventRunner::Create("DataObsMgrService");
        if (eventLoop_ == nullptr) {
            return false;
        }

        handler_ = std::make_shared<AppExecFwk::EventHandler>(eventLoop_);
        if (handler_ == nullptr) {
            return false;
        }

        HILOG_INFO("init success");
        return true;
    }

    std::shared_ptr<EventRunner> eventLoop_;
    std::shared_ptr<EventHandler> handler_;
    DataObsServiceRunningState state_;
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner_;
    const int taskMax_ = 50;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // MOCK_FOUNDATION_AAFWK_MOCK_DATAOBS_MGR_SERVICE_H
