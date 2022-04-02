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
#include <gtest/gtest.h>
#include "ability_manager_service.h"
#include "app_mgr_service.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"

namespace {
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace testing::ext;
class AccessControlCheck : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static sptr<OHOS::AAFwk::IAbilityManager> abilityMs_;
    static sptr<IAppMgr> appMs_;
};

sptr<IAbilityManager> AccessControlCheck::abilityMs_ = nullptr;
sptr<IAppMgr> AccessControlCheck::appMs_ = nullptr;

void AccessControlCheck::SetUpTestCase(void) {}
void AccessControlCheck::TearDownTestCase(void) {}
void AccessControlCheck::SetUp() {}
void AccessControlCheck::TearDown() {}

/**
 * @tc.number    : Access_Control_Check_0200
 * @tc.name      : check whether appManagerService is running
 * @tc.desc      : check whether appManagerService is running
 */
HWTEST_F(AccessControlCheck, Access_Control_Check_0200, Function | MediumTest | Level0)
{
    sptr<IRemoteObject> appMgrRemoteObj =
        OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(APP_MGR_SERVICE_ID);
    EXPECT_TRUE(appMgrRemoteObj != nullptr);
    if (appMgrRemoteObj == nullptr) {
        GTEST_LOG_(INFO) << "SaMgrClient::GetInstance()->GetSystemAbility(APP_MGR_SERVICE_ID) failed.";
        return;
    }
    sptr<IAppMgr> appManager = iface_cast<IAppMgr>(appMgrRemoteObj);
    EXPECT_TRUE(appManager != nullptr);
    if (appManager == nullptr) {
        GTEST_LOG_(INFO) << "iface_cast<IAbilityManager>(appMgrRemoteObj) failed.";
        return;
    }
    std::vector<RunningProcessInfo> info;
    EXPECT_TRUE(appManager->GetAllRunningProcesses(info) == ERR_OK);
    EXPECT_TRUE(info.size() > 0);
    std::string launcherBundle = "com.ohos.launcher";
    bool isLauncherExist = false;
    for (auto process : info) {
        if (process.processName_ == launcherBundle) {
            isLauncherExist = true;
            break;
        }
    }
    EXPECT_TRUE(isLauncherExist);
}
}  // namespace