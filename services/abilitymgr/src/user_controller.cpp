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

#include "user_controller.h"

#include "ability_manager_service.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "os_account_manager.h"
#include "task_data_persistence_mgr.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::AppExecFwk;
namespace {
const int64_t USER_SWITCH_TIMEOUT = 3 * 1000; // 3s
}

UserItem::UserItem(int32_t id) : userId_(id)
{}

UserItem::~UserItem() {}

int32_t UserItem::GetUserId()
{
    return userId_;
}

void UserItem::SetState(const UserState &state)
{
    if (curState_ == state) {
        return;
    }
    lastState_ = curState_;
    curState_ = state;
}

UserState UserItem::GetState()
{
    return curState_;
}

UserController::UserController()
{
}

UserController::~UserController()
{
}

void UserController::Init()
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (!handler) {
        return;
    }

    auto runner = handler->GetEventRunner();
    if (!runner) {
        return;
    }

    if (eventHandler_) {
        return;
    }
    eventHandler_ = std::make_shared<UserEventHandler>(runner, shared_from_this());
}

int32_t UserController::StartUser(int32_t userId, bool isForeground)
{
    if (userId < 0 || userId == USER_ID_NO_HEAD) {
        HILOG_ERROR("StartUser userId is invalid:%{public}d", userId);
        return -1;
    }

    if (IsCurrentUser(userId)) {
        HILOG_WARN("StartUser user is already current:%{public}d", userId);
        return 0;
    }

    if (!IsExistOsAccount(userId)) {
        HILOG_ERROR("StartUser not exist such account:%{public}d", userId);
        return -1;
    }

    if (isForeground) {
        // start freezing screen
        DelayedSingleton<AbilityManagerService>::GetInstance()->StartFreezingScreen();
    }

    auto oldUserId = GetCurrentUserId();
    auto userItem = GetOrCreateUserItem(userId);
    auto state = userItem->GetState();
    if (state == STATE_STOPPING || state == STATE_SHUTDOWN) {
        HILOG_ERROR("StartUser user is stop now, userId:%{public}d", userId);
        return -1;
    }

    if (isForeground) {
        SetCurrentUserId(userId);
        // notify wms switching now
    }

    bool needStart = false;
    if (state == STATE_BOOTING) {
        needStart = true;
        // send user start msg.
        SendSystemUserStart(userId);
    }

    if (isForeground) {
        SendSystemUserCurrent(oldUserId, userId);
        SendReportUserSwitch(oldUserId, userId, userItem);
        SendUserSwitchTimeout(oldUserId, userId, userItem);
    }

    if (needStart) {
        BroadcastUserStarted(userId);
    }

    UserBootDone(userItem);
    if (isForeground) {
        MoveUserToForeground(oldUserId, userId);
    }

    return 0;
}

int32_t UserController::StopUser(int32_t userId)
{
    if (userId < 0 || userId == USER_ID_NO_HEAD || userId == USER_ID_DEFAULT) {
        HILOG_ERROR("userId is invalid:%{public}d", userId);
        return -1;
    }

    if (IsCurrentUser(userId)) {
        HILOG_WARN("user is already current:%{public}d", userId);
        return 0;
    }

    if (!IsExistOsAccount(userId)) {
        HILOG_ERROR("not exist such account:%{public}d", userId);
        return -1;
    }

    BroadcastUserStopping(userId);

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        HILOG_ERROR("appScheduler is null");
        return -1;
    }
    appScheduler->KillProcessesByUserId(userId);

    auto taskDataPersistenceMgr = DelayedSingleton<TaskDataPersistenceMgr>::GetInstance();
    if (!taskDataPersistenceMgr) {
        HILOG_ERROR("taskDataPersistenceMgr is null");
        return -1;
    }
    taskDataPersistenceMgr->RemoveUserDir(userId);

    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityManagerService) {
        HILOG_ERROR("abilityManagerService is null");
        return -1;
    }
    abilityManagerService->ClearUserData(userId);

    BroadcastUserStopped(userId);
    return 0;
}

int32_t UserController::GetCurrentUserId()
{
    std::lock_guard<std::recursive_mutex> guard(userLock_);
    return currentUserId_;
}

std::shared_ptr<UserItem> UserController::GetUserItem(int32_t userId)
{
    std::lock_guard<std::recursive_mutex> guard(userLock_);
    auto it = userItems_.find(userId);
    if (it != userItems_.end()) {
        return it->second;
    }

    return nullptr;
}

bool UserController::IsCurrentUser(int32_t userId)
{
    int32_t oldUserId = GetCurrentUserId();
    if (oldUserId == userId) {
        auto userItem = GetUserItem(userId);
        if (userItem) {
            HILOG_WARN("IsCurrentUser userId is already current:%{public}d", userId);
            return true;
        }
    }
    return false;
}

bool UserController::IsExistOsAccount(int32_t userId)
{
    bool isExist = false;
    auto errCode = AccountSA::OsAccountManager::IsOsAccountExists(userId, isExist);
    return (errCode == 0) && isExist;
}

std::shared_ptr<UserItem> UserController::GetOrCreateUserItem(int32_t userId)
{
    std::lock_guard<std::recursive_mutex> guard(userLock_);
    auto it = userItems_.find(userId);
    if (it != userItems_.end()) {
        return it->second;
    }

    auto userItem = std::make_shared<UserItem>(userId);
    userItems_.emplace(userId, userItem);
    return userItem;
}

void UserController::SetCurrentUserId(int32_t userId)
{
    std::lock_guard<std::recursive_mutex> guard(userLock_);
    currentUserId_ = userId;
}

void UserController::MoveUserToForeground(int32_t oldUserId, int32_t newUserId)
{
    auto manager = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!manager) {
        return;
    }
    manager->SwitchToUser(oldUserId, newUserId);
    BroadcastUserBackground(oldUserId);
    BroadcastUserForeground(newUserId);
}

void UserController::UserBootDone(std::shared_ptr<UserItem> &item)
{
    if (!item) {
        return;
    }
    int32_t userId = item->GetUserId();

    std::lock_guard<std::recursive_mutex> guard(userLock_);
    auto it = userItems_.find(userId);
    if (it != userItems_.end()) {
        return;
    }

    if (item != it->second) {
        return;
    }
    item->SetState(UserState::STATE_STARTED);
    auto manager = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!manager) {
        return;
    }
    manager->UserStarted(userId);
}

void UserController::BroadcastUserStarted(int32_t userId)
{
    // broadcast event user start.
}

void UserController::BroadcastUserBackground(int32_t userId)
{
    // broadcast event user switch to bg.
}

void UserController::BroadcastUserForeground(int32_t userId)
{
    // broadcast event user switch to fg.
}

void UserController::BroadcastUserStopping(int32_t userId)
{
}

void UserController::BroadcastUserStopped(int32_t userId)
{
}

void UserController::SendSystemUserStart(int32_t userId)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    std::shared_ptr<UserEvent> eventData = std::make_shared<UserEvent>();
    eventData->newUserId = userId;
    auto event = InnerEvent::Get(UserEventHandler::EVENT_SYSTEM_USER_START, eventData);
    handler->SendEvent(event);
}

void UserController::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (!event) {
        return;
    }

    auto eventId = event->GetInnerEventId();
    auto eventData = event->GetSharedObject<UserEvent>();
    if (!eventData) {
        HILOG_DEBUG("no event data, event id: %{public}u.", eventId);
        return;
    }

    HILOG_DEBUG("Event id obtained: %{public}u.", eventId);
    switch (eventId) {
        case UserEventHandler::EVENT_SYSTEM_USER_START: {
            HandleSystemUserStart(eventData->newUserId);
            break;
        }
        case UserEventHandler::EVENT_SYSTEM_USER_CURRENT: {
            HandleSystemUserCurrent(eventData->oldUserId, eventData->newUserId);
            break;
        }
        case UserEventHandler::EVENT_REPORT_USER_SWITCH: {
            HandleReportUserSwitch(eventData->oldUserId, eventData->newUserId, eventData->userItem);
            break;
        }
        case UserEventHandler::EVENT_CONTINUE_USER_SWITCH: {
            HandleContinueUserSwitch(eventData->oldUserId, eventData->newUserId, eventData->userItem);
            break;
        }
        case UserEventHandler::EVENT_USER_SWITCH_TIMEOUT: {
            HandleUserSwitchTimeout(eventData->oldUserId, eventData->newUserId, eventData->userItem);
            break;
        }
        case UserEventHandler::EVENT_REPORT_USER_SWITCH_DONE: {
            HandleUserSwitchDone(eventData->newUserId);
            break;
        }
        default: {
            HILOG_WARN("Unsupported  event.");
            break;
        }
    }
}

void UserController::SendSystemUserCurrent(int32_t oldUserId, int32_t newUserId)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    std::shared_ptr<UserEvent> eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    auto event = InnerEvent::Get(UserEventHandler::EVENT_SYSTEM_USER_CURRENT, eventData);
    handler->SendEvent(event);
}

void UserController::SendReportUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    handler->RemoveEvent(UserEventHandler::EVENT_REPORT_USER_SWITCH);
    std::shared_ptr<UserEvent> eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    eventData->userItem = usrItem;
    auto event = InnerEvent::Get(UserEventHandler::EVENT_REPORT_USER_SWITCH, eventData);
    handler->SendEvent(event);
}

void UserController::SendUserSwitchTimeout(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    handler->RemoveEvent(UserEventHandler::EVENT_USER_SWITCH_TIMEOUT);
    std::shared_ptr<UserEvent> eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    eventData->userItem = usrItem;
    auto event = InnerEvent::Get(UserEventHandler::EVENT_USER_SWITCH_TIMEOUT, eventData);
    handler->SendEvent(event, USER_SWITCH_TIMEOUT);
}

void UserController::SendContinueUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    handler->RemoveEvent(UserEventHandler::EVENT_USER_SWITCH_TIMEOUT);
    std::shared_ptr<UserEvent> eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    eventData->userItem = usrItem;
    auto event = InnerEvent::Get(UserEventHandler::EVENT_CONTINUE_USER_SWITCH, eventData);
    handler->SendEvent(event);
}

void UserController::SendUserSwitchDone(int32_t userId)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    handler->RemoveEvent(UserEventHandler::EVENT_REPORT_USER_SWITCH_DONE);
    std::shared_ptr<UserEvent> eventData = std::make_shared<UserEvent>();
    eventData->newUserId = userId;
    auto event = InnerEvent::Get(UserEventHandler::EVENT_REPORT_USER_SWITCH_DONE, eventData);
    handler->SendEvent(event);
}

void UserController::HandleSystemUserStart(int32_t userId)
{
    // notify system mgr user start.
}

void UserController::HandleSystemUserCurrent(int32_t oldUserId, int32_t newUserId)
{
    // notify system mgr user switch to new.
}

void UserController::HandleReportUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    // notify user switch observers, not support yet.
}

void UserController::HandleUserSwitchTimeout(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    // other observers
    SendContinueUserSwitch(oldUserId, newUserId, usrItem);
}

void UserController::HandleContinueUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto manager = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (manager) {
        manager->StopFreezingScreen();
    }
    SendUserSwitchDone(newUserId);
}

void UserController::HandleUserSwitchDone(int32_t userId)
{
    // notify wms switching done.
    // notify user switch observers.
}
}
}
