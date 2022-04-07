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
#include "ability_manager_proxy.h"
#include "ability_manager_stub_mock.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "ability_scheduler_mock.h"
#include "ability_record.h"
#include "ability_scheduler.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
}  // namespace

class AbilityManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerProxy> proxy_ {nullptr};
    sptr<AbilityManagerStubMock> mock_ {nullptr};
};

void AbilityManagerProxyTest::SetUpTestCase(void)
{}
void AbilityManagerProxyTest::TearDownTestCase(void)
{}
void AbilityManagerProxyTest::TearDown()
{}

void AbilityManagerProxyTest::SetUp()
{
    mock_ = new AbilityManagerStubMock();
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
}

/**
 * @tc.name: AbilityManagerProxy_DumpSysState_0100
 * @tc.desc: DumpSysState
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DumpSysState_0100, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerProxy_DumpSysState_0100 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    std::string args;
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;

    proxy_->DumpSysState(args, info, isClient, isUserID, USER_ID);
    EXPECT_EQ(IAbilityManager::DUMPSYS_STATE, mock_->code_);

    HILOG_INFO("AbilityManagerProxy_DumpSysState_0100 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of startability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    auto res = proxy_->StartAbility(want, 9);

    EXPECT_EQ(IAbilityManager::START_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the return value of startability is abnormal
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    auto res = proxy_->StartAbility(want, 9);

    EXPECT_EQ(IAbilityManager::START_ABILITY, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of TerminateAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_003, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->TerminateAbility(token, -1, &want);

    EXPECT_EQ(IAbilityManager::TERMINATE_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the return value of TerminateAbility is abnormal
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_004, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->TerminateAbility(token, -1, &want);

    EXPECT_EQ(IAbilityManager::TERMINATE_ABILITY, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of connectability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_005, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    want.SetFlags(10);
    sptr<IAbilityConnection> nullConnect = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    int res = proxy_->ConnectAbility(want, nullConnect, callerToken);
    EXPECT_NE(res, NO_ERROR);
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res1 = proxy_->ConnectAbility(want, connect, callerToken);
    EXPECT_EQ(res1, NO_ERROR);
    EXPECT_EQ(IAbilityManager::CONNECT_ABILITY, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of connectability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_006, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->ConnectAbility(want, connect, callerToken);

    EXPECT_EQ(IAbilityManager::CONNECT_ABILITY, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of disconnectAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_007, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->DisconnectAbility(connect);

    EXPECT_EQ(IAbilityManager::DISCONNECT_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of disconnectAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_008, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->DisconnectAbility(connect);

    EXPECT_EQ(IAbilityManager::DISCONNECT_ABILITY, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of attachAbilityThread
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_009, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IAbilityScheduler> scheduler = new AbilitySchedulerMock();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->AttachAbilityThread(scheduler, token);

    EXPECT_EQ(IAbilityManager::ATTACH_ABILITY_THREAD, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of attachAbilityThread
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_010, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IAbilityScheduler> scheduler = new AbilitySchedulerMock();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->AttachAbilityThread(scheduler, token);

    EXPECT_EQ(IAbilityManager::ATTACH_ABILITY_THREAD, mock_->code_);
    EXPECT_NE(res, NO_ERROR);

    sptr<IAbilityScheduler> nullScheduler = nullptr;
    auto res1 = proxy_->AttachAbilityThread(nullScheduler, token);
    EXPECT_NE(res1, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of abilityTransitionDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0011, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    PacMap saveData;
    auto res = proxy_->AbilityTransitionDone(token, 1, saveData);

    EXPECT_EQ(IAbilityManager::ABILITY_TRANSITION_DONE, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDone
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of abilityTransitionDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_012, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    PacMap saveData;
    auto res = proxy_->AbilityTransitionDone(token, 1, saveData);

    EXPECT_EQ(IAbilityManager::ABILITY_TRANSITION_DONE, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of scheduleConnectAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0013, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto res = proxy_->ScheduleConnectAbilityDone(token, remoteObject);

    EXPECT_EQ(IAbilityManager::CONNECT_ABILITY_DONE, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of scheduleConnectAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_014, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto res = proxy_->ScheduleConnectAbilityDone(token, remoteObject);

    EXPECT_EQ(IAbilityManager::CONNECT_ABILITY_DONE, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleDisconnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleDisconnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of scheduleDisconnectAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0015, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto res = proxy_->ScheduleDisconnectAbilityDone(token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpState
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of dumpState
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0017, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string args = "aaa";
    std::vector<std::string> info;
    proxy_->DumpState(args, info);

    EXPECT_EQ(IAbilityManager::DUMP_STATE, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityResult
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of terminateAbilityResult
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0018, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->TerminateAbilityResult(token, 1);

    EXPECT_EQ(IAbilityManager::TERMINATE_ABILITY_RESULT, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityResult
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of terminateAbilityResult
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_019, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->TerminateAbilityResult(token, 1);

    EXPECT_EQ(IAbilityManager::TERMINATE_ABILITY_RESULT, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of ScheduleCommandAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_026, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->ScheduleCommandAbilityDone(token);

    EXPECT_EQ(IAbilityManager::COMMAND_ABILITY_DONE, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StopServiceAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_027, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    auto res = proxy_->StopServiceAbility(want);

    EXPECT_EQ(IAbilityManager::STOP_SERVICE_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_028
 * @tc.desc: test StartContinuation send request succeeded
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_028, TestSize.Level0)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int res = proxy_->StartContinuation(want, abilityToken, 0);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(IAbilityManager::START_CONTINUATION, mock_->code_);
}

/**
 * @tc.name: AbilityManagerProxy_029
 * @tc.desc: test StartContinuation send request failed
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_029, TestSize.Level0)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int res = proxy_->StartContinuation(want, abilityToken, 0);

    EXPECT_EQ(IAbilityManager::START_CONTINUATION, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_030
 * @tc.desc: test NotifyContinuationResult send request succeeded
 * @tc.type: FUNC
 * @tc.require: AR000GI8IH
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_030, TestSize.Level0)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t result = 0;
    int res = proxy_->NotifyContinuationResult(0, result);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(IAbilityManager::NOTIFY_CONTINUATION_RESULT, mock_->code_);
}

/**
 * @tc.name: AbilityManagerProxy_031
 * @tc.desc: test NotifyContinuationResult send request failed
 * @tc.type: FUNC
 * @tc.require: AR000GI8IH
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_031, TestSize.Level0)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = 0;
    int res = proxy_->NotifyContinuationResult(0, result);

    EXPECT_EQ(IAbilityManager::NOTIFY_CONTINUATION_RESULT, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbility normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_AcquireDataAbility_001, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->AcquireDataAbility(dataAbilityUri, true, abilityRecord->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbility callerToken is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_AcquireDataAbility_002, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    EXPECT_EQ(proxy_->AcquireDataAbility(dataAbilityUri, true, nullptr), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbility SendRequest return error.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_AcquireDataAbility_003, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(TRANSACTION_ERR));
    EXPECT_EQ(proxy_->AcquireDataAbility(dataAbilityUri, true, abilityRecord->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_001, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->ReleaseDataAbility(scheduler, abilityRecord->GetToken()), 0);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility dataAbilityScheduler is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_002, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    EXPECT_EQ(proxy_->ReleaseDataAbility(nullptr, abilityRecord->GetToken()), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility callerToken is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_003, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    EXPECT_EQ(proxy_->ReleaseDataAbility(scheduler, nullptr), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility SendRequest error.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_004, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(TRANSACTION_ERR));
    EXPECT_EQ(proxy_->ReleaseDataAbility(scheduler, abilityRecord->GetToken()), TRANSACTION_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCall
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByCall connect is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityByCall_001, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = nullptr;
    EXPECT_EQ(proxy_->StartAbilityByCall(want, connect, callerToken), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCall
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByCall is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityByCall_002, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    EXPECT_EQ(proxy_->StartAbilityByCall(want, connect, callerToken), ERR_OK);
    EXPECT_EQ(IAbilityManager::START_CALL_ABILITY, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseAbility connect is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseAbility_001, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    sptr<IAbilityConnection> connect = nullptr;
    EXPECT_EQ(proxy_->ReleaseAbility(connect, element), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseAbility is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseAbility_002, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    EXPECT_EQ(proxy_->ReleaseAbility(connect, element), ERR_OK);
    EXPECT_EQ(IAbilityManager::RELEASE_CALL_ABILITY, mock_->code_);
}
}  // namespace AAFwk
}  // namespace OHOS
