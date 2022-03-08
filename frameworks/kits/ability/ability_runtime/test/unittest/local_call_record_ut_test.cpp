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
#include "ability_loader.h"
#include "ability_thread.h"
#include "mock_serviceability_manager_service.h"
#define private public
#include "local_call_record.h"
#define protected public
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace {
}

class LocalCallRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<LocalCallRecord> context_ = nullptr;
};

void LocalCallRecordTest::SetUpTestCase(void)
{
    OHOS::sptr<OHOS::IRemoteObject> abilityObject = new (std::nothrow) MockServiceAbilityManagerService();

    auto sysMgr = OHOS::DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }
    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, abilityObject);
}

void LocalCallRecordTest::TearDownTestCase(void)
{}

void LocalCallRecordTest::SetUp(void)
{}

void LocalCallRecordTest::TearDown(void)
{}

/**
 * @tc.number: Local_Call_Record_SetRemoteObject_0100
 * @tc.name: SetRemoteObject
 * @tc.desc: LocalCallRecord to process SetRemoteObject success.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_SetRemoteObject_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    EXPECT_TRUE(localCallRecord.callRecipient_ == nullptr);
    sptr<IRemoteObject> call = new (std::nothrow) MockServiceAbilityManagerService();
    EXPECT_TRUE(call != nullptr);
    localCallRecord.SetRemoteObject(call);
    EXPECT_TRUE(localCallRecord.callRecipient_ != nullptr);
    EXPECT_TRUE(localCallRecord.remoteObject_ != nullptr);
}

/**
 * @tc.number: Local_Call_Record_SetRemoteObject_0200
 * @tc.name: SetRemoteObject
 * @tc.desc: LocalCallRecord to process SetRemoteObject fail because call back is null.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_SetRemoteObject_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    EXPECT_TRUE(localCallRecord.callRecipient_ == nullptr);
    localCallRecord.SetRemoteObject(nullptr);
    EXPECT_TRUE(localCallRecord.callRecipient_ == nullptr);
    EXPECT_TRUE(localCallRecord.remoteObject_ == nullptr);
}

/**
 * @tc.number: Local_Call_Record_AddCaller_0100
 * @tc.name: AddCaller
 * @tc.desc: LocalCallRecord to process AddCaller success.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_AddCaller_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    EXPECT_TRUE(localCallRecord.callers_.size() == 0);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    localCallRecord.AddCaller(callback);
    EXPECT_TRUE(localCallRecord.callers_.size() != 0);
}

/**
 * @tc.number: Local_Call_Record_RemoveCaller_0100
 * @tc.name: RemoveCaller
 * @tc.desc: LocalCallRecord to process RemoveCaller success.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_RemoveCaller_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    localCallRecord.callers_.emplace_back(callback);
    bool ret = localCallRecord.RemoveCaller(callback);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.number: Local_Call_Record_RemoveCaller_0200
 * @tc.name: RemoveCaller
 * @tc.desc: LocalCallRecord to process RemoveCaller fail because callers_ is empty.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_RemoveCaller_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    bool ret = localCallRecord.RemoveCaller(callback);
    EXPECT_EQ(ret, false);
}

 /**
 * @tc.number: Local_Call_Record_RemoveCaller_0300
 * @tc.name: RemoveCaller
 * @tc.desc: LocalCallRecord to process RemoveCaller fail because not find call back.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_RemoveCaller_0300, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    localCallRecord.callers_.emplace_back(callback);
    std::shared_ptr<CallerCallBack> callback1 = std::make_shared<CallerCallBack>();
    bool ret = localCallRecord.RemoveCaller(callback1);
    EXPECT_EQ(ret, false);
}

 /**
 * @tc.number: Local_Call_Record_OnCallStubDied_0100
 * @tc.name: OnCallStubDied
 * @tc.desc: LocalCallRecord to process OnCallStubDied success.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_OnCallStubDied_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([](const std::string &result) {
        GTEST_LOG_(ERROR) << "OnRelease-----------" << result;
        EXPECT_TRUE(result == "died");
    });
    localCallRecord.callers_.emplace_back(callback);
    localCallRecord.OnCallStubDied(nullptr);
}

 /**
 * @tc.number: Local_Call_Record_OnCallStubDied_0200
 * @tc.name: OnCallStubDied
 * @tc.desc: LocalCallRecord to process OnCallStubDied fail because callers_ is empty.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_OnCallStubDied_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([](const std::string &result) {
        GTEST_LOG_(ERROR) << "OnRelease-----------" << result;
        EXPECT_TRUE(result != "died");
    });
    localCallRecord.OnCallStubDied(nullptr);
}

 /**
 * @tc.number: Local_Call_Record_OnCallStubDied_0300
 * @tc.name: OnCallStubDied
 * @tc.desc: LocalCallRecord to process OnCallStubDied fail because callback is null.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_OnCallStubDied_0300, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([](const std::string &result) {
        GTEST_LOG_(ERROR) << "OnRelease-----------" << result;
        EXPECT_TRUE(result != "died");
    });
    localCallRecord.callers_.emplace_back(nullptr);
    localCallRecord.OnCallStubDied(nullptr);
}

 /**
 * @tc.number: Local_Call_Record_InvokeCallBack_0100
 * @tc.name: InvokeCallBack
 * @tc.desc: LocalCallRecord to process InvokeCallBack success.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_InvokeCallBack_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});
    EXPECT_TRUE(callback->IsCallBack() == false);
    localCallRecord.callers_.emplace_back(callback);
    localCallRecord.InvokeCallBack();
    EXPECT_TRUE(callback->IsCallBack() == true);
}

 /**
 * @tc.number: Local_Call_Record_InvokeCallBack_0200
 * @tc.name: InvokeCallBack
 * @tc.desc: LocalCallRecord to process InvokeCallBack fail because remote is null.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_InvokeCallBack_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_TRUE(callback->IsCallBack() == false);
    localCallRecord.InvokeCallBack();
    EXPECT_TRUE(callback->IsCallBack() == false);
}

 /**
 * @tc.number: Local_Call_Record_InvokeCallBack_0300
 * @tc.name: InvokeCallBack
 * @tc.desc: LocalCallRecord to process InvokeCallBack fail because callers_ is empty.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_InvokeCallBack_0300, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_TRUE(callback->IsCallBack() == false);
    localCallRecord.InvokeCallBack();
    EXPECT_TRUE(callback->IsCallBack() == false);
}

 /**
 * @tc.number: Local_Call_Record_InvokeCallBack_0400
 * @tc.name: InvokeCallBack
 * @tc.desc: LocalCallRecord to process InvokeCallBack fail because call back is null.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_InvokeCallBack_0400, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_TRUE(callback->IsCallBack() == false);
    localCallRecord.callers_.emplace_back(nullptr);
    localCallRecord.InvokeCallBack();
    EXPECT_TRUE(callback->IsCallBack() == false);
}

 /**
 * @tc.number: Local_Call_Record_GetRemoteObject_0100
 * @tc.name: GetRemoteObject
 * @tc.desc: LocalCallRecord to process GetRemoteObject success.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_GetRemoteObject_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    EXPECT_TRUE(localCallRecord.GetRemoteObject() != nullptr);
}

 /**
 * @tc.number: Local_Call_Record_GetElementName_0100
 * @tc.name: GetElementName
 * @tc.desc: LocalCallRecord to process GetElementName success.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_GetElementName_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    EXPECT_TRUE(localCallRecord.GetElementName().GetBundleName() == "DemoBundleName");
}

 /**
 * @tc.number: Local_Call_Record_IsExistCallBack_0100
 * @tc.name: IsExistCallBack
 * @tc.desc: LocalCallRecord to process  success.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_IsExistCallBack_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    localCallRecord.AddCaller(callback);
    EXPECT_TRUE(localCallRecord.IsExistCallBack() == true);
}
}  // namespace AppExecFwk
}  // namespace OHOS