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
#include "local_call_container.h"
#define protected public
#include "ability_loader.h"
#include "ability_thread.h"
#include "mock_serviceability_manager_service.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace {
}
class LocalCallContainerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void LocalCallContainerTest::SetUpTestCase(void)
{
    OHOS::sptr<OHOS::IRemoteObject> abilityObject = new (std::nothrow) MockServiceAbilityManagerService();

    auto sysMgr = OHOS::DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }

    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, abilityObject);
}

void LocalCallContainerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "LocalCallContainerTest::TearDownTestCase end";
}

void LocalCallContainerTest::SetUp(void)
{}

void LocalCallContainerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "LocalCallContainerTest::TearDown end";
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0100
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is success.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 begain";

    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();

    Want want;
    want.SetElementName("DemoDeviceIdB", "DemoBundleNameB", "DemoAbilityNameB");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 begain1";
    ErrCode ret = localCallContainer->StartAbilityInner(want, callback, nullptr);
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 begain2";
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 end";
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0200
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is fail because callback is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);

    ErrCode ret = localCallContainer.StartAbilityInner(want, nullptr, callRemoteObject);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0300
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is fail because bundle name is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0300, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    Want want;
    want.SetElementName("DemoDeviceId", "", "DemoAbilityName");

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);

    ErrCode ret = localCallContainer.StartAbilityInner(want, nullptr, callRemoteObject);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0400
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is fail because ability name is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0400, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "");

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);

    ErrCode ret = localCallContainer.StartAbilityInner(want, nullptr, callRemoteObject);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_0100
 * @tc.name: Release
 * @tc.desc: Local Call Container to process Release, and the result is success.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0100, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    callback->SetCallBack([](const sptr<IRemoteObject> &) {});

    std::string uri = elementName.GetURI();
    localCallContainer->callProxyRecords_.emplace(uri, localCallRecord);

    ErrCode ret = localCallContainer->Release(callback);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Local_Call_Container_Release_0200
 * @tc.name: Release
 * @tc.desc: Local Call Container to process Release, and the result is fail bacause callProxyRecords_ is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0200, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();

    callback->SetCallBack([](const sptr<IRemoteObject> &) {});

    ErrCode ret = localCallContainer->Release(callback);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0100
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(call back is called).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0100, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->isCallBack_ = true;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    std::string uri = elementName.GetURI();
    localCallContainer.callProxyRecords_.emplace(uri, localCallRecord);

    std::vector<std::string> info;
    std::string resultReceiver = "";
    EXPECT_TRUE(resultReceiver == "");
    localCallContainer.DumpCalls(info);

    for (auto it : info) {
        GTEST_LOG_(ERROR) << it;
        resultReceiver += it + "\n";
    }
    string::size_type idx;
    idx = resultReceiver.find("REQUESTEND");
    EXPECT_TRUE(idx != string::npos);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0200
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(call back is not called).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    std::string uri = elementName.GetURI();
    localCallContainer.callProxyRecords_.emplace(uri, localCallRecord);

    std::vector<std::string> info;
    std::string resultReceiver = "";
    std::string::size_type idx;
    idx = resultReceiver.find("REQUESTING");
    EXPECT_FALSE(idx != string::npos);
    localCallContainer.DumpCalls(info);

    for (auto it : info) {
        resultReceiver += it + "\n";
    }
    idx = resultReceiver.find("REQUESTING");
    EXPECT_TRUE(idx != string::npos);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0300
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(no caller).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0300, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    std::string uri = elementName.GetURI();
    localCallContainer.callProxyRecords_.emplace(uri, localCallRecord);

    std::vector<std::string> info;
    std::string resultReceiver = "";
    std::string::size_type idx;
    idx = resultReceiver.find("REQUESTEND");
    EXPECT_FALSE(idx != string::npos);
    localCallContainer.DumpCalls(info);

    for (auto it : info) {
        resultReceiver += it + "\n";
    }

    idx = resultReceiver.find("REQUESTEND");
    EXPECT_TRUE(idx != string::npos);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0400
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(callProxyRecords_ is empty).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0400, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::vector<std::string> info;
    std::string resultReceiver = "";
    EXPECT_TRUE(resultReceiver == "");
    localCallContainer.DumpCalls(info);
    EXPECT_TRUE(resultReceiver == "");
}

/**
 * @tc.number: Local_Call_Container_OnAbilityConnectDone_0100
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: Local Call Container to process OnAbilityConnectDone, and the result is success resultCode == ERR_OK.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityConnectDone_0100, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallRecord->callers_.emplace_back(callback);

    std::string uri = elementName.GetURI();
    localCallContainer.callProxyRecords_.emplace(uri, localCallRecord);

    OHOS::sptr<OHOS::IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();

    localCallContainer.OnAbilityConnectDone(elementName, remoteObject, 0);
    EXPECT_EQ(callback->isCallBack_, true);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityConnectDone_0200
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: Local Call Container to process OnAbilityConnectDone success when resultCode != ERR_OK.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityConnectDone_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallRecord->callers_.emplace_back(callback);

    std::string uri = elementName.GetURI();
    localCallContainer.callProxyRecords_.emplace(uri, localCallRecord);

    OHOS::sptr<OHOS::IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();

    localCallContainer.OnAbilityConnectDone(elementName, remoteObject, -1);
    EXPECT_EQ(callback->isCallBack_, true);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityConnectDone_0300
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: Local Call Container to process OnAbilityConnectDone fail because localCallRecord is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityConnectDone_0300, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_EQ(callback->isCallBack_, false);
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    OHOS::sptr<OHOS::IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();

    localCallContainer.OnAbilityConnectDone(elementName, remoteObject, 0);
    EXPECT_EQ(callback->isCallBack_, false);
}

/**
 * @tc.number: Local_Call_Container_GetCallLocalreocrd_0100
 * @tc.name: GetCallLocalreocrd
 * @tc.desc: Local Call Container to process GetCallLocalreocrd, and the result is success.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_GetCallLocalreocrd_0100, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    std::string uri = elementName.GetURI();
    localCallContainer.callProxyRecords_.emplace(uri, localCallRecord);

    bool ret = localCallContainer.GetCallLocalreocrd(elementName, localCallRecord);
    EXPECT_TRUE(ret == true);
}

/**
 * @tc.number: Local_Call_Container_GetCallLocalreocrd_0100
 * @tc.name: GetCallLocalreocrd
 * @tc.desc: Local Call Container to process GetCallLocalreocrd, and the result is fail
 *           because call proxy records is empty.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_GetCallLocalreocrd_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    bool ret = localCallContainer.GetCallLocalreocrd(elementName, localCallRecord);
    EXPECT_TRUE(ret == false);
}
}  // namespace AppExecFwk
}  // namespace OHOS