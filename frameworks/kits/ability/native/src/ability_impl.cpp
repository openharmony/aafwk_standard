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

#include "ability_impl.h"

#include "bytrace.h"
#include "data_ability_predicates.h"
#include "hilog_wrapper.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
static bool g_useNewMission = false;
static bool g_isMissionFlagSetted = false;
const std::string PERMISSION_KEY = "ohos.user.grant.permission";
const std::string GRANTED_RESULT_KEY = "ohos.user.grant.permission.result";

void AbilityImpl::Init(std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &record,
    std::shared_ptr<Ability> &ability, std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token,
    std::shared_ptr<ContextDeal> &contextDeal)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityImpl::init begin");
    if ((token == nullptr) || (application == nullptr) || (handler == nullptr) || (record == nullptr) ||
        ability == nullptr || contextDeal == nullptr) {
        HILOG_ERROR("AbilityImpl::init failed, token is nullptr, application is nullptr, handler is nullptr, record is "
                 "nullptr, ability is nullptr, contextDeal is nullptr");
        return;
    }

    token_ = record->GetToken();
    record->SetAbilityImpl(shared_from_this());
    ability_ = ability;
    handler_ = handler;
    auto info = record->GetAbilityInfo();
    isStageBasedModel_ = info && info->isStageBasedModel;
#ifdef SUPPORT_GRAPHICS
    if (info && info->type == AbilityType::PAGE) {
        ability_->SetSceneListener(
            sptr<WindowLifeCycleImpl>(new (std::nothrow) WindowLifeCycleImpl(token_, shared_from_this())));
    }
#endif
    ability_->Init(record->GetAbilityInfo(), application, handler, token);
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    abilityLifecycleCallbacks_ = application;
    contextDeal_ = contextDeal;
    HILOG_INFO("AbilityImpl::init end");
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 * @param want  The Want object to switch the life cycle.
 */
void AbilityImpl::Start(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Start ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    if ((ability_->GetAbilityInfo()->type == AbilityType::PAGE) &&
        (!ability_->GetAbilityInfo()->isStageBasedModel)) {
        ability_->HandleCreateAsContinuation(want);
    }
#endif
    HILOG_INFO("AbilityImpl::Start");
    ability_->OnStart(want);
#ifdef SUPPORT_GRAPHICS
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_STARTED_NEW;
    } else {
#endif
        if (ability_->GetAbilityInfo()->type == AbilityType::DATA) {
            lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
        } else {
            lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
        }
#ifdef SUPPORT_GRAPHICS
    }
#endif

    abilityLifecycleCallbacks_->OnAbilityStart(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INITIAL. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 */
void AbilityImpl::Stop()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Stop ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    ability_->OnStop();
#ifdef SUPPORT_GRAPHICS
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_STOPED_NEW;
    } else {
#endif
        lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
#ifdef SUPPORT_GRAPHICS
    }
#endif
    abilityLifecycleCallbacks_->OnAbilityStop(ability_);
    ability_->DestroyInstance(); // Release window and ability.
    ability_ = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_ACTIVE. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 */
void AbilityImpl::Active()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Active ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    ability_->OnActive();
#ifdef SUPPORT_GRAPHICS
    if ((lifecycleState_ == AAFwk::ABILITY_STATE_INACTIVE) && (ability_->GetAbilityInfo()->type == AbilityType::PAGE)) {
        ability_->OnTopActiveAbilityChanged(true);
        ability_->OnWindowFocusChanged(true);
    }
#endif
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityLifecycleCallbacks_->OnAbilityActive(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 */
void AbilityImpl::Inactive()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Inactive ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    ability_->OnInactive();
#ifdef SUPPORT_GRAPHICS
    if ((lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE) && (ability_->GetAbilityInfo()->type == AbilityType::PAGE)) {
        ability_->OnTopActiveAbilityChanged(false);
        ability_->OnWindowFocusChanged(false);
    }
#endif
    lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    abilityLifecycleCallbacks_->OnAbilityInactive(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

bool AbilityImpl::IsStageBasedModel() const
{
    return isStageBasedModel_;
}

int AbilityImpl::GetCompatibleVersion()
{
    if (ability_) {
        return ability_->GetCompatibleVersion();
    }

    return -1;
}

#ifdef SUPPORT_GRAPHICS
void AbilityImpl::AfterUnFocused()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (!ability_ || !ability_->GetAbilityInfo() || !contextDeal_ || !handler_) {
        HILOG_ERROR("AbilityImpl::AfterUnFocused failed");
        return;
    }

    if (ability_->GetAbilityInfo()->isStageBasedModel) {
        HILOG_INFO("new version ability, do nothing when after unfocused.");
        return;
    }

    HILOG_INFO("old version ability, window after unfocused.");
    auto task = [abilityImpl = shared_from_this(), ability = ability_, contextDeal = contextDeal_]() {
        auto info = contextDeal->GetLifeCycleStateInfo();
        info.state = AbilityLifeCycleState::ABILITY_STATE_INACTIVE;
        info.isNewWant = false;
        Want want(*(ability->GetWant()));
        abilityImpl->HandleAbilityTransaction(want, info);
    };
    handler_->PostTask(task);
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::AfterFocused()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (!ability_ || !ability_->GetAbilityInfo() || !contextDeal_ || !handler_) {
        HILOG_ERROR("AbilityImpl::AfterFocused failed");
        return;
    }

    if (ability_->GetAbilityInfo()->isStageBasedModel) {
        HILOG_INFO("new version ability, do nothing when after focused.");
        return;
    }

    HILOG_INFO("fa mode ability, window after focused.");
    auto task = [abilityImpl = shared_from_this(), ability = ability_, contextDeal = contextDeal_]() {
        auto info = contextDeal->GetLifeCycleStateInfo();
        info.state = AbilityLifeCycleState::ABILITY_STATE_ACTIVE;
        info.isNewWant = false;
        Want want(*(ability->GetWant()));
        abilityImpl->HandleAbilityTransaction(want, info);
    };
    handler_->PostTask(task);
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::WindowLifeCycleImpl::AfterForeground()
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner && !owner->IsStageBasedModel()) {
        return;
    }

    HILOG_INFO("new version ability, window after foreground.");
    PacMap restoreData;
    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
        AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW, restoreData);
}

void AbilityImpl::WindowLifeCycleImpl::AfterBackground()
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner && !owner->IsStageBasedModel()) {
        return;
    }

    HILOG_INFO("new version ability, window after background.");
    PacMap restoreData;
    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
        AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_NEW, restoreData);
}

void AbilityImpl::WindowLifeCycleImpl::AfterFocused()
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterFocused();
    }
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::WindowLifeCycleImpl::AfterUnfocused()
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterUnFocused();
    }
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 * @param want The Want object to switch the life cycle.
 */
void AbilityImpl::Foreground(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Foreground ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    HILOG_INFO("AbilityImpl::Foreground");
    ability_->OnForeground(want);
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    } else {
        lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    }
    abilityLifecycleCallbacks_->OnAbilityForeground(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_BACKGROUND. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 */
void AbilityImpl::Background()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Background ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    HILOG_INFO("AbilityImpl::Background");
    ability_->OnLeaveForeground();
    ability_->OnBackground();
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    } else {
        lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND;
    }
    abilityLifecycleCallbacks_->OnAbilityBackground(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}
#endif

/**
 * @brief Save data and states of an ability when it is restored by the system. and Calling information back to Ability.
 *        This method should be implemented by a Page ability.
 *
 */
void AbilityImpl::DispatchSaveAbilityState()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::DispatchSaveAbilityState ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    HILOG_INFO("AbilityImpl::DispatchSaveAbilityState");
    needSaveDate_ = true;
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Restores data and states of an ability when it is restored by the system. and Calling information back to
 * Ability. This method should be implemented by a Page ability.
 * @param instate The Want object to connect to.
 *
 */
void AbilityImpl::DispatchRestoreAbilityState(const PacMap &inState)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::DispatchRestoreAbilityState ability_ is nullptr");
        return;
    }

    hasSaveData_ = true;
    restoreData_ = inState;
    HILOG_INFO("%{public}s end.", __func__);
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
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::ConnectAbility ability_ is nullptr");
        return nullptr;
    }

    HILOG_INFO("AbilityImpl:: ConnectAbility");
    sptr<IRemoteObject> object = ability_->OnConnect(want);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityLifecycleCallbacks_->OnAbilityActive(ability_);
    HILOG_INFO("%{public}s end.", __func__);

    return object;
}

/**
 * @brief Disconnects the connected object.
 *
 * @param want The Want object to disconnect to.
 */
void AbilityImpl::DisconnectAbility(const Want &want)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("Disconnect ability error, ability_ is nullptr.");
        return;
    }
    HILOG_INFO("Disconnect ability begin, ability:%{public}s.", ability_->GetAbilityName().c_str());
    ability_->OnDisconnect(want);
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
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::CommandAbility ability_ is nullptr");
        return;
    }

    HILOG_INFO("AbilityImpl:: CommandAbility");
    ability_->OnCommand(want, restart, startId);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityLifecycleCallbacks_->OnAbilityActive(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Gets the current Ability status.
 *
 */
int AbilityImpl::GetCurrentState()
{
    return lifecycleState_;
}

#ifdef SUPPORT_GRAPHICS
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
    HILOG_INFO("AbilityImpl::DoKeyDown called");
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
    HILOG_INFO("AbilityImpl::DoKeyUp called");
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
    HILOG_INFO("AbilityImpl::DoPointerEvent called");
}
#endif

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
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::SendResult ability_ is nullptr");
        return;
    }

    if (resultData.HasParameter(PERMISSION_KEY)) {
        std::vector<std::string> permissions = resultData.GetStringArrayParam(PERMISSION_KEY);
        std::vector<int> grantedResult(permissions.size(), -1);
        if (resultCode > 0) {
            grantedResult = resultData.GetIntArrayParam(GRANTED_RESULT_KEY);
            HILOG_INFO("%{public}s Get user granted result.", __func__);
        }
        ability_->OnRequestPermissionsFromUserResult(requestCode, permissions, grantedResult);
    } else {
        ability_->OnAbilityResult(requestCode, resultCode, resultData);
    }

    // for api5 FeatureAbility::startAbilityForResult
    ability_->OnFeatureAbilityResult(requestCode, resultCode, resultData);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch
 * an ability that has been at the top of the ability stack.
 *
 * @param want  Indicates the new Want containing information about the ability.
 */
void AbilityImpl::NewWant(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::NewWant ability_ is nullptr");
        return;
    }
    ability_->SetWant(want);
    ability_->OnNewWant(want);
    HILOG_INFO("%{public}s end.", __func__);
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
    HILOG_INFO("AbilityImpl::GetFileTypes");
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
    HILOG_INFO("AbilityImpl::OpenFile");
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
    HILOG_INFO("AbilityImpl::OpenRawFile");
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
    HILOG_INFO("AbilityImpl::Insert");
    return -1;
}

std::shared_ptr<AppExecFwk::PacMap> AbilityImpl::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    HILOG_INFO("AbilityImpl::Call");
    return nullptr;
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
    HILOG_INFO("AbilityImpl::Update");
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
    HILOG_INFO("AbilityImpl::Delete");
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
    HILOG_INFO("AbilityImpl::Query");
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
    HILOG_INFO("AbilityImpl::GetType");
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
    HILOG_INFO("AbilityImpl::BatchInsert");
    return -1;
}

/**
 * @brief SerUriString
 */
void AbilityImpl::SerUriString(const std::string &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (contextDeal_ == nullptr) {
        HILOG_ERROR("AbilityImpl::SerUriString contextDeal_ is nullptr");
        return;
    }
    contextDeal_->SerUriString(uri);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Set the LifeCycleStateInfo to the deal.
 *
 * @param info the info to set.
 */
void AbilityImpl::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    if (contextDeal_ == nullptr) {
        HILOG_ERROR("AbilityImpl::SetLifeCycleStateInfo contextDeal_ is nullptr");
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
    HILOG_INFO("AbilityImpl::CheckAndRestore called start");
    if (!hasSaveData_) {
        HILOG_ERROR("AbilityImpl::CheckAndRestore hasSaveData_ is false");
        return false;
    }

    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::CheckAndRestore ability_ is nullptr");
        return false;
    }

    HILOG_INFO("AbilityImpl::CheckAndRestore ready to restore");
    ability_->OnRestoreAbilityState(restoreData_);

    HILOG_INFO("AbilityImpl::CheckAndRestore called end");
    return true;
}

/**
 * @brief Check if it needs to save the data to the ability.
 *
 * @return Return true if need and success, otherwise return false.
 */
bool AbilityImpl::CheckAndSave()
{
    HILOG_INFO("AbilityImpl::CheckAndSave called start");
    if (!needSaveDate_) {
        HILOG_ERROR("AbilityImpl::CheckAndSave needSaveDate_ is false");
        return false;
    }

    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::CheckAndSave ability_ is nullptr");
        return false;
    }

    HILOG_INFO("AbilityImpl::CheckAndSave ready to save");
    ability_->OnSaveAbilityState(restoreData_);
    abilityLifecycleCallbacks_->OnAbilitySaveState(restoreData_);

    needSaveDate_ = false;

    HILOG_INFO("AbilityImpl::CheckAndSave called end");
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
    HILOG_INFO("AbilityImpl::NormalizeUri");
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
    HILOG_INFO("AbilityImpl::DenormalizeUri");
    return uri;
}

/*
 * @brief ScheduleUpdateConfiguration, scheduling update configuration.
 */
void AbilityImpl::ScheduleUpdateConfiguration(const Configuration &config)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::ScheduleUpdateConfiguration ability_ is nullptr");
        return;
    }

    if (lifecycleState_ != AAFwk::ABILITY_STATE_INITIAL) {
        HILOG_INFO("ability name: [%{public}s]", ability_->GetAbilityName().c_str());
        ability_->OnConfigurationUpdatedNotify(config);
    }

    HILOG_INFO("%{public}s end.", __func__);
}

#ifdef SUPPORT_GRAPHICS
void AbilityImpl::InputEventConsumerImpl::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    int32_t code = keyEvent->GetKeyAction();
    if (code == MMI::KeyEvent::KEY_ACTION_DOWN) {
        abilityImpl_->DoKeyDown(keyEvent);
        HILOG_INFO("AbilityImpl::OnKeyDown keyAction: %{public}d.", code);
    } else if (code == MMI::KeyEvent::KEY_ACTION_UP) {
        abilityImpl_->DoKeyUp(keyEvent);
        HILOG_INFO("AbilityImpl::DoKeyUp keyAction: %{public}d.", code);
    }
}

void AbilityImpl::InputEventConsumerImpl::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    HILOG_INFO("AbilityImpl::DoPointerEvent called.");
    abilityImpl_->DoPointerEvent(pointerEvent);
}
#endif

/**
 * @brief Create a PostEvent timeout task. The default delay is 5000ms
 *
 * @return Return a smart pointer to a timeout object
 */
std::shared_ptr<AbilityPostEventTimeout> AbilityImpl::CreatePostEventTimeouter(std::string taskstr)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::CreatePostEventTimeouter ability_ is nullptr");
        return nullptr;
    }

    return ability_->CreatePostEventTimeouter(taskstr);
}

std::vector<std::shared_ptr<DataAbilityResult>> AbilityImpl::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    HILOG_INFO("AbilityImpl::ExecuteBatch");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    return results;
}

void AbilityImpl::ContinueAbility(const std::string& deviceId)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::ContinueAbility ability_ is nullptr");
        return;
    }
    ability_->ContinueAbilityWithStack(deviceId);
}

void AbilityImpl::NotifyContinuationResult(int32_t result)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::NotifyContinuationResult ability_ is nullptr");
        return;
    }
    ability_->OnCompleteContinuation(result);
}

void AbilityImpl::SetUseNewMission(bool useNewMission)
{
    if (!g_isMissionFlagSetted) {
        g_isMissionFlagSetted = true;
        g_useNewMission = useNewMission;
    }
}

bool AbilityImpl::IsUseNewMission()
{
    return g_useNewMission;
}
}  // namespace AppExecFwk
}  // namespace OHOS
