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

#include "extension.h"

#include "ability_local_record.h"
#include "configuration.h"
#include "extension_context.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void Extension::Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Init begin.");
    if ((record == nullptr) || (application == nullptr) || (handler == nullptr) || (token == nullptr)) {
        HILOG_ERROR("Extension::init failed, some object is nullptr");
        return;
    }
    abilityInfo_ = record->GetAbilityInfo();
    handler_ = handler;
    application_ = application;
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * Will be called when extension start. You should override this function
 *
 * @param want start information
 */
void Extension::OnStart(const AAFwk::Want &want)
{
    HILOG_INFO("OnStart begin.");
    SetLaunchWant(want);
    SetLastRequestWant(want);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * Will be called when extension stop. You should override this function
 *
 * @param want start information
 */
void Extension::OnStop()
{
    HILOG_INFO("OnStop begin.");
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Called when this Service extension is connected for the first time.
 *
 * You can override this function to implement your own processing logic.
 *
 * @param want Indicates the {@link Want} structure containing connection information about the Service extension.
 * @return Returns a pointer to the <b>sid</b> of the connected Service extension.
 */
sptr<IRemoteObject> Extension::OnConnect(const AAFwk::Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return nullptr;
}

/**
 * @brief Called when all abilities connected to this Service ability are disconnected.
 *
 * You can override this function to implement your own processing logic.
 *
 */
void Extension::OnDisconnect(const AAFwk::Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Called back when Service is started.
 * This method can be called only by Service. You can use the StartAbility(ohos.aafwk.content.Want) method to start
 * Service. Then the system calls back the current method to use the transferred want parameter to execute its own
 * logic.
 *
 * @param want Indicates the want of Service to start.
 * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being destroyed,
 * and the value false indicates a normal startup.
 * @param startId Indicates the number of times the Service extension has been started. The startId is incremented by 1
 * every time the extension is started. For example, if the extension has been started for six times,
 * the value of startId is 6.
 */
void Extension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HILOG_INFO("%{public}s begin restart=%{public}s,startId=%{public}d.",
        __func__,
        restart ? "true" : "false",
        startId);
    SetLastRequestWant(want);
    HILOG_INFO("%{public}s end.", __func__);
}

void Extension::SetLaunchWant(const AAFwk::Want &want)
{
    launchWant_ = std::make_shared<AAFwk::Want>(want);
}

void Extension::SetLastRequestWant(const AAFwk::Want &want)
{
    lastRequestWant_ = std::make_shared<AAFwk::Want>(want);
}

/**
 * @brief Called when the system configuration is updated.
 *
 * @param configuration Indicates the updated configuration information.
 */
void Extension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HILOG_INFO("%{public}s called.", __func__);
}
}
}