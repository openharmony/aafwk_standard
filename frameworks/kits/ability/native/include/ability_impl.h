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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_IMPL_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_IMPL_H

#include "ability.h"
#include "ability_state.h"
#include "iability_lifecycle_callback.h"
#include "context.h"
#include "application_impl.h"
#include "ability_local_record.h"
#include "ability_handler.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "dummy_component_container.h"
#include "dummy_values_bucket.h"
#include "dummy_data_ability_predicates.h"
#include "dummy_result_set.h"
#ifdef SUPPORT_GRAPHICS
#include "foundation/multimodalinput/input/interfaces/native/innerkits/event/include/i_input_event_consumer.h"
#endif
namespace OHOS {
namespace AppExecFwk {
class Ability;
class AbilityHandler;
class ApplicationImpl;
class AbilityLocalRecord;
class AbilityLifecycleCallbacks;
class OHOSApplication;
class AbilityImpl : public std::enable_shared_from_this<AbilityImpl> {
public:
    AbilityImpl() = default;
    virtual ~AbilityImpl() = default;
    virtual void Init(std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &record,
        std::shared_ptr<Ability> &ability, std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token,
        std::shared_ptr<ContextDeal> &contextDeal);

    /**
     * @brief Set if use new mission.
     *
     * @param useNewMission new mission flag.
     */
    static void SetUseNewMission(bool useNewMission);

    /**
     * @brief Get if use new mission.
     *
     * @return return true if use new mission.
     */
    static bool IsUseNewMission();

    /**
     * @brief Connect the ability. and Calling information back to Ability.
     *
     * @param want The Want object to connect to.
     *
     */
    sptr<IRemoteObject> ConnectAbility(const Want &want);

    /**
     * @brief Disconnects the connected object.
     *
     * @param want The Want object to disconnect to.
     */
    void DisconnectAbility(const Want &want);

    /**
     * @brief Command the ability. and Calling information back to Ability.
     *
     * @param want The Want object to command to.
     *
     * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     *
     * @param startId Indicates the number of times the Service ability has been started. The startId is incremented
     * by 1 every time the ability is started. For example, if the ability has been started for six times, the value
     * of startId is 6.
     */
    void CommandAbility(const Want &want, bool restart, int startId);

    /**
     * @brief Gets the current Ability status.
     *
     */
    int GetCurrentState();

    /**
     * @brief Save data and states of an ability when it is restored by the system. and Calling information back to
     * Ability. This method should be implemented by a Page ability.
     *
     */
    void DispatchSaveAbilityState();

    /**
     * @brief Restores data and states of an ability when it is restored by the system. and Calling information back
     * to Ability. This method should be implemented by a Page ability.
     * @param instate The Want object to connect to.
     *
     */
    void DispatchRestoreAbilityState(const PacMap &inState);

    // Page Service Ability has different AbilityTransaction
    virtual void HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState);

#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Execution the KeyDown callback of the ability
     * @param keyEvent Indicates the key-down event.
     *
     * @return Returns true if this event is handled and will not be passed further; returns false if this event is
     * not handled and should be passed to other handlers.
     *
     */
    virtual void DoKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent);

    /**
     * @brief Execution the KeyUp callback of the ability
     * @param keyEvent Indicates the key-up event.
     *
     * @return Returns true if this event is handled and will not be passed further; returns false if this event is
     * not handled and should be passed to other handlers.
     *
     */
    virtual void DoKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent);

    /**
     * @brief Called when a touch event is dispatched to this ability. The default implementation of this callback
     * does nothing and returns false.
     * @param touchEvent Indicates information about the touch event.
     *
     * @return Returns true if the event is handled; returns false otherwise.
     *
     */
    virtual void DoPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent);
#endif

    /**
     * @brief Send the result code and data to be returned by this Page ability to the caller.
     * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int, int, Want)
     * method to receive the result set in the current method. This method can be called only after the ability has
     * been initialized.
     *
     * @param requestCode Indicates the request code.
     * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the
     * result code to identify an error.
     * @param resultData Indicates the data returned after the ability is destroyed. You can define the data
     * returned. This parameter can be null.
     */
    void SendResult(int requestCode, int resultCode, const Want &resultData);

    /**
     * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch
     * an ability that has been at the top of the ability stack.
     *
     * @param want  Indicates the new Want containing information about the ability.
     */
    void NewWant(const Want &want);

    /**
     * @brief Obtains the MIME types of files supported.
     *
     * @param uri Indicates the path of the files to obtain.
     * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
     *
     * @return Returns the matched MIME types. If there is no match, null is returned.
     */
    virtual std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter);

    /**
     * @brief Opens a file in a specified remote path.
     *
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
     * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
     * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing
     * data, or "rwt" for read and write access that truncates any existing file.
     *
     * @return Returns the file descriptor.
     */
    virtual int OpenFile(const Uri &uri, const std::string &mode);

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
    virtual int OpenRawFile(const Uri &uri, const std::string &mode);

    /**
     * @brief Inserts a single data record into the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
     *
     * @return Returns the index of the inserted data record.
     */
    virtual int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value);

    virtual std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap);

    /**
     * @brief Updates data records in the database.
     *
     * @param uri Indicates the path of data to update.
     * @param value Indicates the data to update. This parameter can be null.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is
     * null.
     *
     * @return Returns the number of data records updated.
     */
    virtual int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Deletes one or more data records from the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is
     * null.
     *
     * @return Returns the number of data records deleted.
     */
    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Deletes one or more data records from the database.
     *
     * @param uri Indicates the path of data to query.
     * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is
     * null.
     *
     * @return Returns the query result.
     */
    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should
     * be implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
     *
     * @param uri Indicates the URI of the data.
     *
     * @return Returns the MIME type that matches the data specified by uri.
     */
    virtual std::string GetType(const Uri &uri);

    /**
     * @brief Reloads data in the database.
     *
     * @param uri Indicates the position where the data is to reload. This parameter is mandatory.
     * @param extras Indicates the PacMap object containing the additional parameters to be passed in this call.
     * This parameter can be null. If a custom Sequenceable object is put in the PacMap object and will be
     * transferred across processes, you must call BasePacMap.setClassLoader(ClassLoader) to set a class loader for
     * the custom object.
     *
     * @return Returns true if the data is successfully reloaded; returns false otherwise.
     */
    virtual bool Reload(const Uri &uri, const PacMap &extras);

    /**
     * @brief Inserts multiple data records into the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param values Indicates the data records to insert.
     *
     * @return Returns the number of data records inserted.
     */
    virtual int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values);

    /**
     * @brief Set deviceId/bundleName/abilityName of the calling ability
     *
     * @param deviceId deviceId of the calling ability
     *
     * @param deviceId bundleName of the calling ability
     *
     * @param deviceId abilityName of the calling ability
     */
    void SetCallingContext(const std::string &deviceId, const std::string &bundleName, const std::string &abilityName);

    /**
     * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
     * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
     * context has changed. If you implement URI normalization for a Data ability, you must also implement
     * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to
     * any method that is called on the Data ability must require normalization verification and denormalization. The
     * default implementation of this method returns null, indicating that this Data ability does not support URI
     * normalization.
     *
     * @param uri Indicates the Uri object to normalize.
     *
     * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
     */
    virtual Uri NormalizeUri(const Uri &uri);

    /**
     * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
     * The default implementation of this method returns the original URI passed to it.
     *
     * @param uri uri Indicates the Uri object to denormalize.
     *
     * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed
     * to this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found
     * in the current environment.
     */
    virtual Uri DenormalizeUri(const Uri &uri);

    /**
     * @brief ScheduleUpdateConfiguration, scheduling update configuration.
     */
    void ScheduleUpdateConfiguration(const Configuration &config);

    /**
     * @brief Create a PostEvent timeout task. The default delay is 5000ms
     *
     * @return Return a smart pointer to a timeout object
     */
    std::shared_ptr<AbilityPostEventTimeout> CreatePostEventTimeouter(std::string taskstr);

    virtual std::vector<std::shared_ptr<DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<DataAbilityOperation>> &operations);

    /**
     * @brief continue ability to target device.
     *
     * @param deviceId: target deviceId
     *
     * @return
     */
    void ContinueAbility(const std::string& deviceId);

    /**
     * @brief Notify continuation result to ability.
     *
     * @param result Continuaton result.
     *
     * @return
     */
    virtual void NotifyContinuationResult(int32_t result);

    bool IsStageBasedModel() const;

    int GetCompatibleVersion();
#ifdef SUPPORT_GRAPHICS
    void AfterUnFocused();
    void AfterFocused();
#endif
protected:
    /**
     * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
     * that it belongs to of the lifecycle status.
     *
     * @param want  The Want object to switch the life cycle.
     */
    void Start(const Want &want);

    /**
     * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INITIAL. And notifies the application
     * that it belongs to of the lifecycle status.
     *
     */
    void Stop();

    /**
     * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_ACTIVE. And notifies the application
     * that it belongs to of the lifecycle status.
     *
     */
    void Active();

    /**
     * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
     * that it belongs to of the lifecycle status.
     *
     */
    void Inactive();

#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
     * that it belongs to of the lifecycle status.
     *
     * @param want The Want object to switch the life cycle.
     */
    void Foreground(const Want &want);

    /**
     * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_BACKGROUND. And notifies the
     * application that it belongs to of the lifecycle status.
     *
     */
    void Background();
#endif

    /**
     * @brief SerUriString
     */
    void SerUriString(const std::string &uri);

    /**
     * @brief Set the LifeCycleStateInfo to the deal.
     *
     * @param info the info to set.
     */
    void SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info);

    /**
     * @brief Check if it needs to restore the data to the ability.
     *
     * @return Return true if success, otherwise return false.
     */
    bool CheckAndRestore();

    /**
     * @brief Check if it needs to save the data to the ability.
     *
     * @return Return true if success, otherwise return false.
     */
    bool CheckAndSave();

    PacMap &GetRestoreData();

    bool isStageBasedModel_ = false;
    int lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    sptr<IRemoteObject> token_;
    std::shared_ptr<Ability> ability_;
    std::shared_ptr<AbilityHandler> handler_;

private:
#ifdef SUPPORT_GRAPHICS
class WindowLifeCycleImpl : public Rosen::IWindowLifeCycle {
public:
    WindowLifeCycleImpl(const sptr<IRemoteObject>& token, const std::shared_ptr<AbilityImpl>& owner)
        : token_(token) , owner_(owner) {}
    virtual ~WindowLifeCycleImpl() {}
    void AfterForeground() override;
    void AfterBackground() override;
    void AfterFocused() override;
    void AfterUnfocused() override;
private:
    sptr<IRemoteObject> token_ = nullptr;
    std::weak_ptr<AbilityImpl> owner_;
};

class InputEventConsumerImpl : public MMI::IInputEventConsumer {
public:
    explicit InputEventConsumerImpl(const std::shared_ptr<AbilityImpl>& abilityImpl) : abilityImpl_(abilityImpl) {}
    ~InputEventConsumerImpl() = default;
    void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override;
    void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
    void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const override {}
private:
    std::shared_ptr<AbilityImpl> abilityImpl_;
};
#endif
    typedef enum {
        START,
        INACTIVE,
        ACTIVE,
        BACKGROUND,
        FOREGROUND,
        STOP,
    } Action;

    std::shared_ptr<AbilityLifecycleCallbacks> abilityLifecycleCallbacks_;
    std::shared_ptr<ApplicationImpl> applactionImpl_;
    std::shared_ptr<ContextDeal> contextDeal_;

private:
#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Multimodal Events Register.
     */
    void WindowEventRegister();
#endif
    bool hasSaveData_ = false;
    bool needSaveDate_ = false;
    PacMap restoreData_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_IMPL_H
