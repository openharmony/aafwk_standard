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

#include "mock_app_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string PROCESS_NAME = "process_name";
}  // namespace

MockAppMgrClient::MockAppMgrClient()
{}
MockAppMgrClient::~MockAppMgrClient()
{}

AppMgrResultCode MockAppMgrClient::GetProcessRunningInfosByUserId(
    std::vector<RunningProcessInfo> &info, int32_t userId)
{
    RunningProcessInfo runningProcessInfo;
    runningProcessInfo.processName_ = PROCESS_NAME;
    info.emplace_back(runningProcessInfo);

    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode MockAppMgrClient::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token,
    const sptr<IRemoteObject> &preToken, const int32_t visibility, const int32_t perceptibility,
    const int32_t connectionState)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode MockAppMgrClient::ConnectAppMgrService()
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode MockAppMgrClient::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    return AppMgrResultCode::RESULT_OK;
}

}  // namespace AppExecFwk
}  // namespace OHOS