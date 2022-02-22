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

#include "uri_permission_manager_service.h"

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<UriPermissionManagerService>::GetInstance().get());

UriPermissionManagerService::UriPermissionManagerService() : SystemAbility(URI_PERMISSION_MGR_SERVICE_ID, true) {}

UriPermissionManagerService::~UriPermissionManagerService()
{
    if (impl_ != nullptr) {
        impl_ = nullptr;
    }
}

void UriPermissionManagerService::OnStart()
{
    HILOG_INFO("UriPermissionManagerService start is triggered.");
    if (!Init()) {
        HILOG_ERROR("init failed.");
        return;
    }

    if (!registerToService_) {
        auto systemAabilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAabilityMgr || systemAabilityMgr->AddSystemAbility(URI_PERMISSION_MGR_SERVICE_ID, impl_) != 0) {
            HILOG_ERROR("fail to register to system ability manager");
            return;
        }
        HILOG_INFO("register to system ability manager success");
        registerToService_ = true;
    }
}

void UriPermissionManagerService::OnStop()
{
    HILOG_INFO("OnStop is called.");
    SelfClean();
}

bool UriPermissionManagerService::IsServiceReady() const
{
    return ready_;
}

bool UriPermissionManagerService::Init()
{
    if (ready_) {
        HILOG_WARN("init more than one time.");
        return true;
    }

    if (impl_ == nullptr) {
        impl_ = new UriPermissionManagerStubImpl();
    }
    ready_ = true;
    return true;
}

void UriPermissionManagerService::SelfClean()
{
    if (ready_) {
        ready_ = false;
        if (registerToService_) {
            registerToService_ = false;
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
