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

#define private public
#define protected public
#include "sa_mgr_client.h"
#include "pending_want_manager.h"
#include "app_scheduler.h"
#include "ability_manager_service.h"
#include "want_agent_helper.h"
#include "ability_manager_client.h"
#undef private
#undef protected

#include "context/context.h"
#include "system_ability_definition.h"
#include "mock_ability_scheduler.h"
#include "mock_app_mgr_client.h"
#include "pending_want.h"
#include "mock_want_receiver.h"
#include "mock_bundle_mgr.h"
#include "ability_context.h"
#include "mock_compled_callback.h"
#include "mock_ability_mgr_service.h"
#include "os_account_manager.h"

using namespace OHOS::AbilityRuntime::WantAgent;
using namespace OHOS::AppExecFwk;

using namespace testing;
using testing::_;
using testing::Invoke;
using testing::Return;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
static const int FLAG_ONE_SHOT = 1 << 30;
int abilityRequestCode = 1;
int abilitiesRequestCode = 2;
int serviceRequestCode = 3;
int foregroundServicesRequestCode = 4;
int commonEventRequestCode = 4;
}  // namespace
class PandingWantManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    WantSenderInfo GetAbility();
    WantSenderInfo GetAbilities();
    WantSenderInfo GetService();
    WantSenderInfo GetForegroundService();
    WantSenderInfo GetCommonEvent();
    std::shared_ptr<AAFwk::Want> GetWant(std::string abilityName, std::string bundleName);
    WantAgentInfo MakeWantAgentInfo(WantAgentConstant::OperationType &type, int requestCode,
        std::vector<WantAgentConstant::Flags> &flags, std::vector<std::shared_ptr<AAFwk::Want>> wants);

public:
    MockWantReceiver *receiver = new MockWantReceiver();
    inline static MockAppMgrClient *appClient = nullptr;
    inline static std::shared_ptr<AbilityManagerService> abilityManager = nullptr;
    MockAbilityMgrService *amsSerice = new MockAbilityMgrService();
};

void PandingWantManagerTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new (std::nothrow) BundleMgrService());

    abilityManager = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    abilityManager->OnStart();

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    appClient = new MockAppMgrClient();
    if (appClient) {
        appScheduler->appMgrClient_.reset(appClient);
        GTEST_LOG_(INFO) << "mock appMgrClient_ ok";
    }
}

void PandingWantManagerTest::TearDownTestCase(void)
{
    abilityManager->OnStop();
    if (appClient) {
        delete appClient;
        appClient = nullptr;
    }
}

void PandingWantManagerTest::SetUp()
{
    if (amsSerice == nullptr) {
        amsSerice = new MockAbilityMgrService();
    }
    AbilityManagerClient::GetInstance();
    AbilityManagerClient::GetInstance()->remoteObject_ = amsSerice;
}

void PandingWantManagerTest::TearDown()
{}

std::shared_ptr<AAFwk::Want> PandingWantManagerTest::GetWant(std::string abilityName, std::string bundleName)
{
    if (abilityName == "") {
        abilityName = "hiMusic";
    }
    if (bundleName == "") {
        bundleName = "com.ix.hiMusic";
    }

    ElementName element;
    element.SetDeviceID("devices");
    element.SetAbilityName(abilityName);
    element.SetBundleName(bundleName);
    Want want;
    want.SetElement(element);
    return std::make_shared<Want>(want);
}

WantSenderInfo PandingWantManagerTest::GetAbility()
{
    int32_t flags = FLAG_ONE_SHOT;
    WantsInfo wantsInfo;
    wantsInfo.want = *(GetWant("hiMusic", "com.ix.hiMusic"));
    wantsInfo.resolvedTypes = wantsInfo.want.GetType();

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = (int32_t)WantAgentConstant::OperationType::START_ABILITY;
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = "ix.com.hiMusic";
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = 0;
    wantSenderInfo.requestCode = abilityRequestCode;
    wantSenderInfo.resultWho = "GetAbility";
    GTEST_LOG_(INFO) << "GetAbility";
    return wantSenderInfo;
}

WantSenderInfo PandingWantManagerTest::GetAbilities()
{
    int32_t flags = (int32_t)FLAG_ONE_SHOT;

    WantsInfo wantsInfo;
    wantsInfo.want = *(GetWant("hiMusic", "com.ix.hiMusic"));
    wantsInfo.resolvedTypes = wantsInfo.want.GetType();

    WantsInfo wantsInfo2;
    wantsInfo2.want = *(GetWant("hiRadio", "com.ix.hiRadio"));
    wantsInfo2.resolvedTypes = wantsInfo2.want.GetType();

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = (int32_t)WantAgentConstant::OperationType::START_ABILITIES;
    wantSenderInfo.bundleName = "ix.com.hiRadio";
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = 0;
    wantSenderInfo.requestCode = abilitiesRequestCode;
    wantSenderInfo.resultWho = "GetAbilities";
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.allWants.push_back(wantsInfo2);
    GTEST_LOG_(INFO) << "GetAbilities";
    return wantSenderInfo;
}

WantSenderInfo PandingWantManagerTest::GetService()
{
    int32_t flags = FLAG_ONE_SHOT;

    WantsInfo wantsInfo;
    wantsInfo.want = *(GetWant("hiService", "com.ix.hiService"));
    wantsInfo.resolvedTypes = wantsInfo.want.GetType();

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = (int32_t)WantAgentConstant::OperationType::START_SERVICE;
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = "com.ix.hiService";
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = 0;
    wantSenderInfo.requestCode = serviceRequestCode;
    wantSenderInfo.resultWho = "GetService";
    GTEST_LOG_(INFO) << "GetService";
    return wantSenderInfo;
}

WantSenderInfo PandingWantManagerTest::GetForegroundService()
{
    int32_t flags = FLAG_ONE_SHOT;

    WantsInfo wantsInfo;
    wantsInfo.want = *(GetWant("hiService", "com.ix.hiService"));
    wantsInfo.resolvedTypes = wantsInfo.want.GetType();

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = (int32_t)WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = "ix.com.hiworld";
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = 0;
    wantSenderInfo.requestCode = foregroundServicesRequestCode;
    GTEST_LOG_(INFO) << "GetForegroundService";
    return wantSenderInfo;
}

WantSenderInfo PandingWantManagerTest::GetCommonEvent()
{
    int32_t flags = FLAG_ONE_SHOT;

    WantsInfo wantsInfo;
    wantsInfo.want = *(GetWant("hiMusic", "com.ix.hiMusic"));
    wantsInfo.resolvedTypes = wantsInfo.want.GetType();

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = (int32_t)WantAgentConstant::OperationType::SEND_COMMON_EVENT;
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = "ix.com.hiworld";
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = 0;
    wantSenderInfo.requestCode = commonEventRequestCode;
    GTEST_LOG_(INFO) << "GetCommonEvent";
    return wantSenderInfo;
}

WantAgentInfo PandingWantManagerTest::MakeWantAgentInfo(WantAgentConstant::OperationType &type, int requestCode,
    std::vector<WantAgentConstant::Flags> &flags, std::vector<std::shared_ptr<AAFwk::Want>> wants)
{
    WantAgentInfo info;
    info.operationType_ = type;
    info.requestCode_ = requestCode;
    info.flags_ = flags;
    info.wants_ = wants;
    info.extraInfo_ = nullptr;
    return info;
}

/*
 * Feature: AaFwk
 * Function: GetWantSender
 * SubFunction: start page abilites
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: get want sender / send want sender
 */
HWTEST_F(PandingWantManagerTest, pending_want_mgr_test_002, TestSize.Level1)
{
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_ABILITIES;
    int requsetCode = 10;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::ONE_TIME_FLAG;
    WantAgentConstant::Flags flag1 = WantAgentConstant::Flags::ONE_TIME_FLAG;
    std::vector<WantAgentConstant::Flags> flags;
    flags.push_back(flag);
    flags.push_back(flag1);

    auto abilityWant = GetWant("hiMusic", "com.ix.hiMusic");
    auto abilityWant1 = GetWant("hiRedio", "com.ix.hiRedio");
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(abilityWant);
    wants.push_back(abilityWant1);

    WantAgentInfo info = MakeWantAgentInfo(type, requsetCode, flags, wants);
    std::shared_ptr<AbilityRuntime::Context> context = OHOS::AbilityRuntime::Context::GetApplicationContext();

    // proxy start
    auto amsProxyGetWantSenderReturn = [&](const WantSenderInfo &wantSenderInfo,
                                           const sptr<IRemoteObject> &callerToken) {
        EXPECT_EQ(wantSenderInfo.type, (int32_t)WantAgentConstant::OperationType::START_ABILITIES);
        return abilityManager->GetWantSender(wantSenderInfo, callerToken);
    };
    EXPECT_CALL(*amsSerice, GetWantSender(_, _)).Times(1).WillOnce(Invoke(amsProxyGetWantSenderReturn));

    auto amsProxySendWantSenderReturn = [&](const sptr<IWantSender> &target, const SenderInfo &senderInfo) {
        return abilityManager->SendWantSender(target, senderInfo);
    };
    EXPECT_CALL(*amsSerice, SendWantSender(_, _)).Times(1).WillOnce(Invoke(amsProxySendWantSenderReturn));
    EXPECT_CALL(*amsSerice, GetPendingWantType(_)).Times(1).WillOnce(Return(0));
    // proxy end

    // call GetWantAgent
    std::shared_ptr<WantAgent> wantAgent = WantAgentHelper::GetWantAgent(context, info);
    EXPECT_NE(wantAgent, nullptr);
    auto pandingWant = wantAgent->GetPendingWant();
    EXPECT_NE(pandingWant, nullptr);

    std::shared_ptr<CompletedCallback> callback;
    MockCompletedCallback *call = new MockCompletedCallback();
    callback.reset(call);

    TriggerInfo paramsInfo("", nullptr, abilityWant, 11);

    EXPECT_CALL(*call, OnSendFinished(_, _, _, _)).Times(1);

    auto stackMgr = abilityManager->GetStackManager();
    EXPECT_NE(stackMgr, nullptr);
    auto topAbility = stackMgr->GetCurrentTopAbility();
    EXPECT_NE(topAbility, nullptr);
    topAbility->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    WantAgentHelper::TriggerWantAgent(wantAgent, callback, paramsInfo);
}

/*
 * Feature: AaFwk
 * Function: GetWantSender
 * SubFunction: get foreground service ability
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: get want sender / send want sender
 */
HWTEST_F(PandingWantManagerTest, pending_want_mgr_test_005, TestSize.Level1)
{
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::SEND_COMMON_EVENT;
    int requsetCode = 112;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::ONE_TIME_FLAG;
    std::vector<WantAgentConstant::Flags> flags;
    flags.push_back(flag);

    auto abilityWant = GetWant("hiRedio", "com.ix.hiRedio");
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(abilityWant);

    WantAgentInfo info = MakeWantAgentInfo(type, requsetCode, flags, wants);
    std::shared_ptr<AbilityRuntime::Context> context = OHOS::AbilityRuntime::Context::GetApplicationContext();

    // proxy start
    auto amsProxyGetWantSenderReturn = [&](const WantSenderInfo &wantSenderInfo,
                                           const sptr<IRemoteObject> &callerToken) {
        EXPECT_EQ(wantSenderInfo.type, (int32_t)WantAgentConstant::OperationType::SEND_COMMON_EVENT);
        return abilityManager->GetWantSender(wantSenderInfo, callerToken);
    };
    EXPECT_CALL(*amsSerice, GetWantSender(_, _)).Times(1).WillOnce(Invoke(amsProxyGetWantSenderReturn));

    auto amsProxySendWantSenderReturn = [&](const sptr<IWantSender> &target, const SenderInfo &senderInfo) {
        return abilityManager->SendWantSender(target, senderInfo);
    };
    EXPECT_CALL(*amsSerice, SendWantSender(_, _)).Times(1).WillOnce(Invoke(amsProxySendWantSenderReturn));
    EXPECT_CALL(*amsSerice, GetPendingWantType(_)).Times(1).WillOnce(Return(0));

    // proxy end
    // call GetWantAgent
    std::shared_ptr<WantAgent> wantAgent = WantAgentHelper::GetWantAgent(context, info);
    EXPECT_NE(wantAgent, nullptr);
    auto pandingWant = wantAgent->GetPendingWant();
    EXPECT_NE(pandingWant, nullptr);

    std::shared_ptr<CompletedCallback> callback;
    MockCompletedCallback *call = new MockCompletedCallback();
    callback.reset(call);

    TriggerInfo paramsInfo("", nullptr, abilityWant, 11);

    EXPECT_CALL(*call, OnSendFinished(_, _, _, _)).Times(1);
    WantAgentHelper::TriggerWantAgent(wantAgent, callback, paramsInfo);
}

/*
 * Feature: AaFwk
 * Function: GetWantSender
 * SubFunction: get ability info
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: get want sender / send want sender
 */
HWTEST_F(PandingWantManagerTest, pending_want_mgr_test_006, TestSize.Level1)
{
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_ABILITY;
    int requsetCode = 18;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::UPDATE_PRESENT_FLAG;
    std::vector<WantAgentConstant::Flags> flags;
    flags.push_back(flag);

    auto abilityWant = GetWant("hiMusic", "com.ix.hiMusic");
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(abilityWant);

    WantAgentInfo info = MakeWantAgentInfo(type, requsetCode, flags, wants);
    std::shared_ptr<AbilityRuntime::Context> context = OHOS::AbilityRuntime::Context::GetApplicationContext();

    // proxy start
    auto amsProxyGetWantSenderReturn = [&](const WantSenderInfo &wantSenderInfo,
                                           const sptr<IRemoteObject> &callerToken) {
        EXPECT_EQ(wantSenderInfo.type, (int32_t)WantAgentConstant::OperationType::START_ABILITY);
        return abilityManager->GetWantSender(wantSenderInfo, callerToken);
    };
    EXPECT_CALL(*amsSerice, GetWantSender(_, _)).Times(1).WillOnce(Invoke(amsProxyGetWantSenderReturn));

    auto amsProxyGetPendingWantType =
        [&](const sptr<IWantSender> &target) { return abilityManager->GetPendingWantType(target); };
    EXPECT_CALL(*amsSerice, GetPendingWantType(_)).Times(1).WillOnce(Invoke(amsProxyGetPendingWantType));

    auto amsProxyGetPendingWantCode =
        [&](const sptr<IWantSender> &target) { return abilityManager->GetPendingWantCode(target); };
    EXPECT_CALL(*amsSerice, GetPendingWantCode(_)).Times(1).WillOnce(Invoke(amsProxyGetPendingWantCode));

    auto amsProxyRegisterCancelListener = [&](const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier) {
        return abilityManager->RegisterCancelListener(sender, recevier);
    };
    EXPECT_CALL(*amsSerice, RegisterCancelListener(_, _)).Times(1).WillOnce(Invoke(amsProxyRegisterCancelListener));

    auto amsProxyCancelWantSender =
        [&](const sptr<IWantSender> &sender) { return abilityManager->CancelWantSender(sender); };
    EXPECT_CALL(*amsSerice, CancelWantSender(_)).Times(1).WillOnce(Invoke(amsProxyCancelWantSender));
    // proxy end

    // call GetWantAgent
    std::shared_ptr<WantAgent> wantAgent = WantAgentHelper::GetWantAgent(context, info);
    EXPECT_NE(wantAgent, nullptr);
    auto pandingWant = wantAgent->GetPendingWant();
    EXPECT_NE(pandingWant, nullptr);

    auto getType = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(getType, WantAgentConstant::OperationType::START_ABILITY);

    int code = WantAgentHelper::GetHashCode(wantAgent);
    EXPECT_NE(code, -1);

    // mock CancelListener
    auto cancalCall = [&](int resultCode) -> void { EXPECT_EQ(resultCode, requsetCode); };

    std::shared_ptr<MockCancelListener> listener = std::make_shared<MockCancelListener>();
    EXPECT_CALL(*listener, OnCancelled(_)).Times(1).WillOnce(Invoke(cancalCall));
    WantAgentHelper::RegisterCancelListener(listener, wantAgent);
    WantAgentHelper::Cancel(wantAgent);
}

/*
 * Feature: AaFwk
 * Function: GetWantSender
 * SubFunction: start a page ability
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: get want sender different flag effects
 */
HWTEST_F(PandingWantManagerTest, pending_want_mgr_test_007, TestSize.Level1)
{
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_ABILITY;
    int requsetCode = 21;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::NO_BUILD_FLAG;
    std::vector<WantAgentConstant::Flags> flags;
    flags.push_back(flag);

    auto abilityWant = GetWant("Netease music", "com.ix.hiMusic");
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(abilityWant);

    WantAgentInfo info = MakeWantAgentInfo(type, requsetCode, flags, wants);
    std::shared_ptr<AbilityRuntime::Context> context = OHOS::AbilityRuntime::Context::GetApplicationContext();

    // proxy start
    auto amsProxyGetWantSenderReturn = [&](const WantSenderInfo &wantSenderInfo,
                                           const sptr<IRemoteObject> &callerToken) {
        EXPECT_EQ(wantSenderInfo.type, (int32_t)WantAgentConstant::OperationType::START_ABILITY);
        return abilityManager->GetWantSender(wantSenderInfo, callerToken);
    };
    EXPECT_CALL(*amsSerice, GetWantSender(_, _))
        .Times(4)
        .WillOnce(Invoke(amsProxyGetWantSenderReturn))
        .WillOnce(Invoke(amsProxyGetWantSenderReturn))
        .WillOnce(Invoke(amsProxyGetWantSenderReturn))
        .WillOnce(Invoke(amsProxyGetWantSenderReturn));

    auto amsProxyRegisterCancelListener = [&](const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier) {
        return abilityManager->RegisterCancelListener(sender, recevier);
    };
    EXPECT_CALL(*amsSerice, RegisterCancelListener(_, _)).Times(1).WillOnce(Invoke(amsProxyRegisterCancelListener));
    // proxy end

    // call GetWantAgent
    std::shared_ptr<WantAgent> wantAgent = WantAgentHelper::GetWantAgent(context, info);
    // must be nullptr
    EXPECT_EQ(wantAgent->GetPendingWant()->GetTarget(), nullptr);

    WantAgentConstant::Flags flag1 = WantAgentConstant::Flags::ONE_TIME_FLAG;
    flags.clear();
    flags.push_back(flag1);
    info = MakeWantAgentInfo(type, requsetCode, flags, wants);

    std::shared_ptr<WantAgent> wantAgent1 = WantAgentHelper::GetWantAgent(context, info);
    EXPECT_NE(wantAgent1->GetPendingWant()->GetTarget(), nullptr);
    auto pandingWant1 = wantAgent1->GetPendingWant();
    EXPECT_NE(pandingWant1, nullptr);

    // update
    WantAgentConstant::Flags flag2 = WantAgentConstant::Flags::UPDATE_PRESENT_FLAG;
    flags.clear();
    flags.push_back(flag2);

    requsetCode = 24;
    abilityWant = GetWant("redio", "com.ix.hiRedio");
    wants.clear();
    wants.push_back(abilityWant);
    info = MakeWantAgentInfo(type, requsetCode, flags, wants);

    std::shared_ptr<WantAgent> wantAgent2 = WantAgentHelper::GetWantAgent(context, info);
    EXPECT_NE(wantAgent2->GetPendingWant()->GetTarget(), nullptr);
    auto pandingWant2 = wantAgent2->GetPendingWant();
    EXPECT_NE(pandingWant2, nullptr);

    auto target = pandingWant2->GetTarget();
    EXPECT_NE(target, nullptr);
    sptr<PendingWantRecord> targetRecord = iface_cast<PendingWantRecord>(target->AsObject());

    auto requestWant = targetRecord->GetKey()->GetRequestWant();
    EXPECT_EQ(requestWant.GetElement().GetAbilityName(), "redio");
    EXPECT_EQ(requestWant.GetElement().GetBundleName(), "com.ix.hiRedio");

    // cancal
    WantAgentConstant::Flags flag3 = WantAgentConstant::Flags::CANCEL_PRESENT_FLAG;
    flags.clear();
    flags.push_back(flag3);

    info = MakeWantAgentInfo(type, requsetCode, flags, wants);

    // mock CancelListener
    auto cancalCall = [&](int resultCode) -> void { EXPECT_EQ(resultCode, requsetCode); };

    std::shared_ptr<MockCancelListener> listener = std::make_shared<MockCancelListener>();
    EXPECT_CALL(*listener, OnCancelled(_)).Times(1).WillOnce(Invoke(cancalCall));
    WantAgentHelper::RegisterCancelListener(listener, wantAgent2);

    std::shared_ptr<WantAgent> wantAgent3 = WantAgentHelper::GetWantAgent(context, info);
    EXPECT_EQ(wantAgent3->GetPendingWant()->GetTarget(), nullptr);
}
}  // namespace AAFwk
}  // namespace OHOS