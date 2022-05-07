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

#include <thread>
#include <functional>
#include <fstream>
#include <nlohmann/json.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define private public
#define protected public
#include "sa_mgr_client.h"
#include "mock_ability_connect_callback_stub.h"
#include "mock_ability_scheduler.h"
#include "mock_app_mgr_client.h"
#include "mock_bundle_mgr.h"
#include "ability_state.h"
#include "ability_manager_errors.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "ability_manager_service.h"
#include "ability_connect_callback_proxy.h"
#include "ability_config.h"
#include "pending_want_manager.h"
#include "pending_want_record.h"
#undef private
#undef protected
#include "wants_info.h"
#include "want_receiver_stub.h"
#include "want_sender_stub.h"
#include "os_account_manager.h"

using namespace std::placeholders;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::iface_cast;
using OHOS::IRemoteObject;
using OHOS::sptr;
using testing::_;
using testing::Invoke;
using testing::Return;

namespace OHOS {
namespace AAFwk {
class SpecifiedAbilityServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    Want CreateWant(const std::string &entity);
    AbilityInfo CreateAbilityInfo(const std::string &name, const std::string &appName, const std::string &bundleName);
    ApplicationInfo CreateAppInfo(const std::string &appName, const std::string &name);
    bool MockAppClent();
    void WaitAMS();

    inline static std::shared_ptr<MockAppMgrClient> mockAppMgrClient_ {nullptr};
    inline static std::shared_ptr<AbilityManagerService> abilityMgrServ_ {nullptr};
    sptr<MockAbilityScheduler> scheduler_ {nullptr};
    inline static bool doOnce_ = false;  // In order for mock to execute once
};

Want SpecifiedAbilityServiceTest::CreateWant(const std::string &entity)
{
    Want want;
    if (!entity.empty()) {
        want.AddEntity(entity);
    }
    return want;
}

AbilityInfo SpecifiedAbilityServiceTest::CreateAbilityInfo(
    const std::string &name, const std::string &appName, const std::string &bundleName)
{
    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.name = name;
    abilityInfo.applicationName = appName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.applicationInfo.bundleName = bundleName;
    abilityInfo.applicationName = "hiMusic";
    abilityInfo.applicationInfo.name = "hiMusic";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;

    return abilityInfo;
}

ApplicationInfo SpecifiedAbilityServiceTest::CreateAppInfo(const std::string &appName, const std::string &bundleName)
{
    ApplicationInfo appInfo;
    appInfo.name = appName;
    appInfo.bundleName = bundleName;

    return appInfo;
}

bool SpecifiedAbilityServiceTest::MockAppClent()
{
    if (!mockAppMgrClient_) {
        GTEST_LOG_(INFO) << "MockAppClent::1";
        return false;
    }

    if (!abilityMgrServ_->appScheduler_) {
        GTEST_LOG_(INFO) << "MockAppClent::2";
        return false;
    }

    abilityMgrServ_->appScheduler_->appMgrClient_.reset(mockAppMgrClient_.get());
    return true;
}

void SpecifiedAbilityServiceTest::WaitAMS()
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    if (!abilityMgrServ_) {
        return;
    }
    auto handler = abilityMgrServ_->GetEventHandler();
    if (!handler) {
        return;
    }
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    if (handler->PostTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

void SpecifiedAbilityServiceTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new (std::nothrow) BundleMgrService());
    abilityMgrServ_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    mockAppMgrClient_ = std::make_shared<MockAppMgrClient>();
}

void SpecifiedAbilityServiceTest::TearDownTestCase(void)
{
    abilityMgrServ_->OnStop();
    mockAppMgrClient_.reset();
}

void SpecifiedAbilityServiceTest::SetUp(void)
{
    scheduler_ = new MockAbilityScheduler();
    if (!doOnce_) {
        doOnce_ = true;
        MockAppClent();
    }
    WaitAMS();
}

void SpecifiedAbilityServiceTest::TearDown(void)
{}

/**
 * @tc.name: OnAcceptWantResponse_001
 * @tc.desc: test OnAcceptWantResponse
 * @tc.type: FUNC
 * @tc.require: AR000GJUND
 */
HWTEST_F(SpecifiedAbilityServiceTest, OnAcceptWantResponse_001, TestSize.Level1)
{
    std::string abilityName = "MusicAbility";
    std::string appName = "test_app";
    std::string bundleName = "com.ix.hiMusic";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant("");
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + "1", appName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(appName, bundleName);

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::FOREGROUND);

    abilityMgrServ_->InitMissionListManager(11, true);
    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    EXPECT_TRUE(abilityMgrServ_->currentMissionListManager_);
    abilityMgrServ_->currentMissionListManager_->EnqueueWaittingAbility(abilityRequest);
    abilityMgrServ_->OnAcceptWantResponse(want, "flag");

    EXPECT_EQ(false, abilityRecord->IsNewWant());
}
}  // namespace AAFwk
}  // namespace OHOS
