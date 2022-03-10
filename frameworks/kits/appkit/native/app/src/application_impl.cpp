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

#include "application_impl.h"

#include "hilog_wrapper.h"
#include "ohos_application.h"
#include "uri_permission_manager_client.h"

namespace OHOS {
namespace AppExecFwk {
ApplicationImpl::ApplicationImpl() : curState_(APP_STATE_CREATE), recordId_(0)
{}

/**
 * @brief Set the application to the ApplicationImpl.
 *
 * @param application The application which the mainthread launched.
 *
 */
void ApplicationImpl::SetApplication(const std::shared_ptr<OHOSApplication> &application)
{
    if (application == nullptr) {
        HILOG_ERROR("ApplicationImpl::SetApplication failed, application is nullptr");
        return;
    }
    this->application_ = application;
}

/**
 * @brief Schedule the application to the APP_STATE_READY state.
 *
 * @return Returns true if performAppReady is scheduled successfully;
 *         Returns false otherwise.
 */
bool ApplicationImpl::PerformAppReady()
{
    HILOG_DEBUG("ApplicationImpl::PerformAppReady called");
    if (curState_ == APP_STATE_CREATE) {
        application_->OnStart();
        curState_ = APP_STATE_READY;
        return true;
    }
    HILOG_ERROR("ApplicationImpl::PerformAppReady error! curState is %{public}d", curState_);
    return false;
}

/**
 * @brief Schedule the application to the APP_STATE_FOREGROUND state.
 *
 * @return Returns true if PerformForeground is scheduled successfully;
 *         Returns false otherwise.
 */
bool ApplicationImpl::PerformForeground()
{
    HILOG_DEBUG("ApplicationImpl::performForeground called");
    if ((curState_ == APP_STATE_READY) || (curState_ == APP_STATE_BACKGROUND)) {
        application_->OnForeground();
        curState_ = APP_STATE_FOREGROUND;
        return true;
    }
    HILOG_ERROR("ApplicationImpl::performForeground error! curState is %{public}d", curState_);
    return false;
}

/**
 * @brief Schedule the application to the APP_STATE_BACKGROUND state.
 *
 * @return Returns true if PerformBackground is scheduled successfully;
 *         Returns false otherwise.
 */
bool ApplicationImpl::PerformBackground()
{
    HILOG_DEBUG("ApplicationImpl::performBackground called");
    if (curState_ == APP_STATE_FOREGROUND) {
        application_->OnBackground();
        curState_ = APP_STATE_BACKGROUND;
        return true;
    }
    HILOG_ERROR("ApplicationImpl::performBackground error! curState is %{public}d", curState_);
    return false;
}

/**
 * @brief Schedule the application to the APP_STATE_TERMINATED state.
 *
 * @return Returns true if PerformTerminate is scheduled successfully;
 *         Returns false otherwise.
 */
bool ApplicationImpl::PerformTerminate()
{
    HILOG_DEBUG("ApplicationImpl::PerformTerminate called");
    if (curState_ == APP_STATE_BACKGROUND) {
        application_->OnTerminate();
        curState_ = APP_STATE_TERMINATED;
        RemoveUriPermission();
        return true;
    }
    HILOG_ERROR("ApplicationImpl::performTerminate error! curState is %{public}d", curState_);
    return false;
}

/**
 * @brief Schedule the application to the APP_STATE_TERMINATED state.
 *
 * @return Returns true if PerformTerminate is scheduled successfully;
 *         Returns false otherwise.
 */
void ApplicationImpl::PerformTerminateStrong()
{
    HILOG_DEBUG("ApplicationImpl::PerformTerminateStrong called");
    application_->OnTerminate();
    RemoveUriPermission();
}

void ApplicationImpl::RemoveUriPermission()
{
    auto appContext = application_->GetAppContext();
    if (!appContext) {
        HILOG_ERROR("ApplicationImpl::RemoveUriPermission: Get appliction context failed.");
        return;
    }
    auto appInfo = appContext->GetApplicationInfo();
    if (!appInfo) {
        HILOG_ERROR("ApplicationImpl::RemoveUriPermission: Get appliction info failed.");
        return;
    }
    auto uriPermMgrClient = AAFwk::UriPermissionManagerClient::GetInstance();
    uriPermMgrClient->RemoveUriPermission(appInfo->accessTokenId);
}

/**
 * @brief System determines to trim the memory.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 *
 */
void ApplicationImpl::PerformMemoryLevel(int level)
{
    HILOG_DEBUG("ApplicationImpl::PerformMemoryLevel called");
    application_->OnMemoryLevel(level);
}

/**
 * @brief System determines to send the new config to application.
 *
 * @param config Indicates the updated configuration information.
 *
 */
void ApplicationImpl::PerformConfigurationUpdated(const Configuration &config)
{
    HILOG_DEBUG("ApplicationImpl::PerformConfigurationUpdated called");
    application_->OnConfigurationUpdated(config);
}

/**
 * @brief Set the target state to application.
 *
 * @param state The target state of application.
 *
 */
int ApplicationImpl::SetState(int state)
{
    curState_ = state;
    return curState_;
}

/**
 * @brief Get the current state of application.
 *
 * @return Returns the current state of application.
 *
 */
int ApplicationImpl::GetState() const
{
    return curState_;
}

/**
 * @brief Set the RecordId to application.
 *
 * @param id recordId.
 *
 */
void ApplicationImpl::SetRecordId(int id)
{
    recordId_ = id;
}

/**
 * @brief Get the recordId of application.
 *
 * @return Returns the recordId of application.
 *
 */
int ApplicationImpl::GetRecordId() const
{
    return recordId_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
