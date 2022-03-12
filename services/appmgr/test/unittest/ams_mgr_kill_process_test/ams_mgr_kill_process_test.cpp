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

#include <gtest/gtest.h>

#define private public
#include "ams_mgr_scheduler.h"
#undef private
#include "app_mgr_interface.h"
#include "hilog_wrapper.h"
#include "mock_bundle_manager.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const std::string STRING_BUNDLE_NAME = "com.example.bundle";

constexpr int ACCOUNT_ID = 100;
}  // namespace

class AmsMgrKillProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void) override;
    void TearDown(void) override;

    std::shared_ptr<AppMgrServiceInner> GetAppMgrServiceInner(void);
    sptr<IAppMgr> GetAppMgrProxy(void);
};

void AmsMgrKillProcessTest::SetUpTestCase(void)
{}

void AmsMgrKillProcessTest::TearDownTestCase(void)
{}

void AmsMgrKillProcessTest::SetUp(void)
{}

void AmsMgrKillProcessTest::TearDown(void)
{}

std::shared_ptr<AppMgrServiceInner> AmsMgrKillProcessTest::GetAppMgrServiceInner(void)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IBundleMgr> bundleMgr = new BundleMgrService();
    appMgrServiceInner->remoteClientManager_->SetBundleManager(bundleMgr);

    return appMgrServiceInner;
}

sptr<IAppMgr> AmsMgrKillProcessTest::GetAppMgrProxy(void)
{
    auto instance = DelayedSingleton<SysMrgClient>::GetInstance();
    EXPECT_NE(instance, nullptr);

    auto object = instance->GetSystemAbility(APP_MGR_SERVICE_ID);
    EXPECT_NE(object, nullptr);

    auto proxy = iface_cast<IAppMgr>(object);
    EXPECT_NE(proxy, nullptr);

    return proxy;
}

/*
 * Feature: AppMgrServiceInner
 * Function: KillApplicationByUserId
 * SubFunction: NA
 * FunctionPoints:Kill process
 * EnvConditions: NA
 * CaseDescription: creat AppMgrServiceInner object, call function.
 */
HWTEST_F(AmsMgrKillProcessTest, KillProcess_0100, TestSize.Level0)
{
    HILOG_INFO("AmsMgrKillProcessTest_KillProcess_0100");

    auto appMgrServiceInner = GetAppMgrServiceInner();
    EXPECT_NE(appMgrServiceInner, nullptr);

    ErrCode result = appMgrServiceInner->KillApplicationByUserId(STRING_BUNDLE_NAME, ACCOUNT_ID);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AppMgrProxy
 * Function: KillApplicationByUserId
 * SubFunction: NA
 * FunctionPoints:Kill process
 * EnvConditions: NA
 * CaseDescription: creat AppMgrProxy object, call function.
 */
HWTEST_F(AmsMgrKillProcessTest, KillProcess_0200, TestSize.Level0)
{
    HILOG_INFO("AmsMgrKillProcessTest_KillProcess_0200");

    auto proxy = GetAppMgrProxy();
    EXPECT_NE(proxy, nullptr);

    ErrCode result = proxy->GetAmsMgr()->KillProcessWithAccount(STRING_BUNDLE_NAME, ACCOUNT_ID);
    EXPECT_EQ(result, ERR_OK);
}
