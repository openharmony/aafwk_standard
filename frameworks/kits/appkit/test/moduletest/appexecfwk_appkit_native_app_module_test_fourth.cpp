/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "main_thread.h"
#include "mock_ability_mgr_service.h"
#include "mock_ability_token.h"
#include "mock_app_mgr_service.h"
#include "mock_app_thread.h"
#include "mock_bundle_mgr_service.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS;
using namespace OHOS::AAFwk;
const int TestProcessInfo = 9999;

template<typename F>
static void WaitUntilTaskCalled(const F &f, const std::shared_ptr<EventHandler> &handler, std::atomic<bool> &taskCalled)
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    if (handler->PostTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            // if delay more than 1 second, break
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

static void WaitUntilTaskDone(const std::shared_ptr<EventHandler> &handler)
{
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    WaitUntilTaskCalled(f, handler, taskCalled);
}

class AppkitNativeModuleTestThird : public testing::Test {
public:
    AppkitNativeModuleTestThird() : AppMgrObject_(nullptr), mockAppMgr(nullptr), mockHandler_(nullptr), runner_(nullptr)
    {}
    ~AppkitNativeModuleTestThird()
    {}
    OHOS::sptr<OHOS::IRemoteObject> AppMgrObject_ = nullptr;
    MockAppMgrService *mockAppMgr = nullptr;
    std::shared_ptr<MockHandler> mockHandler_ = nullptr;
    std::shared_ptr<EventRunner> runner_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppkitNativeModuleTestThird::SetUpTestCase(void)
{}

void AppkitNativeModuleTestThird::TearDownTestCase(void)
{}

void AppkitNativeModuleTestThird::SetUp(void)
{
    GTEST_LOG_(INFO) << "AppkitNativeModuleTestThird SetUp";
    AppMgrObject_ = new (std::nothrow) MockAppMgrService();
    mockAppMgr = iface_cast<MockAppMgrService>(AppMgrObject_);

    runner_ = EventRunner::Create("AppkitNativeModuleTestMockHandlerSecond");
    mockHandler_ = std::make_shared<MockHandler>(runner_);

    auto task = [abilityThread = this]() { MainThread::Start(); };
    mockHandler_->PostTask(task);
    WaitUntilTaskDone(mockHandler_);
}

void AppkitNativeModuleTestThird::TearDown(void)
{
    GTEST_LOG_(INFO) << "AppkitNativeModuleTestThird TearDown";
    AppLaunchData lanchdate;
    ApplicationInfo appinf;
    ProcessInfo processing("TestProcess", TestProcessInfo);
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    Configuration config;
    mockAppMgr->ScheduleLaunchApplication(lanchdate, config);
    WaitUntilTaskDone(mockHandler_);

    mockAppMgr->ScheduleTerminateApplication();
    WaitUntilTaskDone(mockHandler_);
}

/**
 * @tc.number: App_CleanAbility_0100
 * @tc.name: App CleanAbility
 * @tc.desc: Mock appmgr and register it into the systemmanager.
 *           The Appfwk has started successfully.
 *           The application has been launched successfully.
 *           The ability has been launched successfully.
 *           Clean the ability which has been launched before.
 */
HWTEST_F(AppkitNativeModuleTestThird, App_CleanAbility_0100, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "App_CleanAbility_0100 start";
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(APP_MGR_SERVICE_ID, AppMgrObject_);
    runner_->Run();

    AppLaunchData lanchdate;
    ApplicationInfo appinf;
    ProcessInfo processing("TestProcess", TestProcessInfo);
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    Configuration config;
    mockAppMgr->ScheduleLaunchApplication(lanchdate, config);
    WaitUntilTaskDone(mockHandler_);

    AbilityInfo abilityinf;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    mockAppMgr->ScheduleLaunchAbility(abilityinf, token, nullptr);
    WaitUntilTaskDone(mockHandler_);

    mockAppMgr->ScheduleCleanAbility(token);
    WaitUntilTaskDone(mockHandler_);

    runner_->Stop();
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->UnregisterSystemAbility(APP_MGR_SERVICE_ID);
    GTEST_LOG_(INFO) << "App_CleanAbility_0100 end";
}

/**
 * @tc.number: App_CleanAbility_0200
 * @tc.name: App CleanAbility
 * @tc.desc: Mock appmgr and register it into the systemmanager.
 *           The Appfwk has started successfully.
 *           The application has been launched successfully.
 *           The application has not been launched successfully.
 *           Clean the ability before launch the application.
 */
HWTEST_F(AppkitNativeModuleTestThird, App_CleanAbility_0200, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "App_CleanAbility_0200 start";
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(APP_MGR_SERVICE_ID, AppMgrObject_);
    runner_->Run();

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    mockAppMgr->ScheduleCleanAbility(token);
    WaitUntilTaskDone(mockHandler_);

    runner_->Stop();
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->UnregisterSystemAbility(APP_MGR_SERVICE_ID);
    GTEST_LOG_(INFO) << "App_CleanAbility_0200 end";
}

/**
 * @tc.number: App_CleanAbility_0300
 * @tc.name: App CleanAbility
 * @tc.desc: Mock appmgr and register it into the systemmanager.
 *           The Appfwk has started successfully.
 *           The application has been launched successfully.
 *           The ability has not been launched successfully.
 *           Clean the ability before launch the ability.
 */
HWTEST_F(AppkitNativeModuleTestThird, App_CleanAbility_0300, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "App_CleanAbility_0300 start";
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(APP_MGR_SERVICE_ID, AppMgrObject_);
    runner_->Run();

    AppLaunchData lanchdate;
    ApplicationInfo appinf;
    ProcessInfo processing("TestProcess", TestProcessInfo);
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    Configuration config;
    mockAppMgr->ScheduleLaunchApplication(lanchdate, config);
    WaitUntilTaskDone(mockHandler_);

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    mockAppMgr->ScheduleCleanAbility(token);
    WaitUntilTaskDone(mockHandler_);

    runner_->Stop();
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->UnregisterSystemAbility(APP_MGR_SERVICE_ID);
    GTEST_LOG_(INFO) << "App_CleanAbility_0300 end";
}

/**
 * @tc.number: App_CleanAbility_0400
 * @tc.name: App CleanAbility
 * @tc.desc: Mock appmgr and register it into the systemmanager.
 *           The Appfwk has started successfully.
 *           The application has been launched successfully.
 *           The ability has been launched successfully.
 *           Clean the ability with a null token.
 */
HWTEST_F(AppkitNativeModuleTestThird, App_CleanAbility_0400, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "App_CleanAbility_0400 start";
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(APP_MGR_SERVICE_ID, AppMgrObject_);
    runner_->Run();

    AppLaunchData lanchdate;
    ApplicationInfo appinf;
    ProcessInfo processing("TestProcess", TestProcessInfo);
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    Configuration config;
    mockAppMgr->ScheduleLaunchApplication(lanchdate, config);
    WaitUntilTaskDone(mockHandler_);

    AbilityInfo abilityinf;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    mockAppMgr->ScheduleLaunchAbility(abilityinf, token, nullptr);
    WaitUntilTaskDone(mockHandler_);

    mockAppMgr->ScheduleCleanAbility(nullptr);
    WaitUntilTaskDone(mockHandler_);

    runner_->Stop();
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->UnregisterSystemAbility(APP_MGR_SERVICE_ID);
    GTEST_LOG_(INFO) << "App_CleanAbility_0400 end";
}

/**
 * @tc.number: App_CleanAbility_0500
 * @tc.name: App CleanAbility
 * @tc.desc: Mock appmgr and register it into the systemmanager.
 *           The Appfwk has started successfully.
 *           The application has been launched successfully.
 *           The ability has been launched successfully.
 *           Clean the ability with a wrong token.
 */
HWTEST_F(AppkitNativeModuleTestThird, App_CleanAbility_0500, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "App_CleanAbility_0500 start";
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(APP_MGR_SERVICE_ID, AppMgrObject_);
    runner_->Run();

    AppLaunchData lanchdate;
    ApplicationInfo appinf;
    ProcessInfo processing("TestProcess", TestProcessInfo);
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    Configuration config;
    mockAppMgr->ScheduleLaunchApplication(lanchdate, config);
    WaitUntilTaskDone(mockHandler_);

    AbilityInfo abilityinf;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    mockAppMgr->ScheduleLaunchAbility(abilityinf, token, nullptr);
    WaitUntilTaskDone(mockHandler_);
    sptr<IRemoteObject> tokenOhter = new (std::nothrow) MockAbilityToken();
    mockAppMgr->ScheduleCleanAbility(tokenOhter);
    WaitUntilTaskDone(mockHandler_);

    runner_->Stop();
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->UnregisterSystemAbility(APP_MGR_SERVICE_ID);
    GTEST_LOG_(INFO) << "App_CleanAbility_0500 end";
}

/**
 * @tc.number: App_ElementsCallbacks_0100
 * @tc.name: Application ElementsCallbacks
 * @tc.desc: Mock application Mock appmgr and register it into the systemmanager.
 *           The Appfwk has started successfully.
 *           The Appfwk has started successfully.
 *           Verifying whether ElementsCallbacks registration, unregister, and its observer mechanism are valid.
 */
HWTEST_F(AppkitNativeModuleTestThird, App_ElementsCallbacks_0100, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "App_ElementsCallbacks_0100 start";
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(APP_MGR_SERVICE_ID, AppMgrObject_);
    runner_->Run();

    AppLaunchData lanchdate;
    ApplicationInfo appinf;
    ProcessInfo processing("TestProcess", 9997);
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    Configuration config;
    mockAppMgr->ScheduleLaunchApplication(lanchdate, config);
    WaitUntilTaskDone(mockHandler_);

    mockAppMgr->ScheduleShrinkMemory(10);
    WaitUntilTaskDone(mockHandler_);

    config.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "ZH-HANS");

    mockAppMgr->ScheduleConfigurationUpdated(config);
    WaitUntilTaskDone(mockHandler_);

    mockAppMgr->ScheduleTerminateApplication();
    WaitUntilTaskDone(mockHandler_);

    runner_->Stop();
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->UnregisterSystemAbility(APP_MGR_SERVICE_ID);
    GTEST_LOG_(INFO) << "App_ElementsCallbacks_0100 end";
}

/**
 * @tc.number: App_AbilityLifecycleCallbacks_0100
 * @tc.name: Application AbilityLifecycleCallbacks
 * @tc.desc: Mock ability
 *           Mock application
 *           Mock appmgr and register it into the systemmanager.
 *           The Appfwk has started successfully.
 *           The application has been launched successfully.
 *           The ability has been launched successfully.
 *           Verifying whether AbilityLifecycleCallbacks registration, unregister, and its observer mechanism are
 valid.
 */
HWTEST_F(AppkitNativeModuleTestThird, App_AbilityLifecycleCallbacks_0100, Function | MediumTest | Level2)
{
    GTEST_LOG_(INFO) << "App_AbilityLifecycleCallbacks_0100 start";
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(APP_MGR_SERVICE_ID, AppMgrObject_);
    runner_->Run();

    AppLaunchData lanchdate;
    ApplicationInfo appinf;
    ProcessInfo processing("TestProcess", 9996);
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    Configuration config;
    mockAppMgr->ScheduleLaunchApplication(lanchdate, config);
    WaitUntilTaskDone(mockHandler_);

    runner_->Stop();
    OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->UnregisterSystemAbility(APP_MGR_SERVICE_ID);
    GTEST_LOG_(INFO) << "App_AbilityLifecycleCallbacks_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS