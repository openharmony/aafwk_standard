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

#include "form_timer_mgr.h"

#include <cinttypes>

#include "ability_context.h"
#include "appexecfwk_errors.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "context/context.h"
#include "form_constants.h"
#include "form_provider_mgr.h"
#include "form_refresh_limiter.h"
#include "form_timer_option.h"
#include "form_util.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#ifdef OS_ACCOUNT_PART_ENABLED
#include "os_account_manager.h"
#endif // OS_ACCOUNT_PART_ENABLED
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
const int REQUEST_UPDATE_AT_CODE = 1;
const int REQUEST_LIMITER_CODE = 2;
const int REQUEST_DYNAMIC_CODE = 3;
const int SHIFT_BIT_LENGTH = 32;
const std::string FMS_TIME_SPEED = "fms.time_speed";
#ifndef OS_ACCOUNT_PART_ENABLED
const int DEFAULT_OS_ACCOUNT_ID = 0; // 0 is the default id when there is no os_account part
#endif // OS_ACCOUNT_PART_ENABLED

FormTimerMgr::FormTimerMgr()
{
    Init();
}
FormTimerMgr::~FormTimerMgr()
{
    ClearIntervalTimer();
}
/**
 * @brief Add form timer by timer task.
 * @param task The form timer task.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::AddFormTimer(const FormTimer &task)
{
    HILOG_INFO("%{public}s formId:%{public}s userId:%{public}d", __func__,
        std::to_string(task.formId).c_str(), task.userId);
    if (task.isUpdateAt) {
        if (task.hour >= Constants::MIN_TIME && task.hour <= Constants::MAX_HOUR &&
            task.min >= Constants::MIN_TIME && task.min <= Constants::MAX_MINUTE) {
            return AddUpdateAtTimer(task);
        } else {
            HILOG_ERROR("%{public}s failed, update at time is invalid", __func__);
            return false;
        }
    } else {
        if (task.period >= (Constants::MIN_PERIOD / timeSpeed_) && // Min period is 30 minutes
            task.period <= (Constants::MAX_PERIOD / timeSpeed_) && // Max period is 1 week
            task.period % (Constants::MIN_PERIOD / timeSpeed_) == 0) {
            return AddIntervalTimer(task);
        } else {
            HILOG_ERROR("%{public}s failed, interval time is invalid", __func__);
            return false;
        }
    }
}
/**
 * @brief Add duration form timer.
 * @param formId The Id of the form.
 * @param updateDuration Update duration
 * @param userId User ID.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::AddFormTimer(const int64_t formId, const long updateDuration, const int32_t userId)
{
    auto duration = updateDuration / timeSpeed_;
    HILOG_INFO("%{public}s formId:%{public}s duration:%{public}s", __func__,
        std::to_string(formId).c_str(), std::to_string(duration).c_str());
    FormTimer timerTask(formId, duration, userId);
    return AddFormTimer(timerTask);
}
/**
 * @brief Add scheduled form timer.
 * @param formId The Id of the form.
 * @param updateAtHour Hour.
 * @param updateAtMin Min.
 * @param userId User ID.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::AddFormTimer(const int64_t formId, const long updateAtHour,
                                const long updateAtMin, const int32_t userId)
{
    HILOG_INFO("%{public}s formId:%{public}s time:%{public}s-%{public}s", __func__,
        std::to_string(formId).c_str(), std::to_string(updateAtHour).c_str(), std::to_string(updateAtMin).c_str());
    FormTimer timerTask(formId, updateAtHour, updateAtMin, userId);
    return AddFormTimer(timerTask);
}
/**
 * @brief Remove form timer by form id.
 * @param formId The Id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::RemoveFormTimer(const int64_t formId)
{
    HILOG_INFO("%{public}s, task: %{public}" PRId64 "", __func__, formId);

    if (!DeleteIntervalTimer(formId)) {
        if (!DeleteUpdateAtTimer(formId)) {
            HILOG_ERROR("%{public}s, failed to DeleteUpdateAtTimer", __func__);
            return false;
        }
    }

    if (!DeleteDynamicItem(formId)) {
        HILOG_ERROR("%{public}s, failed to DeleteDynamicItem", __func__);
        return false;
    }
    refreshLimiter_.DeleteItem(formId);

    return true;
}
/**
 * @brief Update form timer.
 * @param formId The Id of the form.
 * @param type Timer type.
 * @param timerCfg Timer config.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::UpdateFormTimer(const int64_t formId, const UpdateType &type, const FormTimerCfg &timerCfg)
{
    if (!timerCfg.enableUpdate) {
        HILOG_WARN("%{public}s, enableUpdate is false", __func__);
        return false;
    }

    switch (type) {
        case UpdateType::TYPE_INTERVAL_CHANGE: {
            return UpdateIntervalValue(formId, timerCfg);
        }
        case UpdateType::TYPE_ATTIME_CHANGE: {
            return UpdateAtTimerValue(formId, timerCfg);
        }
        case UpdateType::TYPE_INTERVAL_TO_ATTIME: {
            return IntervalToAtTimer(formId, timerCfg);
        }
        case UpdateType::TYPE_ATTIME_TO_INTERVAL: {
            return AtTimerToIntervalTimer(formId, timerCfg);
        }
        default: {
            HILOG_ERROR("%{public}s failed, invalid UpdateType", __func__);
            return false;
        }
    }
}
/**
 * @brief Update Interval timer task value.
 * @param formId The Id of the form.
 * @param timerCfg task value.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::UpdateIntervalValue(const int64_t formId, const FormTimerCfg &timerCfg)
{
    if (timerCfg.updateDuration < Constants::MIN_PERIOD || timerCfg.updateDuration > Constants::MAX_PERIOD
        || (timerCfg.updateDuration % Constants::MIN_PERIOD) != 0) {
        HILOG_ERROR("%{public}s failed, invalid param", __func__);
        return false;
    }

    std::lock_guard<std::mutex> lock(intervalMutex_);
    auto intervalTask = intervalTimerTasks_.find(formId);
    if (intervalTask != intervalTimerTasks_.end()) {
        intervalTask->second.period = timerCfg.updateDuration / timeSpeed_;
        return true;
    } else {
        HILOG_ERROR("%{public}s failed, the interval timer is not exist", __func__);
        return false;
    }
}
/**
 * @brief Update update at timer task value.
 * @param formId The Id of the form.
 * @param timerCfg task value.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::UpdateAtTimerValue(const int64_t formId, const FormTimerCfg &timerCfg)
{
    if (timerCfg.updateAtHour < Constants::MIN_TIME || timerCfg.updateAtHour > Constants::MAX_HOUR
        || timerCfg.updateAtMin < Constants::MIN_TIME || timerCfg.updateAtMin > Constants::MAX_MINUTE) {
        HILOG_ERROR("%{public}s failed, time is invalid", __func__);
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);
        std::list<UpdateAtItem>::iterator itItem;
        UpdateAtItem changedItem;
        for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
            if (itItem->refreshTask.formId == formId) {
                changedItem = *itItem;
                updateAtTimerTasks_.erase(itItem);
                break;
            }
        }

        if (changedItem.refreshTask.formId == 0) {
            HILOG_ERROR("%{public}s failed, the update at timer is not exist", __func__);
            return false;
        }
        changedItem.refreshTask.hour = timerCfg.updateAtHour;
        changedItem.refreshTask.min = timerCfg.updateAtMin;
        changedItem.updateAtTime = changedItem.refreshTask.hour * Constants::MIN_PER_HOUR + changedItem.refreshTask.min;
        AddUpdateAtItem(changedItem);
    }

    if (!UpdateAtTimerAlarm()) {
        HILOG_ERROR("%{public}s, failed to update attimer alarm.", __func__);
        return false;
    }
    return true;
}
/**
 * @brief Interval timer task to update at timer task.
 * @param formId The Id of the form.
 * @param timerCfg task value.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::IntervalToAtTimer(const int64_t formId, const FormTimerCfg &timerCfg)
{
    if (timerCfg.updateAtHour < Constants::MIN_TIME || timerCfg.updateAtHour > Constants::MAX_HOUR
        || timerCfg.updateAtMin < Constants::MIN_TIME || timerCfg.updateAtMin > Constants::MAX_MINUTE) {
        HILOG_ERROR("%{public}s failed, time is invalid", __func__);
        return false;
    }

    std::lock_guard<std::mutex> lock(intervalMutex_);
    FormTimer timerTask;
    auto intervalTask = intervalTimerTasks_.find(formId);
    if (intervalTask != intervalTimerTasks_.end()) {
        timerTask = intervalTask->second;
        intervalTimerTasks_.erase(intervalTask);

        timerTask.isUpdateAt = true;
        timerTask.hour = timerCfg.updateAtHour;
        timerTask.min = timerCfg.updateAtMin;
        if (!AddUpdateAtTimer(timerTask)) {
            HILOG_ERROR("%{public}s, failed to add update at timer", __func__);
            return false;
        }
        return true;
    } else {
        HILOG_ERROR("%{public}s failed, the interval timer is not exist", __func__);
        return false;
    }
}
/**
 * @brief Update at timer task to interval timer task.
 * @param formId The Id of the form.
 * @param timerCfg task value.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::AtTimerToIntervalTimer(const int64_t formId, const FormTimerCfg &timerCfg)
{
    if (timerCfg.updateDuration < Constants::MIN_PERIOD || timerCfg.updateDuration > Constants::MAX_PERIOD
        || (timerCfg.updateDuration % Constants::MIN_PERIOD) != 0) {
        HILOG_ERROR("%{public}s failed, time is invalid", __func__);
        return false;
    }

    UpdateAtItem targetItem;
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);
        std::list<UpdateAtItem>::iterator itItem;
        for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
            if (itItem->refreshTask.formId == formId) {
                targetItem = *itItem;
                updateAtTimerTasks_.erase(itItem);
                break;
            }
        }
    }

    if (!UpdateAtTimerAlarm()) {
        HILOG_ERROR("%{public}s, failed to update attimer alarm.", __func__);
        return false;
    }

    if (targetItem.refreshTask.formId == 0) {
        HILOG_ERROR("%{public}s failed, the update at timer is not exist", __func__);
        return false;
    }
    targetItem.refreshTask.isUpdateAt = false;
    targetItem.refreshTask.period = timerCfg.updateDuration;
    targetItem.refreshTask.refreshTime = INT64_MAX;
    if (!AddIntervalTimer(targetItem.refreshTask)) {
        HILOG_ERROR("%{public}s, failed to add interval timer", __func__);
        return false;
    }
    return true;
}
/**
 * @brief Is limiter enable refresh.
 * @param formId The Id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::IsLimiterEnableRefresh(const int64_t formId)
{
    return refreshLimiter_.IsEnableRefresh(formId);
}
/**
 * @brief Increase refresh count.
 * @param formId The Id of the form.
 */
void FormTimerMgr::IncreaseRefreshCount(const int64_t formId)
{
    refreshLimiter_.Increase(formId);
}
/**
 * @brief Set next refresh time.
 * @param formId The Id of the form.
 * @param nextGapTime Next gap time(ms).
 * @param userId User ID.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::SetNextRefreshTime(const int64_t formId, const long nextGapTime, const int32_t userId)
{
    if (nextGapTime < Constants::MIN_NEXT_TIME) {
        HILOG_ERROR("%{public}s failed, nextGapTime is invalid, nextGapTime:%{public}ld", __func__, nextGapTime);
        return false;
    }
    auto timeSinceEpoch = std::chrono::steady_clock::now().time_since_epoch();
    int64_t timeInSec = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch).count();
    int64_t refreshTime = timeInSec + nextGapTime * Constants::MS_PER_SECOND / timeSpeed_;
    HILOG_INFO("%{public}s currentTime:%{public}s refreshTime:%{public}s", __func__,
        std::to_string(timeInSec).c_str(), std::to_string(refreshTime).c_str());
    std::lock_guard<std::mutex> lock(refreshMutex_);
    bool isExist = false;
    for (auto &refreshItem : dynamicRefreshTasks_) {
        if ((refreshItem.formId == formId) && (refreshItem.userId == userId)) {
            refreshItem.settedTime = refreshTime;
            isExist = true;
            break;
        }
    }
    if (!isExist) {
        DynamicRefreshItem theItem;
        theItem.formId = formId;
        theItem.settedTime = refreshTime;
        theItem.userId = userId;
        dynamicRefreshTasks_.emplace_back(theItem);
    }
    std::sort(dynamicRefreshTasks_.begin(), dynamicRefreshTasks_.end(), CompareDynamicRefreshItem);
    if (!UpdateDynamicAlarm()) {
        HILOG_ERROR("%{public}s, failed to UpdateDynamicAlarm", __func__);
        return false;
    }
    if (!UpdateLimiterAlarm()) {
        HILOG_ERROR("%{public}s, failed to UpdateLimiterAlarm", __func__);
        return false;
    }
    refreshLimiter_.AddItem(formId);
    SetEnableFlag(formId, false);

    return true;
}

void FormTimerMgr::SetEnableFlag(int64_t formId, bool flag)
{
    // try interval list
    auto iter = intervalTimerTasks_.find(formId);
    if (iter != intervalTimerTasks_.end()) {
        iter->second.isEnable = flag;
        HILOG_INFO("%{public}s, formId:%{public}" PRId64 ", isEnable:%{public}d", __func__, formId, flag ? 1 : 0);
        return;
    }
}

/**
 * @brief Get refresh count.
 * @param formId The Id of the form.
 * @return Returns refresh count.
 */
int FormTimerMgr::GetRefreshCount(const int64_t formId) const
{
    return refreshLimiter_.GetRefreshCount(formId);
}
/**
 * @brief Mark remind.
 * @param formId The Id of the form.
 * @return true or false.
 */
void FormTimerMgr::MarkRemind(const int64_t  formId)
{
    refreshLimiter_.MarkRemind(formId);
}
/**
 * @brief Add update at timer.
 * @param task Update time task.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::AddUpdateAtTimer(const FormTimer &task)
{
    HILOG_INFO("%{public}s start", __func__);
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);
        for (auto &updateAtTimer : updateAtTimerTasks_) {
            if (updateAtTimer.refreshTask.formId == task.formId) {
                HILOG_WARN(
                    "%{public}s, already exist formTimer, formId:%{public}" PRId64 " task", __func__, task.formId);
                return true;
            }
        }

        UpdateAtItem atItem;
        atItem.refreshTask = task;
        atItem.updateAtTime = task.hour * Constants::MIN_PER_HOUR + task.min;

        AddUpdateAtItem(atItem);
    }
    if (!UpdateLimiterAlarm()) {
        HILOG_ERROR("%{public}s, failed to UpdateLimiterAlarm", __func__);
        return false;
    }

    if (!UpdateAtTimerAlarm()) {
        HILOG_ERROR("%{public}s, failed to update attimer alarm.", __func__);
        return false;
    }

    return refreshLimiter_.AddItem(task.formId);
}
/**
 * @brief Add update interval timer task.
 * @param task Update interval timer task.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::AddIntervalTimer(const FormTimer &task)
{
    HILOG_INFO("%{public}s start", __func__);
    {
        std::lock_guard<std::mutex> lock(intervalMutex_);
        EnsureInitIntervalTimer();
        if (intervalTimerTasks_.find(task.formId) != intervalTimerTasks_.end()) {
            HILOG_WARN("%{public}s, already exist formTimer, formId:%{public}" PRId64 " task", __func__, task.formId);
            return true;
        }
        intervalTimerTasks_.emplace(task.formId, task);
    }
    if (!UpdateLimiterAlarm()) {
        HILOG_ERROR("%{public}s, failed to UpdateLimiterAlarm", __func__);
        return false;
    }
    return refreshLimiter_.AddItem(task.formId);
}
/**
 * @brief Add update at timer item.
 * @param task Update at timer item.
 */
void FormTimerMgr::AddUpdateAtItem(const UpdateAtItem &atItem)
{
    if (updateAtTimerTasks_.empty()) {
        updateAtTimerTasks_.emplace_back(atItem);
        return;
    }

    UpdateAtItem firstItem = updateAtTimerTasks_.front();
    if (atItem.updateAtTime < firstItem.updateAtTime) {
        updateAtTimerTasks_.emplace_front(atItem);
        return;
    }

    bool isInsert = false;
    std::list<UpdateAtItem>::iterator itItem;
    for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
        if (atItem.updateAtTime < itItem->updateAtTime) {
            updateAtTimerTasks_.insert(itItem, atItem);
            isInsert = true;
            break;
        }
    }

    if (!isInsert) {
        updateAtTimerTasks_.emplace_back(atItem);
    }
}
/**
 * @brief Handle system time changed.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::HandleSystemTimeChanged()
{
    HILOG_INFO("%{public}s start", __func__);
    if (!updateAtTimerTasks_.empty()) {
        atTimerWakeUpTime_ = LONG_MAX;
        UpdateAtTimerAlarm();
    }
    UpdateLimiterAlarm();
    HILOG_INFO("%{public}s end", __func__);
    return true;
}
/**
 * @brief Reset form limiter.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::HandleResetLimiter()
{
    HILOG_INFO("%{public}s start", __func__);

    std::vector<FormTimer> remindTasks;
    bool bGetTasks = GetRemindTasks(remindTasks);
    if (bGetTasks) {
        HILOG_INFO("%{public}s failed, remind when reset limiter", __func__);
        for (auto &task : remindTasks) {
            ExecTimerTask(task);
        }
    }

    HILOG_INFO("%{public}s end", __func__);
    return true;
}
/**
 * @brief Update attime trigger.
 * @param updateTime Update time.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::OnUpdateAtTrigger(long updateTime)
{
    HILOG_INFO("%{public}s start, updateTime:%{public}ld", __func__, updateTime);
    std::vector<UpdateAtItem> updateList;
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);
        std::list<UpdateAtItem>::iterator itItem;
        for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
            if (itItem->updateAtTime == updateTime && itItem->refreshTask.isEnable) {
                updateList.emplace_back(*itItem);
            }
        }
    }

    if (!UpdateAtTimerAlarm()) {
        HILOG_ERROR("%{public}s, failed to update attimer alarm.", __func__);
        return false;
    }

    if (!updateList.empty()) {
        HILOG_INFO("%{public}s, update at timer triggered, trigged time: %{public}ld", __func__, updateTime);
        for (auto &item : updateList) {
            ExecTimerTask(item.refreshTask);
        }
    }

    HILOG_INFO("%{public}s end", __func__);
    return true;
}
/**
 * @brief Dynamic time trigger.
 * @param updateTime Update time.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::OnDynamicTimeTrigger(int64_t updateTime)
{
    HILOG_INFO("%{public}s start, updateTime:%{public}" PRId64 "", __func__, updateTime);
    std::vector<FormTimer> updateList;
    {
        std::lock_guard<std::mutex> lock(dynamicMutex_);
        auto timeSinceEpoch = std::chrono::steady_clock::now().time_since_epoch();
        auto timeInSec = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch).count();
        int64_t markedTime = timeInSec + Constants::ABS_REFRESH_MS;
        std::vector<DynamicRefreshItem>::iterator itItem;
        for (itItem = dynamicRefreshTasks_.begin(); itItem != dynamicRefreshTasks_.end();) {
            if (itItem->settedTime <= updateTime || itItem->settedTime <= markedTime) {
                if (refreshLimiter_.IsEnableRefresh(itItem->formId)) {
                    FormTimer timerTask(itItem->formId, true, itItem->userId);
                    updateList.emplace_back(timerTask);
                }
                SetIntervalEnableFlag(itItem->formId, true);
                itItem = dynamicRefreshTasks_.erase(itItem);
            } else {
                itItem++;
            }
        }
        std::sort(dynamicRefreshTasks_.begin(), dynamicRefreshTasks_.end(),  CompareDynamicRefreshItem);
    }

    if (!UpdateDynamicAlarm()) {
        HILOG_ERROR("%{public}s, failed to update dynamic alarm.", __func__);
        return false;
    }

    if (!updateList.empty()) {
        HILOG_INFO("%{public}s triggered, trigged time: %{public}" PRId64 "", __func__, updateTime);
        for (auto &task : updateList) {
            ExecTimerTask(task);
        }
    }

    HILOG_INFO("%{public}s end", __func__);
    return true;
}
/**
 * @brief Get remind tasks.
 * @param remindTasks Remind tasks.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::GetRemindTasks(std::vector<FormTimer> &remindTasks)
{
    HILOG_INFO("%{public}s start", __func__);
    std::vector<int64_t> remindList = refreshLimiter_.GetRemindListAndResetLimit();
    for (int64_t id : remindList) {
        FormTimer formTimer(id, false);
        remindTasks.emplace_back(formTimer);
    }

    if (!UpdateLimiterAlarm()) {
        HILOG_ERROR("%{public}s, failed to UpdateLimiterAlarm", __func__);
        return false;
    }

    if (remindTasks.size() > 0) {
        HILOG_INFO("%{public}s end", __func__);
        return true;
    } else {
        HILOG_INFO("%{public}s end, remindTasks is empty", __func__);
        return false;
    }
}
/**
 * @brief Set enableFlag for interval timer task.
 * @param formId The Id of the form.
 * @param flag Enable flag.
 */
void FormTimerMgr::SetIntervalEnableFlag(int64_t formId, bool flag)
{
    std::lock_guard<std::mutex> lock(intervalMutex_);
    // try interval list
    auto refreshTask = intervalTimerTasks_.find(formId);
    if (refreshTask != intervalTimerTasks_.end()) {
        refreshTask->second.isEnable = flag;
        HILOG_INFO("%{public}s, formId:%{public}" PRId64 ", isEnable:%{public}d", __func__, formId, flag ? 1 : 0);
        return;
    }
}
/**
 * @brief Get interval timer task.
 * @param formId The Id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::GetIntervalTimer(const int64_t formId, FormTimer &formTimer)
{
    HILOG_INFO("%{public}s start", __func__);
    std::lock_guard<std::mutex> lock(intervalMutex_);
    auto intervalTask = intervalTimerTasks_.find(formId);
    if (intervalTask == intervalTimerTasks_.end()) {
        HILOG_INFO("%{public}s, interval timer not find", __func__);
        return false;
    }
    formTimer = intervalTask->second;
    HILOG_INFO("%{public}s, get interval timer successfully", __func__);
    return true;
}
/**
 * @brief Get update at timer.
 * @param formId The Id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::GetUpdateAtTimer(const int64_t formId, UpdateAtItem &updateAtItem)
{
    HILOG_INFO("%{public}s start", __func__);
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);
        std::list<UpdateAtItem>::iterator itItem;
        for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
            if (itItem->refreshTask.formId == formId) {
                updateAtItem.refreshTask = itItem->refreshTask;
                updateAtItem.updateAtTime = itItem->updateAtTime;
                HILOG_INFO("%{public}s, get update at timer successfully", __func__);
                return true;
            }
        }
    }
    HILOG_INFO("%{public}s, update at timer not find", __func__);
    return false;
}
/**
 * @brief Get dynamic refresh item.
 * @param formId The Id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::GetDynamicItem(const int64_t formId, DynamicRefreshItem &dynamicItem)
{
    HILOG_INFO("%{public}s start", __func__);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    std::vector<DynamicRefreshItem>::iterator itItem;
    for (itItem = dynamicRefreshTasks_.begin(); itItem != dynamicRefreshTasks_.end();) {
        if (itItem->formId == formId) {
            dynamicItem.formId = itItem->formId;
            dynamicItem.settedTime = itItem->settedTime;
            dynamicItem.userId = itItem->userId;
            HILOG_INFO("%{public}s, get dynamic item successfully", __func__);
            return true;
        }
    }
    HILOG_INFO("%{public}s, dynamic item not find", __func__);
    return false;
}
/**
 * @brief Set time speed.
 * @param timeSpeed The time speed.
 */
void FormTimerMgr::SetTimeSpeed(int32_t timeSpeed)
{
    HILOG_INFO("%{public}s set time speed to:%{public}d", __func__, timeSpeed);
    timeSpeed_ = timeSpeed;
    HandleResetLimiter();
    ClearIntervalTimer();
}
/**
 * @brief Delete interval timer task.
 * @param formId The Id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::DeleteIntervalTimer(const int64_t formId)
{
    HILOG_INFO("%{public}s start", __func__);
    bool isExist = false;
    std::lock_guard<std::mutex> lock(intervalMutex_);
    auto intervalTask = intervalTimerTasks_.find(formId);
    if (intervalTask != intervalTimerTasks_.end()) {
        intervalTimerTasks_.erase(intervalTask);
        isExist = true;
    }

    if (intervalTimerTasks_.empty()) {
        ClearIntervalTimer();
    }
    HILOG_INFO("%{public}s end", __func__);
    return isExist;
}
/**
 * @brief Delete update at timer.
 * @param formId The Id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::DeleteUpdateAtTimer(const int64_t formId)
{
    HILOG_INFO("%{public}s start", __func__);
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);
        std::list<UpdateAtItem>::iterator itItem;
        for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
            if (itItem->refreshTask.formId == formId) {
                updateAtTimerTasks_.erase(itItem);
                break;
            }
        }
    }

    if (!UpdateAtTimerAlarm()) {
        HILOG_ERROR("%{public}s, failed to update attimer alarm.", __func__);
        return false;
    }
    HILOG_INFO("%{public}s end", __func__);
    return true;
}
/**
 * @brief Delete dynamic refresh item.
 * @param formId The Id of the form.
 */
bool FormTimerMgr::DeleteDynamicItem(const int64_t formId)
{
    HILOG_INFO("%{public}s start", __func__);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    std::vector<DynamicRefreshItem>::iterator itItem;
    for (itItem = dynamicRefreshTasks_.begin(); itItem != dynamicRefreshTasks_.end();) {
        if (itItem->formId == formId) {
            itItem = dynamicRefreshTasks_.erase(itItem);
            if (itItem != dynamicRefreshTasks_.end()) {
                SetIntervalEnableFlag(itItem->formId, true);
            }
            break;
        }
    }
    std::sort(dynamicRefreshTasks_.begin(), dynamicRefreshTasks_.end(),  CompareDynamicRefreshItem);

    if (!UpdateDynamicAlarm()) {
        HILOG_ERROR("%{public}s, failed to UpdateDynamicAlarm", __func__);
        return false;
    }
    HILOG_INFO("%{public}s end", __func__);
    return true;
}
/**
* @brief interval timer task timeout.
*/
void FormTimerMgr::OnIntervalTimeOut()
{
    HILOG_INFO("%{public}s start", __func__);
    std::lock_guard<std::mutex> lock(intervalMutex_);
    std::vector<FormTimer> updateList;
    int64_t currentTime = FormUtil::GetCurrentNanosecond() / Constants::TIME_1000000;
    for (auto &intervalPair : intervalTimerTasks_) {
        FormTimer &intervalTask = intervalPair.second;
        if ((intervalTask.refreshTime == INT64_MAX || (currentTime - intervalTask.refreshTime) >= intervalTask.period ||
            std::abs((currentTime - intervalTask.refreshTime) - intervalTask.period) < Constants::ABS_TIME) &&
            intervalTask.isEnable && refreshLimiter_.IsEnableRefresh(intervalTask.formId)) {
            intervalTask.refreshTime = currentTime;
            updateList.emplace_back(intervalTask);
        }
    }

    if (!updateList.empty()) {
        for (auto &task : updateList) {
            ExecTimerTask(task);
        }
    }
    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @brief Update at timer task alarm.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::UpdateAtTimerAlarm()
{
    HILOG_INFO("%{public}s start", __func__);
    struct tm tmAtTime = {0};
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime_r(&tt, &tmAtTime);
    if (ptm == nullptr) {
        HILOG_ERROR("%{public}s failed, localtime error", __func__);
        return false;
    }

    long nowAtTime = tmAtTime.tm_hour * Constants::MIN_PER_HOUR + tmAtTime.tm_min;
    int64_t currentTime = FormUtil::GetCurrentMillisecond();
    UpdateAtItem findedItem;
    bool bFinded = FindNextAtTimerItem(nowAtTime, findedItem);
    if (!bFinded) {
        if (!updateAtTimerTasks_.empty()) {
            HILOG_WARN("%{public}s, updateAtTimerTasks_ is not empty", __func__);
            return true;
        }
        ClearUpdateAtTimerResource();
        atTimerWakeUpTime_ = LONG_MAX;
        HILOG_INFO("%{public}s, no update at task in system now.", __func__);
        return true;
    }

    long nextWakeUpTime = findedItem.updateAtTime;
    tmAtTime.tm_sec = 0;
    tmAtTime.tm_hour = findedItem.refreshTask.hour;
    tmAtTime.tm_min = findedItem.refreshTask.min;
    int64_t selectTime = FormUtil::GetMillisecondFromTm(tmAtTime);
    if (selectTime < currentTime) {
        selectTime += Constants::MS_PER_DAY;
        nextWakeUpTime += (Constants::HOUR_PER_DAY * Constants::MIN_PER_HOUR);
    }
    HILOG_INFO("%{public}s, selectTime: %{public}" PRId64 ", currentTime: %{public}" PRId64 ".",
        __func__, selectTime, currentTime);

    if (nextWakeUpTime == atTimerWakeUpTime_) {
        HILOG_WARN("%{public}s end, wakeUpTime not change, no need update alarm.", __func__);
        return true;
    }

    auto timerOption = std::make_shared<FormTimerOption>();
    timerOption->SetType(((unsigned int)(timerOption->TIMER_TYPE_REALTIME))
      | ((unsigned int)(timerOption->TIMER_TYPE_WAKEUP)));
    timerOption->SetRepeat(false);
    timerOption->SetInterval(0);
    int32_t userId = findedItem.refreshTask.userId;
    std::shared_ptr<WantAgent> wantAgent = GetUpdateAtWantAgent(findedItem.updateAtTime, userId);
    if (wantAgent == nullptr) {
        HILOG_ERROR("%{public}s, failed to create wantAgent.", __func__);
        return false;
    }
    timerOption->SetWantAgent(wantAgent);

    atTimerWakeUpTime_ = nextWakeUpTime;
    if (currentUpdateAtWantAgent != nullptr) {
        ClearUpdateAtTimerResource();
    }
    auto timeSinceEpoch = std::chrono::steady_clock::now().time_since_epoch();
    int64_t timeInSec = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch).count();
    int64_t nextTime = timeInSec + (selectTime - currentTime);
    HILOG_INFO("%{public}s, nextTime: %{public}" PRId64 ".", __func__, nextTime);

    currentUpdateAtWantAgent = wantAgent;
    updateAtTimerId_ = MiscServices::TimeServiceClient::GetInstance()->CreateTimer(timerOption);
    bool bRet = MiscServices::TimeServiceClient::GetInstance()->StartTimer(updateAtTimerId_,
        static_cast<uint64_t>(nextTime));
    if (!bRet) {
        HILOG_ERROR("%{public}s failed, init update at timer task error", __func__);
        return false;
    }

    HILOG_INFO("%{public}s end", __func__);
    return true;
}

/**
 * @brief Get WantAgent.
 * @param updateAtTime The next update time.
 * @return Returns WantAgent.
 */
std::shared_ptr<WantAgent> FormTimerMgr::GetUpdateAtWantAgent(long updateAtTime, int32_t userId)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("", "", "");
    want->SetElement(element);
    want->SetAction(Constants::ACTION_UPDATEATTIMER);
    want->SetParam(Constants::KEY_ACTION_TYPE, Constants::TYPE_STATIC_UPDATE);
    want->SetParam(Constants::KEY_WAKEUP_TIME, updateAtTime);

    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.emplace_back(want);
    WantAgentInfo wantAgentInfo(REQUEST_UPDATE_AT_CODE, WantAgentConstant::OperationType::SEND_COMMON_EVENT,
        WantAgentConstant::Flags::UPDATE_PRESENT_FLAG, wants, nullptr);
    return IN_PROCESS_CALL(WantAgentHelper::GetWantAgent(wantAgentInfo, userId));
}

/**
 * @brief Clear update at timer resource.
 */
void FormTimerMgr::ClearUpdateAtTimerResource()
{
    HILOG_INFO("%{public}s start", __func__);
    if (updateAtTimerId_ != 0L) {
        HILOG_INFO("%{public}s clear update at timer start", __func__);
        MiscServices::TimeServiceClient::GetInstance()->StopTimer(updateAtTimerId_);
        MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(updateAtTimerId_);
        HILOG_INFO("%{public}s clear update at timer end", __func__);
        updateAtTimerId_ = 0L;
    }
    if (currentUpdateAtWantAgent != nullptr) {
        WantAgentHelper::Cancel(currentUpdateAtWantAgent);
        currentUpdateAtWantAgent = nullptr;
    }
    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @brief Update limiter task alarm.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::UpdateLimiterAlarm()
{
    HILOG_INFO("%{public}s start", __func__);
    if (limiterTimerId_ != 0L) {
        HILOG_INFO("%{public}s clear limiter timer start", __func__);
        MiscServices::TimeServiceClient::GetInstance()->StopTimer(limiterTimerId_);
        MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(limiterTimerId_);
        HILOG_INFO("%{public}s clear limiter timer end", __func__);
        limiterTimerId_ = 0L;
    }

    // make limiter wakeup time
    struct tm tmAtTime = {0};
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime_r(&tt, &tmAtTime);
    if (ptm == nullptr) {
        HILOG_ERROR("%{public}s failed, localtime error", __func__);
        return false;
    }
    tmAtTime.tm_sec = Constants::MAX_SECOND; // max value can be 61
    tmAtTime.tm_hour = Constants::MAX_HOUR;
    tmAtTime.tm_min = Constants::MAX_MINUTE;
    int64_t limiterWakeUpTime = FormUtil::GetMillisecondFromTm(tmAtTime);

    auto timerOption = std::make_shared<FormTimerOption>();
    timerOption->SetType(timerOption->TIMER_TYPE_WAKEUP);
    timerOption->SetRepeat(false);
    timerOption->SetInterval(0);
    std::shared_ptr<WantAgent> wantAgent = GetLimiterWantAgent();
    if (!wantAgent) {
        HILOG_ERROR("%{public}s, failed to create wantAgent.", __func__);
        return false;
    }
    timerOption->SetWantAgent(wantAgent);

    if (currentLimiterWantAgent != nullptr) {
        ClearLimiterTimerResource();
    }
    currentLimiterWantAgent = wantAgent;

    limiterTimerId_ = MiscServices::TimeServiceClient::GetInstance()->CreateTimer(timerOption);
    bool bRet = MiscServices::TimeServiceClient::GetInstance()->StartTimer(limiterTimerId_,
        static_cast<uint64_t>(limiterWakeUpTime));
    if (!bRet) {
        HILOG_ERROR("%{public}s failed, init limiter timer task error", __func__);
        return false;
    }
    HILOG_INFO("%{public}s end", __func__);
    return true;
}
/**
 * @brief Clear limiter timer resource.
 */
void FormTimerMgr::ClearLimiterTimerResource()
{
    HILOG_INFO("%{public}s start", __func__);
    if (limiterTimerId_ != 0L) {
        HILOG_INFO("%{public}s clear limiter timer start", __func__);
        MiscServices::TimeServiceClient::GetInstance()->StopTimer(limiterTimerId_);
        MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(limiterTimerId_);
        HILOG_INFO("%{public}s clear limiter timer end", __func__);
        limiterTimerId_ = 0L;
    }
    if (currentLimiterWantAgent != nullptr) {
        WantAgentHelper::Cancel(currentLimiterWantAgent);
        currentLimiterWantAgent = nullptr;
    }
    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @brief Get WantAgent.
 * @return Returns WantAgent.
 */
std::shared_ptr<WantAgent> FormTimerMgr::GetLimiterWantAgent()
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("", "", "");
    want->SetElement(element);
    want->SetAction(Constants::ACTION_UPDATEATTIMER);
    want->SetParam(Constants::KEY_ACTION_TYPE, Constants::TYPE_RESET_LIMIT);

    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.emplace_back(want);
    WantAgentInfo wantAgentInfo(REQUEST_LIMITER_CODE, WantAgentConstant::OperationType::SEND_COMMON_EVENT,
        WantAgentConstant::Flags::UPDATE_PRESENT_FLAG, wants, nullptr);
    return IN_PROCESS_CALL(WantAgentHelper::GetWantAgent(wantAgentInfo));
}

/**
 * @brief Update dynamic refresh task alarm.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::UpdateDynamicAlarm()
{
    HILOG_INFO("%{public}s start", __func__);
    if (dynamicRefreshTasks_.empty()) {
        ClearDynamicResource();
        dynamicWakeUpTime_ = INT64_MAX;
        return true;
    }

    bool needUpdate = false;
    DynamicRefreshItem firstTask = dynamicRefreshTasks_.at(0);
    if (dynamicWakeUpTime_ != firstTask.settedTime) {
        dynamicWakeUpTime_ = firstTask.settedTime;
        needUpdate = true;
    }

    if (!needUpdate) {
        HILOG_ERROR("%{public}s failed, no need to  UpdateDynamicAlarm.", __func__);
        return true;
    }

    auto timerOption = std::make_shared<FormTimerOption>();
    timerOption->SetType(((unsigned int)(timerOption->TIMER_TYPE_REALTIME))
     | ((unsigned int)(timerOption->TIMER_TYPE_WAKEUP)));
    timerOption->SetRepeat(false);
    timerOption->SetInterval(0);
    std::shared_ptr<WantAgent> wantAgent = GetDynamicWantAgent(dynamicWakeUpTime_, firstTask.userId);
    if (!wantAgent) {
        HILOG_ERROR("%{public}s, failed to create wantAgent.", __func__);
        return false;
    }
    timerOption->SetWantAgent(wantAgent);

    if (currentDynamicWantAgent != nullptr) {
        ClearDynamicResource();
    }
    currentDynamicWantAgent = wantAgent;

    dynamicAlarmTimerId_ = MiscServices::TimeServiceClient::GetInstance()->CreateTimer(timerOption);
    bool bRet = MiscServices::TimeServiceClient::GetInstance()->StartTimer(dynamicAlarmTimerId_,
        static_cast<uint64_t>(dynamicWakeUpTime_));
    if (!bRet) {
        HILOG_ERROR("%{public}s failed, init dynamic timer task error", __func__);
    }
    HILOG_INFO("%{public}s end, dynamicWakeUpTime_ : %{public}" PRId64 ".", __func__, dynamicWakeUpTime_);
    return true;
}
/**
 * @brief Get WantAgent.
 * @param nextTime The next update time.
 * @return Returns WantAgent.
 */
std::shared_ptr<WantAgent> FormTimerMgr::GetDynamicWantAgent(int64_t nextTime, int32_t userId)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("", "", "");
    want->SetElement(element);
    want->SetAction(Constants::ACTION_UPDATEATTIMER);
    want->SetParam(Constants::KEY_ACTION_TYPE, Constants::TYPE_DYNAMIC_UPDATE);
    int nextTimeRight = static_cast<int>(nextTime);
    int nextTimLeft = static_cast<int>(nextTime >> SHIFT_BIT_LENGTH);

    want->SetParam(Constants::KEY_WAKEUP_TIME_LEFT, nextTimLeft);
    want->SetParam(Constants::KEY_WAKEUP_TIME_RIGHT, nextTimeRight);
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.emplace_back(want);
    WantAgentInfo wantAgentInfo(REQUEST_DYNAMIC_CODE, WantAgentConstant::OperationType::SEND_COMMON_EVENT,
        WantAgentConstant::Flags::UPDATE_PRESENT_FLAG, wants, nullptr);
    return IN_PROCESS_CALL(WantAgentHelper::GetWantAgent(wantAgentInfo, userId));
}

/**
 * @brief Clear dynamic refresh resource.
 */
void FormTimerMgr::ClearDynamicResource()
{
    HILOG_INFO("%{public}s start", __func__);
    if (dynamicAlarmTimerId_ != 0L) {
        HILOG_INFO("%{public}s clear dynamic timer start", __func__);
        MiscServices::TimeServiceClient::GetInstance()->StopTimer(dynamicAlarmTimerId_);
        MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(dynamicAlarmTimerId_);
        HILOG_INFO("%{public}s clear dynamic timer end", __func__);
        dynamicAlarmTimerId_ = 0L;
    }

    if (currentDynamicWantAgent != nullptr) {
        WantAgentHelper::Cancel(currentDynamicWantAgent);
        currentDynamicWantAgent = nullptr;
    }
    HILOG_INFO("%{public}s end", __func__);
}
/**
 * @brief Fint next at timer item.
 * @param nowTime Update time.
 * @param updateAtItem Next at timer item.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::FindNextAtTimerItem(const long nowTime, UpdateAtItem &updateAtItem)
{
    HILOG_INFO("%{public}s start", __func__);
    if (updateAtTimerTasks_.empty()) {
        HILOG_WARN("%{public}s, updateAtTimerTasks_ is empty", __func__);
        return false;
    }

    std::lock_guard<std::mutex> lock(updateAtMutex_);
    std::list<UpdateAtItem>::iterator itItem;
    for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
        if (itItem->updateAtTime > nowTime) {
            updateAtItem = *itItem;
            break;
        }
    }

    if (itItem == updateAtTimerTasks_.end()) {
        updateAtItem = updateAtTimerTasks_.front();
    }
    HILOG_INFO("%{public}s end", __func__);
    return true;
}

/**
 * @brief Ensure init interval timer resource.
 */
void FormTimerMgr::EnsureInitIntervalTimer()
{
    HILOG_INFO("%{public}s, init base timer task", __func__);
    if (intervalTimer_ != nullptr) {
        return;
    }

    intervalTimer_ = std::make_shared<Utils::Timer>("interval timer");
    auto timeCallback = []() { FormTimerMgr::GetInstance().OnIntervalTimeOut(); };
    intervalTimer_->Register(timeCallback, Constants::MIN_PERIOD / timeSpeed_);
    intervalTimer_->Setup();

    HILOG_INFO("%{public}s end", __func__);
}
/**
 * @brief Clear interval timer resource.
 */
void FormTimerMgr::ClearIntervalTimer()
{
    HILOG_INFO("%{public}s start", __func__);
    if (intervalTimer_ != nullptr) {
        intervalTimer_->Shutdown();
        intervalTimer_.reset();
    }
    HILOG_INFO("%{public}s end", __func__);
}
/**
 * @brief Creat thread pool for timer task.
 */

void FormTimerMgr::CreatTaskThreadExecutor()
{
    HILOG_INFO("%{public}s start", __func__);
    if (taskExecutor_ == nullptr) {
        taskExecutor_ = std::make_unique<ThreadPool>("timer task thread");
        taskExecutor_->Start(Constants::WORK_POOL_SIZE);
    }
    HILOG_INFO("%{public}s end", __func__);
    return;
}

/**
 * @brief Execute Form timer task.
 * @param timerTask Form timer task.
 */
void FormTimerMgr::ExecTimerTask(const FormTimer &timerTask)
{
    HILOG_INFO("%{public}s start", __func__);
    CreatTaskThreadExecutor();
    if (taskExecutor_ != nullptr) {
        HILOG_INFO("%{public}s run", __func__);
        AAFwk::Want want;
        if (timerTask.isCountTimer) {
            want.SetParam(Constants::KEY_IS_TIMER, true);
        }
        // multi user
        if (IsActiveUser(timerTask.userId)) {
            HILOG_INFO("timerTask.userId is current user");
            want.SetParam(Constants::PARAM_FORM_USER_ID, timerTask.userId);
        }
        HILOG_INFO("%{public}s, userId:%{public}d", __func__, timerTask.userId);

        auto task = std::bind(&FormProviderMgr::RefreshForm, &FormProviderMgr::GetInstance(), timerTask.formId, want,
            false);
        taskExecutor_->AddTask(task);
    }
    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @brief Init.
 */
void FormTimerMgr::Init()
{
    HILOG_INFO("%{public}s start", __func__);
    timerReceiver_ = nullptr;
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(Constants::ACTION_UPDATEATTIMER);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_TIME_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED);
    matchingSkills.AddEvent(FMS_TIME_SPEED);

    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    timerReceiver_ = std::make_shared<TimerReceiver>(subscribeInfo);
    EventFwk::CommonEventManager::SubscribeCommonEvent(timerReceiver_);

    intervalTimer_ = nullptr;
    updateAtTimerId_ = 0L;
    dynamicAlarmTimerId_ = 0L;
    limiterTimerId_ = 0L;
    taskExecutor_ = nullptr;

    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @brief Receiver Constructor.
 * @param subscriberInfo Subscriber info.
 */
FormTimerMgr::TimerReceiver::TimerReceiver(const EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : EventFwk::CommonEventSubscriber(subscriberInfo)
{}
/**
 * @brief Receive common event.
 * @param eventData Common event data.
 */
void FormTimerMgr::TimerReceiver::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    AAFwk::Want want = eventData.GetWant();
    std::string action = want.GetAction();

    HILOG_INFO("%{public}s, action:%{public}s.", __func__, action.c_str());

    if (action == FMS_TIME_SPEED) {
        // Time speed must between 1 and 1000.
        auto timeSpeed = std::clamp(eventData.GetCode(), Constants::MIN_TIME_SPEED, Constants::MAX_TIME_SPEED);
        FormTimerMgr::GetInstance().SetTimeSpeed(timeSpeed);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_TIME_CHANGED
        || action == EventFwk::CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED) {
        FormTimerMgr::GetInstance().HandleSystemTimeChanged();
    } else if (action == Constants::ACTION_UPDATEATTIMER) {
        int type = want.GetIntParam(Constants::KEY_ACTION_TYPE, Constants::TYPE_STATIC_UPDATE);
        if (type == Constants::TYPE_RESET_LIMIT) {
            FormTimerMgr::GetInstance().HandleResetLimiter();
        } else if (type == Constants::TYPE_STATIC_UPDATE) {
            long updateTime = want.GetLongParam(Constants::KEY_WAKEUP_TIME, -1);
            if (updateTime < 0) {
                HILOG_ERROR("%{public}s failed, invalid updateTime:%{public}ld.", __func__, updateTime);
                return;
            }
            FormTimerMgr::GetInstance().OnUpdateAtTrigger(updateTime);
        } else if (type == Constants::TYPE_DYNAMIC_UPDATE) {
            int updateTimeLeft = want.GetIntParam(Constants::KEY_WAKEUP_TIME_LEFT, -1);
            int updateTimeRight = want.GetIntParam(Constants::KEY_WAKEUP_TIME_RIGHT, -1);
            int64_t updateTime = static_cast<int64_t>(updateTimeLeft);
            updateTime = updateTime << SHIFT_BIT_LENGTH;
            updateTime |= updateTimeRight;
            if (updateTime <= 0) {
                HILOG_ERROR("%{public}s failed, invalid updateTime:%{public}" PRId64 "", __func__, updateTime);
                return;
            }
            FormTimerMgr::GetInstance().OnDynamicTimeTrigger(updateTime);
        } else {
            HILOG_ERROR("%{public}s failed, invalid type when action is update at timer.", __func__);
        }
    } else {
        HILOG_ERROR("%{public}s failed, invalid action.", __func__);
    }
}
/**
 * @brief check if user is active or not.
 *
 * @param userId User ID.
 * @return true:active, false:inactive
 */
bool FormTimerMgr::IsActiveUser(const int32_t userId)
{
    std::vector<int32_t> activeList;
#ifdef OS_ACCOUNT_PART_ENABLED
    auto refCode = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeList);
#else // OS_ACCOUNT_PART_ENABLED
    ErrCode refCode = ERR_OK;
    activeList.push_back(DEFAULT_OS_ACCOUNT_ID);
#endif // OS_ACCOUNT_PART_ENABLED
    auto iter = std::find(activeList.begin(), activeList.end(), userId);
    if (iter != activeList.end() && refCode == ERR_OK) {
        return true;
    }
    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
