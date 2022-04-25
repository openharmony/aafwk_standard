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

#ifndef FOUNDATION_APPEXECFWK_OHOS_CONTEXT_H
#define FOUNDATION_APPEXECFWK_OHOS_CONTEXT_H

#include "context.h"
#include "gmock/gmock.h"

class MockContext {
public:
    MockContext() = default;
    virtual ~MockContext() = default;
    MOCK_METHOD0(GetProcessInfo, std::shared_ptr<ProcessInfo>());
    MOCK_METHOD0(GetApplicationInfo, std::shared_ptr<ApplicationInfo>());
    MOCK_METHOD0(GetApplicationContext, std::shared_ptr<Context>());
    MOCK_METHOD0(GetBundleCodePath, std::string());
    MOCK_METHOD0(GetAbilityInfo, std::shared_ptr<AbilityInfo>());
    MOCK_METHOD0(GetContext, std::shared_ptr<Context>());
    MOCK_METHOD0(GetBundleManager, sptr<IBundleMgr>());
    MOCK_METHOD0(GetResourceManager, std::shared_ptr<Global::Resource::ResourceManager>());
    MOCK_METHOD1(DeleteFile, bool(const std::string &fileName));
    MOCK_METHOD1(StopAbility, bool(const AAFwk::Want &want));
    MOCK_METHOD0(GetCacheDir, std::string ());

    virtual std::string GetCodeCacheDir() = 0;
    virtual std::string GetDatabaseDir() = 0;
    virtual std::string GetDataDir() = 0;
    virtual std::string GetDir(const std::string &name, int mode) = 0;
    virtual std::string GetExternalCacheDir() = 0;
    virtual std::string GetExternalFilesDir(std::string &type) = 0;
    virtual std::string GetFilesDir() = 0;
    virtual std::string GetNoBackupFilesDir() = 0;
    virtual int VerifyCallingPermission(const std::string &permission) = 0;
    virtual int VerifySelfPermission(const std::string &permission) = 0;
    virtual std::string GetBundleName() = 0;
    virtual std::string GetBundleResourcePath() = 0;
    virtual void StartAbility(const AAFwk::Want &want, int requestCode) = 0;
    virtual void UnauthUriPermission(const std::string &permission, const Uri &uri, int uid) = 0;
    virtual sptr<AAFwk::IAbilityManager> GetAbilityManager() = 0;
    virtual std::string GetAppType() = 0;
    virtual void TerminateAbility(int requestCode) = 0;
    virtual void TerminateAbility() = 0;
    virtual bool CanRequestPermission(const std::string &permission) = 0;
    virtual int VerifyCallingOrSelfPermission(const std::string &permission) = 0;
    virtual int VerifyPermission(const std::string &permission, int pid, int uid) = 0;
    virtual std::string GetDistributedDir() = 0;
    virtual void SetPattern(int patternId) = 0;
    virtual std::shared_ptr<Context> GetAbilityPackageContext() = 0;
    virtual std::shared_ptr<HapModuleInfo> GetHapModuleInfo() = 0;
    virtual std::string GetProcessName() = 0;
    virtual std::string GetCallingBundle() = 0;
    virtual void RequestPermissionsFromUser(std::vector<std::string> &permissions, std::vector<int> &permissionsState,
        int requestCode) = 0;
    virtual void StartAbility(const Want &want, int requestCode, const AbilityStartSetting &abilityStartSetting) = 0;
    virtual bool ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn) = 0;
    virtual void DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn) = 0;
    virtual Uri GetCaller() = 0;
    virtual std::string GetString(int resId) = 0;
    virtual std::vector<std::string> GetStringArray(int resId) = 0;
    virtual std::vector<int> GetIntArray(int resId) = 0;
    virtual std::map<std::string, std::string> GetTheme() = 0;
    virtual void SetTheme(int themeId) = 0;
    virtual std::map<std::string, std::string> GetPattern() = 0;
    virtual int GetColor(int resId) = 0;
    virtual int GetThemeId() = 0;
    virtual bool TerminateAbilityResult(int startId) = 0;
    virtual int GetDisplayOrientation() = 0;
    virtual std::string GetPreferencesDir() = 0;
    virtual void SetColorMode(int mode) = 0;
    virtual int GetColorMode() = 0;
    virtual int GetMissionId() = 0;
    virtual void StartAbilities(const std::vector<AAFwk::Want> &wants) = 0;
    friend DataAbilityHelper;
    friend DataShareHelper;
    friend ContinuationConnector;
protected:
    virtual sptr<IRemoteObject> GetToken() = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_CONTEXT_H
