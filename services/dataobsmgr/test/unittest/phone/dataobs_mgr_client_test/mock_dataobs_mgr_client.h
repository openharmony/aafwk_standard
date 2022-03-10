
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

#ifndef MOCK_FOUNDATION_AAFWK_MOCK_DATAOBS_MGR_CLIENT_H
#define MOCK_FOUNDATION_AAFWK_MOCK_DATAOBS_MGR_CLIENT_H

#include <gmock/gmock.h>
#include <memory>
#include <map>
#define protected public
#define private public
#include "system_ability.h"
#include "dataobs_mgr_service.h"
#include "semaphore_ex.h"
#include "mock_dataobs_mgr_service.h"
#include "dataobs_mgr_client.h"
#include "mock_dataobs_mgr_service.h"

namespace OHOS {
namespace AAFwk {
class MockDataObsMgrClient : public DataObsMgrClient {
public:
    static std::shared_ptr<DataObsMgrClient> GetInstance();
    ErrCode Connect();
};

std::shared_ptr<DataObsMgrClient> MockDataObsMgrClient::GetInstance()
{
    if (instance_ == nullptr) {
        if (instance_ == nullptr) {
            std::shared_ptr<DataObsMgrClient> client {new (std::nothrow) MockDataObsMgrClient()};
            if (client != nullptr) {
                ((MockDataObsMgrClient *)client.get())->Connect();
            }
            instance_ = client;
        }
    }
    return instance_;
}

ErrCode MockDataObsMgrClient::Connect()
{
    if (remoteObject_ == nullptr) {
        sptr<IRemoteObject> mockDataObsMgrService(new (std::nothrow) MockDataObsMgrService());
        if (mockDataObsMgrService != nullptr) {
            ((MockDataObsMgrService *)mockDataObsMgrService.GetRefPtr())->OnStart();
        }

        remoteObject_ = mockDataObsMgrService;
    }
    return ERR_OK;
}

}  // namespace AAFwk
}  // namespace OHOS
#endif  // MOCK_FOUNDATION_AAFWK_MOCK_DATAOBS_MGR_CLIENT_H