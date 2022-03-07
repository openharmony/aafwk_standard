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
#include "ability_record.h"
#undef private
#undef protected

#include "ability_manager_service.h"
#include "ability_scheduler.h"
#include "connection_record.h"
#include "mission_record.h"
#include "mock_ability_connect_callback.h"
#include "ability_scheduler_mock.h"
#include "ability_connect_callback_stub.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class AbilityRecordTest : public testing::TestWithParam<OHOS::AAFwk::AbilityState> {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityRecord> abilityRecord_ {nullptr};
    std::shared_ptr<AbilityResult> abilityResult_ {nullptr};
    std::shared_ptr<AbilityRequest> abilityRequest_ {nullptr};
    static constexpr unsigned int CHANGE_CONFIG_LOCALE = 0x00000001;
};

void AbilityRecordTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->OnStart();
}
void AbilityRecordTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->OnStop();
}

void AbilityRecordTest::SetUp(void)
{
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityResult_ = std::make_shared<AbilityResult>(-1, -1, want);
    abilityRequest_ = std::make_shared<AbilityRequest>();
    abilityRecord_->Init();
}

void AbilityRecordTest::TearDown(void)
{
    abilityRecord_.reset();
    abilityResult_.reset();
    abilityRequest_.reset();
}

bool IsTestAbilityExist(const std::string &data)
{
    return std::string::npos != data.find("previous ability app name [NULL]");
}

bool IsTestAbilityExist1(const std::string &data)
{
    return std::string::npos != data.find("test_pre_app");
}

bool IsTestAbilityExist2(const std::string &data)
{
    return std::string::npos != data.find("test_next_app");
}

/*
 * Feature: AbilityRecord
 * Function: IsCallType
 * SubFunction: IsCallType
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through IsCallType INVALID_TYPE
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsCallType_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRequest_->IsCallType(AbilityCallType::INVALID_TYPE), true);
}

/*
 * Feature: AbilityRecord
 * Function: IsCallType
 * SubFunction: IsCallType
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through IsCallType CALL_REQUEST_TYPE
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsCallType_002, TestSize.Level1)
{
    abilityRequest_->callType = AbilityCallType::CALL_REQUEST_TYPE;
    EXPECT_EQ(abilityRequest_->IsCallType(AbilityCallType::CALL_REQUEST_TYPE), true);
}

/*
 * Feature: AbilityRecord
 * Function: Resolve
 * SubFunction: Resolve
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through Resolve
 * ResolveResultType::NG_INNER_ERROR
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Resolve_001, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::NG_INNER_ERROR);
}

/*
 * Feature: AbilityRecord
 * Function: Resolve
 * SubFunction: Resolve
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through Resolve
 * ResolveResultType::NG_INNER_ERROR
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Resolve_002, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::NG_INNER_ERROR);
}

/*
 * Feature: AbilityRecord
 * Function: Resolve
 * SubFunction: Resolve
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through Resolve
 * ResolveResultType::NG_INNER_ERROR
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Resolve_003, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::NG_INNER_ERROR);
}

/*
 * Feature: AbilityRecord
 * Function: Resolve
 * SubFunction: Resolve
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through Resolve
 * ResolveResultType::OK_HAS_REMOTE_OBJ
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Resolve_004, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::REQUESTING);
    abilityRecord_->callContainer_ = std::make_shared<CallContainer>();
    abilityRecord_->callContainer_->AddCallRecord(abilityRequest.connect, callRecord);
    class AbilitySchedulerMockFunction : public AbilitySchedulerMock
    {
        public:
            virtual sptr<IRemoteObject> CallRequest() override
            {
                return sptr<IRemoteObject>(this);
            }
    };

    OHOS::sptr<AbilitySchedulerMockFunction> scheduler = new AbilitySchedulerMockFunction();
    sptr<IRemoteObject> object = scheduler->CallRequest();
    abilityRecord_->callContainer_->CallRequestDone(object);
    callRecord->SetCallState(CallState::REQUESTED);
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::OK_HAS_REMOTE_OBJ);
}

/*
 * Feature: AbilityRecord
 * Function: Resolve
 * SubFunction: Resolve
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through Resolve
 * ResolveResultType::OK_NO_REMOTE_OBJ
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Resolve_005, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::OK_NO_REMOTE_OBJ);
}

/*
 * Feature: AbilityRecord
 * Function: Release
 * SubFunction: Release
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through Release false
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Release_001, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityRecord_->Release(connect), false);
}

/*
 * Feature: AbilityRecord
 * Function: Release
 * SubFunction: Release
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through Release false
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Release_002, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = connect;
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    abilityRecord_->callContainer_ = std::make_shared<CallContainer>();
    abilityRecord_->callContainer_->AddCallRecord(abilityRequest.connect, callRecord);
    EXPECT_EQ(abilityRecord_->Release(connect), true);
}

/*
 * Feature: AbilityRecord
 * Function: IsStartedByCall
 * SubFunction: IsStartedByCall
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through IsStartedByCall false
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsStartedByCall_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRecord_->IsStartedByCall(), false);
}

/*
 * Feature: AbilityRecord
 * Function: SetStartedByCall
 * SubFunction: SetStartedByCall
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through IsStartedByCall true
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetStartedByCall_001, TestSize.Level1)
{
    abilityRecord_->SetStartedByCall(true);
    EXPECT_EQ(abilityRecord_->IsStartedByCall(), true);
}

/*
 * Feature: AbilityRecord
 * Function: CallRequest
 * SubFunction: CallRequest
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through Release success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CallRequest_001, TestSize.Level1)
{
    class AbilitySchedulerMockFunction : public AbilitySchedulerMock
    {
        virtual sptr<IRemoteObject> CallRequest() override
        {
            return sptr<IRemoteObject>(this);
        }
    };

    OHOS::sptr<AbilitySchedulerMockFunction> scheduler = new AbilitySchedulerMockFunction();
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    EXPECT_EQ(false, abilityRecord_->IsReady());
    abilityRecord_->SetScheduler(scheduler);

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::OK_NO_REMOTE_OBJ);
    EXPECT_EQ(abilityRecord_->CallRequest(), true);
}

/*
 * Feature: AbilityRecord
 * Function: CallRequest
 * SubFunction: CallRequest
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through Release success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CallRequest_002, TestSize.Level1)
{
    OHOS::sptr<AbilitySchedulerMock> scheduler = new AbilitySchedulerMock();
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    EXPECT_EQ(false, abilityRecord_->IsReady());
    abilityRecord_->SetScheduler(scheduler);

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::OK_NO_REMOTE_OBJ);
    EXPECT_EQ(abilityRecord_->CallRequest(), false);
}

/*
 * Feature: AbilityRecord
 * Function: IsStartToBackground
 * SubFunction: IsStartToBackground
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through IsStartToBackground false
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsStartToBackground_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRecord_->IsStartToBackground(), false);
}

/*
 * Feature: AbilityRecord
 * Function: SetStartToBackground
 * SubFunction: SetStartToBackground
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through SetStartToBackground success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetStartToBackground_002, TestSize.Level1)
{
    abilityRecord_->SetStartToBackground(true);
    EXPECT_EQ(abilityRecord_->IsStartToBackground(), true);
    abilityRecord_->SetStartToBackground(false);
    EXPECT_EQ(abilityRecord_->IsStartToBackground(), false);
}

/*
 * Feature: AbilityRecord
 * Function: SetSpecifiedFlag
 * SubFunction: SetSpecifiedFlag
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through SetSpecifiedFlag success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetSpecifiedFlag_001, TestSize.Level1)
{
    const std::string specifiedFlag= "flag";
    abilityRecord_->SetSpecifiedFlag(specifiedFlag);
    EXPECT_EQ(specifiedFlag, abilityRecord_->GetSpecifiedFlag());
}

/*
 * Feature: AbilityRecord
 * Function: GetSpecifiedFlag
 * SubFunction: GetSpecifiedFlag
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through GetSpecifiedFlag success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetSpecifiedFlag_002, TestSize.Level1)
{
    EXPECT_EQ(std::string(), abilityRecord_->GetSpecifiedFlag());
}

/*
 * Feature: AbilityRecord
 * Function: IsNeedToCallRequest
 * SubFunction: IsNeedToCallRequest
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through SetSpecifiedFlag false
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsNeedToCallRequest_001, TestSize.Level1)
{
    EXPECT_EQ(false, abilityRecord_->IsNeedToCallRequest());
}

/*
 * Feature: AbilityRecord
 * Function: IsNeedToCallRequest
 * SubFunction: IsNeedToCallRequest
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through GetSpecifiedFlag success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsNeedToCallRequest_002, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::INIT);

    abilityRecord_->callContainer_ = std::make_shared<CallContainer>();
    abilityRecord_->callContainer_->AddCallRecord(abilityRequest.connect, callRecord);

    EXPECT_EQ(true, abilityRecord_->IsNeedToCallRequest());
}
}  // namespace AAFwk
}  // namespace OHOS