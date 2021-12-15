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
#include "bundlemgr/mock_bundle_manager.h"
#include "mock_ability_connect_callback.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#define private public
#define protected public
#include "ability_event_handler.h"
#include "ability_manager_service.h"
#undef private
#undef protected
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#define private public
#define protected public
#include "pending_want_record.h"
#include "pending_want_manager.h"
#undef private
#undef protected
#include "sa_mgr_client.h"
#include "sender_info.h"
#include "system_ability_definition.h"
#include "wants_info.h"
#include "want_receiver_stub.h"
#include "want_sender_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AAFwk {
#define SLEEP(milli) std::this_thread::sleep_for(std::chrono::seconds(milli))

namespace {}  // namespace
class PendingWantRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    WantSenderInfo MakeWantSenderInfo(Want &want, int32_t flags, int32_t userId, int32_t type = 1);
    std::shared_ptr<PendingWantKey> MakeWantKey(WantSenderInfo &wantSenderInfo);
    static constexpr int TEST_WAIT_TIME = 100000;

    class CancelReceiver : public AAFwk::WantReceiverStub {
    public:
        static int performReceiveCount;
        static int sendCount;
        void Send(const int32_t resultCode) override;
        void PerformReceive(const AAFwk::Want &want, int resultCode, const std::string &data,
            const AAFwk::WantParams &extras, bool serialized, bool sticky, int sendingUser) override;
        virtual sptr<IRemoteObject> AsObject() override
        {
            return nullptr;
        }
    };

public:
    std::shared_ptr<PendingWantManager> pendingManager_ {nullptr};
    std::shared_ptr<AbilityManagerService> abilityMs_ {nullptr};
};

int PendingWantRecordTest::CancelReceiver::performReceiveCount = 0;
int PendingWantRecordTest::CancelReceiver::sendCount = 0;

void PendingWantRecordTest::CancelReceiver::Send(const int32_t resultCode)
{
    sendCount = 100;
}

void PendingWantRecordTest::CancelReceiver::PerformReceive(const AAFwk::Want &want, int resultCode,
    const std::string &data, const AAFwk::WantParams &extras, bool serialized, bool sticky, int sendingUser)
{
    performReceiveCount = 100;
}

void PendingWantRecordTest::SetUpTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void PendingWantRecordTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

void PendingWantRecordTest::SetUp()
{
    abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    abilityMs_->OnStart();
}

void PendingWantRecordTest::TearDown()
{}

WantSenderInfo PendingWantRecordTest::MakeWantSenderInfo(Want &want, int32_t flags, int32_t userId, int32_t type)
{
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = type;
    // wantSenderInfo.type is OperationType::START_ABILITY
    wantSenderInfo.bundleName = "com.ix.hiRadio";
    wantSenderInfo.resultWho = "RadioTopAbility";
    int requestCode = 10;
    wantSenderInfo.requestCode = requestCode;
    std::vector<WantsInfo> allWant;
    WantsInfo wantInfo;
    wantInfo.want = want;
    wantInfo.resolvedTypes = "nihao";
    allWant.emplace_back(wantInfo);
    wantSenderInfo.allWants = allWant;
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = userId;
    return wantSenderInfo;
}

std::shared_ptr<PendingWantKey> PendingWantRecordTest::MakeWantKey(WantSenderInfo &wantSenderInfo)
{
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->SetBundleName(wantSenderInfo.bundleName);
    pendingKey->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    pendingKey->SetFlags(wantSenderInfo.flags);
    pendingKey->SetUserId(wantSenderInfo.userId);
    pendingKey->SetType(wantSenderInfo.type);
    pendingKey->SetRequestWant(wantSenderInfo.allWants.back().want);
    pendingKey->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
    wantSenderInfo.allWants.back().want = pendingKey->GetRequestWant();
    wantSenderInfo.allWants.back().resolvedTypes = pendingKey->GetRequestResolvedType();
    pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    return pendingKey;
}

/*
 * @tc.number    : PendingWantRecordTest_0100
 * @tc.name      : PendingWantManager Constructors
 * @tc.desc      : 1.Constructors
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0100, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    EXPECT_EQ(pendingWantRecord->uid_, 1);
    EXPECT_EQ(pendingWantRecord->callerToken_, nullptr);
}

/*
 * @tc.number    : PendingWantRecordTest_0200
 * @tc.name      : PendingWantManager RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener(nullptr)
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0200, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(cance);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->mCancelCallbacks_.size()), 1);
}

/*
 * @tc.number    : PendingWantRecordTest_0300
 * @tc.name      : PendingWantManager RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener,one sptr<CancelReceiver>
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0300, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(nullptr);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->mCancelCallbacks_.size()), 0);
}

/*
 * @tc.number    : PendingWantRecordTest_0400
 * @tc.name      : PendingWantManager RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener,two sptr<CancelReceiver>
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0400, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    sptr<CancelReceiver> cance1 = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(cance);
    pendingWantRecord->RegisterCancelListener(cance1);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->mCancelCallbacks_.size()), 2);
}

/*
 * @tc.number    : PendingWantRecordTest_0500
 * @tc.name      : PendingWantManager UnregisterCancelListener
 * @tc.desc      : 1.UnregisterCancelListener(sptr<CancelReceiver>)
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0500, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    sptr<CancelReceiver> cance1 = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(cance);
    pendingWantRecord->RegisterCancelListener(cance1);
    pendingWantRecord->UnregisterCancelListener(cance);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->mCancelCallbacks_.size()), 1);
}

/*
 * @tc.number    : PendingWantRecordTest_0600
 * @tc.name      : PendingWantManager UnregisterCancelListener
 * @tc.desc      : 1.UnregisterCancelListener(nullptr)
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0600, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    sptr<CancelReceiver> cance1 = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(cance);
    pendingWantRecord->RegisterCancelListener(cance1);
    pendingWantRecord->UnregisterCancelListener(nullptr);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->mCancelCallbacks_.size()), 2);
}

/*
 * @tc.number    : PendingWantRecordTest_0700
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,canceled_ == true
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0700, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    pendingWantRecord->canceled_ = true;
    EXPECT_EQ(pendingWantRecord->SenderInner(info), -96);
}

/*
 * @tc.number    : PendingWantRecordTest_0800
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,flags:CONSTANT_FLAG,type:OperationType::START_ABILITY
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0800, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::START_ABILITY);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    EXPECT_EQ(pendingWantRecord->SenderInner(info), NO_ERROR);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_0900
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ == nullptr,flags:CONSTANT_FLAG
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0900, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(nullptr, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    EXPECT_EQ(pendingWantRecord->SenderInner(info), ERR_INVALID_VALUE);
    EXPECT_TRUE(info.resolvedType != key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1000
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ == nullptr,flags:ONE_TIME_FLAG
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1000, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ONE_TIME_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(nullptr, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    info.finishedReceiver = new CancelReceiver();
    EXPECT_EQ(pendingWantRecord->SenderInner(info), ERR_INVALID_VALUE);
    EXPECT_TRUE(info.resolvedType != key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1100
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,flags:CONSTANT_FLAG,type:OperationType::START_ABILITIES
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1100, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::START_ABILITIES);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    EXPECT_NE(pendingWantRecord->SenderInner(info), NO_ERROR);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1200
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,flags:CONSTANT_FLAG,type:OperationType::START_SERVICE
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1200, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::START_SERVICE);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    EXPECT_NE(pendingWantRecord->SenderInner(info), NO_ERROR);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1300
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ !=
 * nullptr,flags:CONSTANT_FLAG,type:OperationType::START_FOREGROUND_SERVICE
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1300, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::START_FOREGROUND_SERVICE);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    EXPECT_NE(pendingWantRecord->SenderInner(info), NO_ERROR);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1400
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ !=
 * nullptr,flags:CONSTANT_FLAG,type:OperationType::SEND_COMMON_EVENT
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1400, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    pendingWantRecord->SenderInner(info);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1500
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ !=
 * nullptr,flags:CONSTANT_FLAG,type:OperationType::SEND_COMMON_EVENT, 2.PerformReceive Called
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1500, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    info.finishedReceiver = new CancelReceiver();
    pendingWantRecord->SenderInner(info);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
    EXPECT_TRUE(CancelReceiver::performReceiveCount == 100);
    CancelReceiver::performReceiveCount = 0;
}

/*
 * @tc.number    : PendingWantRecordTest_1600
 * @tc.name      : PendingWantManager GetKey
 * @tc.desc      : 1.GetKey
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1600, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    EXPECT_EQ(pendingWantRecord->GetKey(), key);
}

/*
 * @tc.number    : PendingWantRecordTest_1700
 * @tc.name      : PendingWantManager GetUid
 * @tc.desc      : 1.GetUid
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1700, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    EXPECT_EQ(pendingWantRecord->GetUid(), 1);
}

/*
 * @tc.number    : PendingWantRecordTest_1800
 * @tc.name      : PendingWantManager SetCanceled/GetCanceled
 * @tc.desc      : 1.SetCanceled/GetCanceled
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1800, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    EXPECT_EQ(pendingWantRecord->GetCanceled(), false);
    pendingWantRecord->SetCanceled();
    EXPECT_EQ(pendingWantRecord->GetCanceled(), true);
}

/*
 * @tc.number    : PendingWantRecordTest_1900
 * @tc.name      : PendingWantManager GetCancelCallbacks
 * @tc.desc      : 1.GetCancelCallbacks
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1900, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    sptr<CancelReceiver> cance1 = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(cance);
    pendingWantRecord->RegisterCancelListener(cance1);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->GetCancelCallbacks().size()), 2);
}

/*
 * @tc.number    : PendingWantRecordTest_2000
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,flags:CONSTANT_FLAG
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_2000, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    pendingWantRecord->SenderInner(info);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_2100
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,flags:ONE_TIME_FLAG
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_2100, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ONE_TIME_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    info.finishedReceiver = new CancelReceiver();
    pendingWantRecord->SenderInner(info);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}
}  // namespace AAFwk
}  // namespace OHOS