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

#include "ability_impl.h"

#include "app_log_wrapper.h"
#include "bytrace.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
const int TARGET_VERSION_THRESHOLDS = 8;

void AbilityImpl::Init(std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &record,
    std::shared_ptr<Ability> &ability, std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token,
    std::shared_ptr<ContextDeal> &contextDeal)
{
    BYTRACE(BYTRACE_TAG_ABILITY_MANAGER);
    APP_LOGI("AbilityImpl::init begin");
    if ((token == nullptr) || (application == nullptr) || (handler == nullptr) || (record == nullptr) ||
        ability == nullptr || contextDeal == nullptr) {
        APP_LOGE("AbilityImpl::init failed, token is nullptr, application is nullptr, handler is nullptr, record is "
                 "nullptr, ability is nullptr, contextDeal is nullptr");
        return;
    }

    token_ = record->GetToken();
    record->SetAbilityImpl(shared_from_this());
    ability_ = ability;
    handler_ = handler;
    ability_->SetSceneListener(
        sptr<WindowLifeCycleImpl>(new (std::nothrow) WindowLifeCycleImpl(token_, shared_from_this())));
    ability_->Init(record->GetAbilityInfo(), application, handler, token);
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    abilityLifecycleCallbacks_ = application;
    contextDeal_ = contextDeal;
    APP_LOGI("AbilityImpl::init end");
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 * @param want  The Want object to switch the life cycle.
 */
void AbilityImpl::Start(const Want &want)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        APP_LOGE("AbilityImpl::Start ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    if (ability_->GetAbilityInfo()->type == AbilityType::PAGE) {
        ability_->HandleCreateAsContinuation(want);
    }

    APP_LOGI("AbilityImpl::Start");
    ability_->OnStart(want);
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetCompatibleVersion() >= TARGET_VERSION_THRESHOLDS)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_STARTED_NEW;
    } else {
        if (ability_->GetAbilityInfo()->type == AbilityType::DATA) {
            lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
        } else {
            lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
        }
    }

    abilityLifecycleCallbacks_->OnAbilityStart(ability_);

    // Multimodal Events Register
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetCompatibleVersion() < TARGET_VERSION_THRESHOLDS)) {
        WindowEventRegister();
    }
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INITIAL. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 */
void AbilityImpl::Stop()
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        APP_LOGE("AbilityImpl::Stop ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    APP_LOGI("AbilityImpl::Stop");
    ability_->OnStop();
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetCompatibleVersion() >= TARGET_VERSION_THRESHOLDS)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_STOPED_NEW;
    } else {
        lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    }
    abilityLifecycleCallbacks_->OnAbilityStop(ability_);

    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_ACTIVE. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 */
void AbilityImpl::Active()
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        APP_LOGE("AbilityImpl::Active ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    APP_LOGI("AbilityImpl::Active");
    ability_->OnActive();

    if ((lifecycleState_ == AAFwk::ABILITY_STATE_INACTIVE) && (ability_->GetAbilityInfo()->type == AbilityType::PAGE)) {
        ability_->OnTopActiveAbilityChanged(true);
        ability_->OnWindowFocusChanged(true);
    }

    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityLifecycleCallbacks_->OnAbilityActive(ability_);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 */
void AbilityImpl::Inactive()
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        APP_LOGE("AbilityImpl::Inactive ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    APP_LOGI("AbilityImpl::Inactive");
    ability_->OnInactive();

    if ((lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE) && (ability_->GetAbilityInfo()->type == AbilityType::PAGE)) {
        ability_->OnTopActiveAbilityChanged(false);
        ability_->OnWindowFocusChanged(false);
    }

    lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    abilityLifecycleCallbacks_->OnAbilityInactive(ability_);
    APP_LOGI("%{public}s end.", __func__);
}

int AbilityImpl::GetCompatibleVersion()
{
    if (ability_) {
        return ability_->GetCompatibleVersion();
    }

    return -1;
}

void AbilityImpl::AfterUnFocused()
{
    APP_LOGI("%{public}s begin.", __func__);
    if (!ability_ || !contextDeal_ || !handler_) {
        return;
    }

    if (ability_->GetCompatibleVersion() >= TARGET_VERSION_THRESHOLDS) {
        APP_LOGI("new version ability, do nothing when after unfocused.");
        return;
    }

    APP_LOGI("old version ability, window after unfocused.");
    auto task = [abilityImpl = shared_from_this(), ability = ability_, contextDeal = contextDeal_]() {
        auto info = contextDeal->GetLifeCycleStateInfo();
        info.state = AbilityLifeCycleState::ABILITY_STATE_INACTIVE;
        Want want(*(ability->GetWant()));
        abilityImpl->HandleAbilityTransaction(want, info);
    };
    handler_->PostTask(task);
    APP_LOGI("%{public}s end.", __func__);
}

void AbilityImpl::WindowLifeCycleImpl::AfterForeground()
{
    APP_LOGI("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner && owner->GetCompatibleVersion() < TARGET_VERSION_THRESHOLDS) {
        return;
    }

    APP_LOGI("new version ability, window after foreground.");
    PacMap restoreData;
    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
        AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW, restoreData);
}

void AbilityImpl::WindowLifeCycleImpl::AfterBackground()
{
    APP_LOGI("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner && owner->GetCompatibleVersion() < TARGET_VERSION_THRESHOLDS) {
        return;
    }

    APP_LOGI("new version ability, window after background.");
    PacMap restoreData;
    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
        AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_NEW, restoreData);
}

void AbilityImpl::WindowLifeCycleImpl::AfterFocused()
{
    APP_LOGI("%{public}s.", __func__);
}

void AbilityImpl::WindowLifeCycleImpl::AfterUnFocused()
{
    APP_LOGI("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterUnFocused();
    }
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 * @param want The Want object to switch the life cycle.
 */
void AbilityImpl::Foreground(const Want &want)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        APP_LOGE("AbilityImpl::Foreground ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    APP_LOGI("AbilityImpl::Foreground");
    ability_->OnForeground(want);
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetCompatibleVersion() >= TARGET_VERSION_THRESHOLDS)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    } else {
        lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    }
    abilityLifecycleCallbacks_->OnAbilityForeground(ability_);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_BACKGROUND. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 */
void AbilityImpl::Background()
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        APP_LOGE("AbilityImpl::Background ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    APP_LOGI("AbilityImpl::Background");
    ability_->OnLeaveForeground();
    ability_->OnBackground();
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetCompatibleVersion() >= TARGET_VERSION_THRESHOLDS)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    } else {
        lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND;
    }
    abilityLifecycleCallbacks_->OnAbilityBackground(ability_);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Save data and states of an ability when it is restored by the system. and Calling information back to Ability.
 *        This method should be implemented by a Page ability.
 *
 */
void AbilityImpl::DispatchSaveAbilityState()
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        APP_LOGE("AbilityImpl::DispatchSaveAbilityState ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    APP_LOGI("AbilityImpl::DispatchSaveAbilityState");
    needSaveDate_ = true;
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Restores data and states of an ability when it is restored by the system. and Calling information back to
 * Ability. This method should be implemented by a Page ability.
 * @param instate The Want object to connect to.
 *
 */
void AbilityImpl::DispatchRestoreAbilityState(const PacMap &inState)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::DispatchRestoreAbilityState ability_ is nullptr");
        return;
    }

    hasSaveData_ = true;
    restoreData_ = inState;
    APP_LOGI("%{public}s end.", __func__);
}

void AbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{}

/**
 * @brief Connect the ability. and Calling information back to Ability.
 *
 * @param want The Want object to connect to.
 *
 */
sptr<IRemoteObject> AbilityImpl::ConnectAbility(const Want &want)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::ConnectAbility ability_ is nullptr");
        return nullptr;
    }

    APP_LOGI("AbilityImpl:: ConnectAbility");
    sptr<IRemoteObject> object = ability_->OnConnect(want);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityLifecycleCallbacks_->OnAbilityActive(ability_);
    APP_LOGI("%{public}s end.", __func__);

    return object;
}

/**
 * @brief Disconnects the connected object.
 *
 * @param want The Want object to disconnect to.
 */
void AbilityImpl::DisconnectAbility(const Want &want)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::DisconnectAbility ability_ is nullptr");
        return;
    }

    ability_->OnDisconnect(want);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Command the ability. and Calling information back to Ability.
 *
 * @param want The Want object to command to.
 *
 * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
 * destroyed, and the value false indicates a normal startup.
 *
 * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by 1
 * every time the ability is started. For example, if the ability has been started for six times, the value of startId
 * is 6.
 */
void AbilityImpl::CommandAbility(const Want &want, bool restart, int startId)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::CommandAbility ability_ is nullptr");
        return;
    }

    APP_LOGI("AbilityImpl:: CommandAbility");
    ability_->OnCommand(want, restart, startId);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityLifecycleCallbacks_->OnAbilityActive(ability_);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Gets the current Ability status.
 *
 */
int AbilityImpl::GetCurrentState()
{
    return lifecycleState_;
}

/**
 * @brief Execution the KeyDown callback of the ability
 * @param keyEvent Indicates the key-down event.
 *
 * @return Returns true if this event is handled and will not be passed further; returns false if this event is
 * not handled and should be passed to other handlers.
 *
 */
void AbilityImpl::DoKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    APP_LOGI("AbilityImpl::DoKeyDown called");
}

/**
 * @brief Execution the KeyUp callback of the ability
 * @param keyEvent Indicates the key-up event.
 *
 * @return Returns true if this event is handled and will not be passed further; returns false if this event is
 * not handled and should be passed to other handlers.
 *
 */
void AbilityImpl::DoKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    APP_LOGI("AbilityImpl::DoKeyUp called");
}

/**
 * @brief Called when a touch event is dispatched to this ability. The default implementation of this callback
 * does nothing and returns false.
 * @param touchEvent Indicates information about the touch event.
 *
 * @return Returns true if the event is handled; returns false otherwise.
 *
 */
void AbilityImpl::DoPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    APP_LOGI("AbilityImpl::DoPointerEvent called");
}

/**
 * @brief Send the result code and data to be returned by this Page ability to the caller.
 * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int, int, Want) method to
 * receive the result set in the current method. This method can be called only after the ability has been initialized.
 *
 * @param requestCode Indicates the request code.
 * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the result code
 * to identify an error.
 * @param resultData Indicates the data returned after the ability is destroyed. You can define the data returned. This
 * parameter can be null.
 */
void AbilityImpl::SendResult(int requestCode, int resultCode, const Want &resultData)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::SendResult ability_ is nullptr");
        return;
    }

    if (resultData.HasParameter(OHOS_RESULT_PERMISSION_KEY) && resultData.HasParameter(OHOS_RESULT_PERMISSIONS_LIST) &&
        resultData.HasParameter(OHOS_RESULT_CALLER_BUNDLERNAME)) {

        if (resultCode > 0) {
            std::vector<std::string> permissions = resultData.GetStringArrayParam(OHOS_RESULT_PERMISSIONS_LIST);
            std::vector<std::string> grantYes = resultData.GetStringArrayParam(OHOS_RESULT_PERMISSIONS_LIST_YES);
            std::vector<std::string> grantNo = resultData.GetStringArrayParam(OHOS_RESULT_PERMISSIONS_LIST_NO);
            std::vector<int> grantResult;
            int intOK = 0;
            for (size_t i = 0; i < permissions.size(); i++) {
                intOK = 0;
                for (size_t j = 0; j < grantYes.size(); j++) {
                    if (permissions[i] == grantYes[j]) {
                        intOK = 1;
                        break;
                    }
                }
                grantResult.push_back(intOK);
            }
            APP_LOGI("%{public}s begin OnRequestPermissionsFromUserResult.", __func__);
            ability_->OnRequestPermissionsFromUserResult(requestCode, permissions, grantResult);
            APP_LOGI("%{public}s end OnRequestPermissionsFromUserResult.", __func__);
        } else {
            APP_LOGI("%{public}s user cancel permissions.", __func__);
        }
    } else {
        ability_->OnAbilityResult(requestCode, resultCode, resultData);
    }
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch
 * an ability that has been at the top of the ability stack.
 *
 * @param want  Indicates the new Want containing information about the ability.
 */
void AbilityImpl::NewWant(const Want &want)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::NewWant ability_ is nullptr");
        return;
    }
    ability_->SetWant(want);
    ability_->OnNewWant(want);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Obtains the MIME types of files supported.
 *
 * @param uri Indicates the path of the files to obtain.
 * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
 *
 * @return Returns the matched MIME types. If there is no match, null is returned.
 */
std::vector<std::string> AbilityImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    APP_LOGI("AbilityImpl::GetFileTypes");
    std::vector<std::string> types;
    return types;
}

/**
 * @brief Opens a file in a specified remote path.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
 *  or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the file descriptor.
 */
int AbilityImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    APP_LOGI("AbilityImpl::OpenFile");
    return -1;
}

/**
 * @brief This is like openFile, open a file that need to be able to return sub-sections of files，often assets
 * inside of their .hap.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing
 * data, or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the RawFileDescriptor object containing file descriptor.
 */
int AbilityImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    APP_LOGI("AbilityImpl::OpenRawFile");
    return -1;
}

/**
 * @brief Inserts a single data record into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
 *
 * @return Returns the index of the inserted data record.
 */
int AbilityImpl::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    APP_LOGI("AbilityImpl::Insert");
    return -1;
}

/**
 * @brief Updates data records in the database.
 *
 * @param uri Indicates the path of data to update.
 * @param value Indicates the data to update. This parameter can be null.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the number of data records updated.
 */
int AbilityImpl::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("AbilityImpl::Update");
    return -1;
}

/**
 * @brief Deletes one or more data records from the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the number of data records deleted.
 */
int AbilityImpl::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("AbilityImpl::Delete");
    return -1;
}

/**
 * @brief Deletes one or more data records from the database.
 *
 * @param uri Indicates the path of data to query.
 * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the query result.
 */
std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilityImpl::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("AbilityImpl::Query");
    return nullptr;
}

/**
 * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
 * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
 *
 * @param uri Indicates the URI of the data.
 *
 * @return Returns the MIME type that matches the data specified by uri.
 */
std::string AbilityImpl::GetType(const Uri &uri)
{
    APP_LOGI("AbilityImpl::GetType");
    return "";
}

/**
 * @brief Reloads data in the database.
 *
 * @param uri Indicates the position where the data is to reload. This parameter is mandatory.
 * @param extras Indicates the PacMap object containing the additional parameters to be passed in this call. This
 * parameter can be null. If a custom Sequenceable object is put in the PacMap object and will be transferred across
 * processes, you must call BasePacMap.setClassLoader(ClassLoader) to set a class loader for the custom object.
 *
 * @return Returns true if the data is successfully reloaded; returns false otherwise.
 */
bool AbilityImpl::Reload(const Uri &uri, const PacMap &extras)
{
    return false;
}

/**
 * @brief Inserts multiple data records into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param values Indicates the data records to insert.
 *
 * @return Returns the number of data records inserted.
 */
int AbilityImpl::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    APP_LOGI("AbilityImpl::BatchInsert");
    return -1;
}

/**
 * @brief SerUriString
 */
void AbilityImpl::SerUriString(const std::string &uri)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (contextDeal_ == nullptr) {
        APP_LOGE("AbilityImpl::SerUriString contextDeal_ is nullptr");
        return;
    }
    contextDeal_->SerUriString(uri);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Set the LifeCycleStateInfo to the deal.
 *
 * @param info the info to set.
 */
void AbilityImpl::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    if (contextDeal_ == nullptr) {
        APP_LOGE("AbilityImpl::SetLifeCycleStateInfo contextDeal_ is nullptr");
        return;
    }
    contextDeal_->SetLifeCycleStateInfo(info);
}

/**
 * @brief Check if it needs to restore the data to the ability.
 *
 * @return Return true if need and success, otherwise return false.
 */
bool AbilityImpl::CheckAndRestore()
{
    APP_LOGI("AbilityImpl::CheckAndRestore called start");
    if (!hasSaveData_) {
        APP_LOGE("AbilityImpl::CheckAndRestore hasSaveData_ is false");
        return false;
    }

    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::CheckAndRestore ability_ is nullptr");
        return false;
    }

    APP_LOGI("AbilityImpl::CheckAndRestore ready to restore");
    ability_->OnRestoreAbilityState(restoreData_);

    APP_LOGI("AbilityImpl::CheckAndRestore called end");
    return true;
}

/**
 * @brief Check if it needs to save the data to the ability.
 *
 * @return Return true if need and success, otherwise return false.
 */
bool AbilityImpl::CheckAndSave()
{
    APP_LOGI("AbilityImpl::CheckAndSave called start");
    if (!needSaveDate_) {
        APP_LOGE("AbilityImpl::CheckAndSave needSaveDate_ is false");
        return false;
    }

    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::CheckAndSave ability_ is nullptr");
        return false;
    }

    APP_LOGI("AbilityImpl::CheckAndSave ready to save");
    ability_->OnSaveAbilityState(restoreData_);
    abilityLifecycleCallbacks_->OnAbilitySaveState(restoreData_);

    needSaveDate_ = false;

    APP_LOGI("AbilityImpl::CheckAndSave called end");
    return true;
}

PacMap &AbilityImpl::GetRestoreData()
{
    return restoreData_;
}

/**
 * @brief Set deviceId/bundleName/abilityName of the calling ability
 *
 * @param deviceId deviceId of the calling ability
 *
 * @param deviceId bundleName of the calling ability
 *
 * @param deviceId abilityName of the calling ability
 */
void AbilityImpl::SetCallingContext(
    const std::string &deviceId, const std::string &bundleName, const std::string &abilityName)
{
    if (ability_ != nullptr) {
        ability_->SetCallingContext(deviceId, bundleName, abilityName);
    }
}

/**
 * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
 * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
 * context has changed. If you implement URI normalization for a Data ability, you must also implement
 * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to any
 * method that is called on the Data ability must require normalization verification and denormalization. The default
 * implementation of this method returns null, indicating that this Data ability does not support URI normalization.
 *
 * @param uri Indicates the Uri object to normalize.
 *
 * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
 */
Uri AbilityImpl::NormalizeUri(const Uri &uri)
{
    APP_LOGI("AbilityImpl::NormalizeUri");
    return uri;
}

/**
 * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
 * The default implementation of this method returns the original URI passed to it.
 *
 * @param uri uri Indicates the Uri object to denormalize.
 *
 * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed to
 * this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found in the
 * current environment.
 */
Uri AbilityImpl::DenormalizeUri(const Uri &uri)
{
    APP_LOGI("AbilityImpl::DenormalizeUri");
    return uri;
}

/*
 * @brief ScheduleUpdateConfiguration, scheduling update configuration.
 */
void AbilityImpl::ScheduleUpdateConfiguration(const Configuration &config)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::ScheduleUpdateConfiguration ability_ is nullptr");
    }

    ability_->OnConfigurationUpdated(config);
    APP_LOGI("%{public}s end.", __func__);
}

void AbilityImpl::InputEventConsumerImpl::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    int32_t code = keyEvent->GetKeyAction();
    if (code == MMI::KeyEvent::KEY_ACTION_DOWN) {
        abilityImpl_->DoKeyDown(keyEvent);
        APP_LOGI("AbilityImpl::OnKeyDown keyAction: %{public}d.", code);
    } else if (code == MMI::KeyEvent::KEY_ACTION_UP) {
        abilityImpl_->DoKeyUp(keyEvent);
        APP_LOGI("AbilityImpl::DoKeyUp keyAction: %{public}d.", code);
    }
}

void AbilityImpl::InputEventConsumerImpl::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    APP_LOGI("AbilityImpl::DoPointerEvent called.");
    abilityImpl_->DoPointerEvent(pointerEvent);
}

/**
 * @brief Multimodal Events Register.
 */
void AbilityImpl::WindowEventRegister()
{
    APP_LOGI("%{public}s called.", __func__);
    if (ability_->GetCompatibleVersion() < TARGET_VERSION_THRESHOLDS) {
        auto window = ability_->GetWindow();
        if (window) {
            std::shared_ptr<MMI::IInputEventConsumer> inputEventListener =
                std::make_shared<AbilityImpl::InputEventConsumerImpl>(shared_from_this());
            window->AddInputEventListener(inputEventListener);
        }
    }
}

/**
 * @brief Create a PostEvent timeout task. The default delay is 5000ms
 *
 * @return Return a smart pointer to a timeout object
 */
std::shared_ptr<AbilityPostEventTimeout> AbilityImpl::CreatePostEventTimeouter(std::string taskstr)
{
    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::CreatePostEventTimeouter ability_ is nullptr");
        return nullptr;
    }

    return ability_->CreatePostEventTimeouter(taskstr);
}

std::vector<std::shared_ptr<DataAbilityResult>> AbilityImpl::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    APP_LOGI("AbilityImpl::ExecuteBatch");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    return results;
}

void AbilityImpl::NotifyContinuationResult(const int32_t result)
{
    if (ability_ == nullptr) {
        APP_LOGE("AbilityImpl::NotifyContinuationResult ability_ is nullptr");
        return;
    }
    ability_->OnCompleteContinuation(result);
}
}  // namespace AppExecFwk
}  // namespace OHOS
