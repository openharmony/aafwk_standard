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
#include <gmock/gmock.h>

#include <iremote_object.h>
#include <iremote_stub.h>

#include "ability_connect_callback_interface.h"
#include "ability_loader.h"
#include "ability_thread.h"
#define private public
#define protected public
#include "ability_record.h"
#include "call_record.h"
#include "mission_list.h"
#include "mission.h"
#include "mission_info_mgr.h"
#include "mission_list_manager.h"

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace {
}

class MissionListManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<MissionListManager> missionListMgr_ = nullptr;
};

void MissionListManagerTest::SetUpTestCase(void)
{}

void MissionListManagerTest::TearDownTestCase(void)
{}

void MissionListManagerTest::SetUp(void)
{}

void MissionListManagerTest::TearDown(void)
{}

class MissionListManagerTestStub : public IRemoteStub<IAbilityConnection> {
public:
    MissionListManagerTestStub() {};
    virtual ~MissionListManagerTestStub() {};

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        return 0;
    };

    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) {};

    /**
     * OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) {};
};

class MissionListManagerTestAbilityThreadStub : public AppExecFwk::AbilityThread {
public:
    MissionListManagerTestAbilityThreadStub() {};
    ~MissionListManagerTestAbilityThreadStub() {};

    sptr<IRemoteObject> CallRequest()
    {
        sptr<MissionListManagerTestStub> remote = new (std::nothrow) MissionListManagerTestStub();
        return remote->AsObject();
    }
};

/**
 * @tc.number: MissionListManager_001
 * @tc.name: OnCallConnectDied
 * @tc.desc: MissionListManager to process OnCallConnectDied success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_001 begin";
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    int32_t callerUid;
    sptr<IAbilityConnection> connCallback = new (std::nothrow) MissionListManagerTestStub();
    sptr<IRemoteObject> callToken = nullptr;
    AppExecFwk::ElementName element;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord =
        std::make_shared<CallRecord>(callerUid, abilityRecord, connCallback, callToken);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    auto testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(1, testValue);
    missionListMgr->OnCallConnectDied(callRecord);

    testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(0, testValue);
    GTEST_LOG_(INFO) << "MissionListManager_001 end";
}

/**
 * @tc.number: MissionListManager_002
 * @tc.name: OnCallConnectDied
 * @tc.desc: MissionListManager to process OnCallConnectDied success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_002 begin";
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    int32_t callerUid;
    sptr<IAbilityConnection> connCallback = new (std::nothrow) MissionListManagerTestStub();
    sptr<IRemoteObject> callToken = nullptr;
    AppExecFwk::ElementName element;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord =
        std::make_shared<CallRecord>(callerUid, abilityRecord, connCallback, callToken);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    auto testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(1, testValue);
    missionListMgr->OnCallConnectDied(nullptr);

    testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(1, testValue);
    GTEST_LOG_(INFO) << "MissionListManager_002 end";
}

/**
 * @tc.number: MissionListManager_003
 * @tc.name: OnCallConnectDied
 * @tc.desc: MissionListManager to process OnCallConnectDied success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_003 begin";
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    sptr<IAbilityConnection> connCallback = new (std::nothrow) MissionListManagerTestStub();
    sptr<IAbilityConnection> connCallback1 = new (std::nothrow) MissionListManagerTestStub();
    sptr<IRemoteObject> callToken = nullptr;
    AppExecFwk::ElementName element;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord =
        std::make_shared<CallRecord>(1, abilityRecord, connCallback, callToken);
    std::shared_ptr<CallRecord> callRecord1 =
        std::make_shared<CallRecord>(2, abilityRecord, connCallback1, callToken);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callContainer->AddCallRecord(connCallback1, callRecord1);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    auto testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(1, testValue);
    missionListMgr->OnCallConnectDied(callRecord);

    testValue = static_cast<int>(callContainer->callRecordMap_.size());
    EXPECT_EQ(1, testValue);
    GTEST_LOG_(INFO) << "MissionListManager_003 end";
}

/**
 * @tc.number: MissionListManager_004
 * @tc.name: CallAbilityLocked
 * @tc.desc: MissionListManager to process CallAbilityLocked success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_004 begin";

    AbilityRequest abilityRequest;
    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);

    int testRet = missionListMgr->CallAbilityLocked(abilityRequest);

    EXPECT_EQ(ERR_INVALID_VALUE, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_004 end";
}

/**
 * @tc.number: MissionListManager_005
 * @tc.name: CallAbilityLocked
 * @tc.desc: MissionListManager to process CallAbilityLocked success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_005, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_005 begin";

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.abilityInfo.bundleName = "test_bundle";
    abilityRequest.abilityInfo.name = "test_name";
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;

    Want want;
    AppExecFwk::AbilityInfo abilityInfo = abilityRequest.abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    int32_t id;
    std::string missionName = "#" + abilityRequest.abilityInfo.bundleName + ":" + abilityRequest.abilityInfo.name;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();
    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);

    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::INIT);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    int testRet = missionListMgr->CallAbilityLocked(abilityRequest);

    EXPECT_EQ(RESOLVE_CALL_ABILITY_INNER_ERR, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_005 end";
}

/**
 * @tc.number: MissionListManager_006
 * @tc.name: CallAbilityLocked
 * @tc.desc: MissionListManager to process CallAbilityLocked success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_006, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_006 begin";

    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.abilityInfo.bundleName = "test_bundle";
    abilityRequest.abilityInfo.name = "test_name";
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityRequest.connect = connCallback;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo = abilityRequest.abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    int32_t id;
    std::string missionName = "#" + abilityRequest.abilityInfo.bundleName + ":" + abilityRequest.abilityInfo.name;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);
    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::REQUESTED);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    int testRet = missionListMgr->CallAbilityLocked(abilityRequest);

    EXPECT_EQ(ERR_OK, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_006 end";
}

/**
 * @tc.number: MissionListManager_007
 * @tc.name: ResolveAbility
 * @tc.desc: MissionListManager to process ResolveAbility success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_007, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_007 begin";

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);

    int testRet = missionListMgr->ResolveAbility(abilityRecord, abilityRequest);

    EXPECT_EQ(ResolveResultType::NG_INNER_ERROR, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_007 end";
}


/**
 * @tc.number: MissionListManager_008
 * @tc.name: GetAbilityRecordByName
 * @tc.desc: MissionListManager to process GetAbilityRecordByName success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_008, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_008 begin";

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    AppExecFwk::ElementName element;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();

    missionListMgr->currentMissionLists_.push_front(missionList);
    missionListMgr->launcherList_ = missionList;
    missionListMgr->defaultSingleList_ = missionList;

    auto testRet = missionListMgr->GetAbilityRecordByName(element);

    EXPECT_TRUE(nullptr == testRet);
    GTEST_LOG_(INFO) << "MissionListManager_008 end";
}

/**
 * @tc.number: MissionListManager_009
 * @tc.name: GetAbilityRecordByName
 * @tc.desc: MissionListManager to process GetAbilityRecordByName success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_009, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_009 begin";

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    AppExecFwk::ElementName element;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord, "");

    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    auto testRet = missionListMgr->GetAbilityRecordByName(element);

    EXPECT_TRUE(nullptr != testRet);
    GTEST_LOG_(INFO) << "MissionListManager_009 end";
}

/**
 * @tc.number: MissionListManager_010
 * @tc.name: ResolveAbility
 * @tc.desc: MissionListManager to process ResolveAbility success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_010, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_010 begin";

    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = connCallback;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::REQUESTED);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;

    int testRet = missionListMgr->ResolveAbility(abilityRecord, abilityRequest);

    EXPECT_EQ(ResolveResultType::OK_HAS_REMOTE_OBJ, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_010 end";
}

/**
 * @tc.number: MissionListManager_011
 * @tc.name: ResolveAbility
 * @tc.desc: MissionListManager to process ResolveAbility success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_011, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_011 begin";

    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = connCallback;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::INIT);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    abilityRecord->isReady_ = true;
    abilityRecord->scheduler_ = new (std::nothrow) MissionListManagerTestAbilityThreadStub();

    int testRet = missionListMgr->ResolveAbility(abilityRecord, abilityRequest);

    EXPECT_EQ(ResolveResultType::OK_HAS_REMOTE_OBJ, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_011 end";
}

/**
 * @tc.number: MissionListManager_012
 * @tc.name: ResolveAbility
 * @tc.desc: MissionListManager to process ResolveAbility success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_012, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_012 begin";

    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = connCallback;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();

    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::INIT);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    abilityRecord->isReady_ = false;

    int testRet = missionListMgr->ResolveAbility(abilityRecord, abilityRequest);

    EXPECT_EQ(ResolveResultType::OK_NO_REMOTE_OBJ, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_012 end";
}

/**
 * @tc.number: MissionListManager_013
 * @tc.name: ResolveLocked
 * @tc.desc: MissionListManager to process ResolveLocked success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_013, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_013 begin";
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::INVALID_TYPE;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);

    int testRet = missionListMgr->ResolveLocked(abilityRequest);

    EXPECT_EQ(RESOLVE_CALL_ABILITY_INNER_ERR, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_013 end";
}


/**
 * @tc.number: MissionListManager_014
 * @tc.name: ResolveLocked
 * @tc.desc: MissionListManager to process ResolveLocked success.
 */
HWTEST_F(MissionListManagerTest, MissionListManager_014, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "MissionListManager_014 begin";


    sptr<MissionListManagerTestStub> connCallback = new (std::nothrow) MissionListManagerTestStub();
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.abilityInfo.bundleName = "test_bundle";
    abilityRequest.abilityInfo.name = "test_name";
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityRequest.connect = connCallback;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo = abilityRequest.abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    int32_t id;
    std::string missionName = "#" + abilityRequest.abilityInfo.bundleName + ":" + abilityRequest.abilityInfo.name;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord =
        std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    std::shared_ptr<CallContainer> callContainer = std::make_shared<CallContainer>();
    std::shared_ptr<CallRecord> callRecord = std::make_shared<CallRecord>(0, abilityRecord, connCallback, nullptr);
    callRecord->connCallback_ = connCallback;
    callRecord->callRemoteObject_ = connCallback->AsObject();
    callRecord->SetCallState(CallState::REQUESTED);
    callContainer->AddCallRecord(connCallback, callRecord);
    abilityRecord->callContainer_ = callContainer;
    missionList->AddMissionToTop(mission);
    missionListMgr->currentMissionLists_.push_front(missionList);

    int testRet = missionListMgr->ResolveLocked(abilityRequest);

    EXPECT_EQ(ERR_OK, testRet);
    GTEST_LOG_(INFO) << "MissionListManager_014 end";
}
}  // namespace AAFwk
}  // namespace OHOS