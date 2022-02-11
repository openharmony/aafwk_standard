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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_H

#include <functional>
#include <string>
#include <unistd.h>

#include "ability_context.h"
#include "ability_continuation_interface.h"
#include "ability_event_interface.h"
#include "ability_lifecycle_executor.h"
#include "ability_lifecycle_interface.h"
#include "ability_window.h"
#include "configuration.h"
#include "context.h"
#include "continuation_handler.h"
#include "continuation_state.h"
#include "dummy_ability_package.h"
#include "dummy_component_container.h"
#include "dummy_notification_request.h"
#include "form_callback_interface.h"
#include "form_constants.h"
#include "form_death_callback.h"
#include "form_info.h"
#include "form_provider_info.h"
#include "foundation/multimodalinput/input/interfaces/native/innerkits/event/include/key_event.h"
#include "foundation/multimodalinput/input/interfaces/native/innerkits/event/include/pointer_event.h"
#include "iability_callback.h"
#include "iremote_object.h"
#include "pac_map.h"
#include "want.h"
#include "window_option.h"
#include "window_scene.h"
#include "wm_common.h"
#include "../../ability_runtime/include/ability_context.h"

using Uri = OHOS::Uri;

namespace OHOS {
namespace NativeRdb {
class AbsSharedResultSet;
class DataAbilityPredicates;
class ValuesBucket;
}  // namespace NativeRdb
namespace AbilityRuntime {
class Runtime;
}
class KeyEvent;
namespace AppExecFwk {
class DataAbilityResult;
class DataAbilityOperation;
class AbilityPostEventTimeout;
class OHOSApplication;
class AbilityHandler;
class AbilityWindow;
class ILifeCycle;
class ContinuationManager;
class ContinuationRegisterManager;
class IContinuationRegisterManager;
class Ability : public IAbilityEvent,
                public ILifeCycle,
                public AbilityContext,
                public FormCallbackInterface,
                public IAbilityContinuation,
                public IAbilityCallback,
                public std::enable_shared_from_this<Ability> {
public:
    friend class PageAbilityImpl;
    friend class NewAbilityImpl;

    static Ability* Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime);

    Ability() = default;
    virtual ~Ability() = default;

    /**
     * @brief Obtains the AbilityContext object of the ability.
     *
     * @return Returns the AbilityContext object of the ability.
     */
    inline std::shared_ptr<AbilityRuntime::AbilityContext> GetAbilityContext()
    {
        return abilityContext_;
    }

    /**
     * @brief Destroys ability.
     *
     * @param want Indicates the want containing information about TerminateAbility
     *
     * @return Returns the result of TerminateAbility
     */
    using AbilityContext::TerminateAbility;
    int TerminateAbility(Want &want);

    /**
     * @brief By binding an action, you can set different action parameters in want to present different initial
     * pages. You must register actions in the profile file.
     *
     * @param action Indicates the action to bind.
     *
     * @param entry Indicates the entry, which is the fully qualified name of your AbilitySlice class.
     *
     * @return Returns the result of AddActionRoute
     */
    virtual void AddActionRoute(const std::string &action, const std::string &entry) final;

    /**
     * @brief Sets the background color of the window in RGB color mode.
     *
     * @param red The value ranges from 0 to 255.
     *
     * @param green The value ranges from 0 to 255.
     *
     * @param blue The value ranges from 0 to 255.
     *
     * @return Returns the result of SetWindowBackgroundColor
     */
    virtual int SetWindowBackgroundColor(int red, int green, int blue) final;

    /**
     * @brief Destroys this Page or Service ability.
     * After a Page or Service ability performs all operations, it can use this method to destroy itself
     * to free up memory. This method can be called only after the ability is initialized.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode TerminateAbility() final;

    /**
     * @brief Obtains the Lifecycle object of the current ability.
     *
     * @return Returns the Lifecycle object.
     */
    virtual std::shared_ptr<LifeCycle> GetLifecycle() override final;

    /**
     * @brief Obtains a resource manager.
     *
     * @return Returns a ResourceManager object.
     */
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;

    /**
     * @brief Inflates UI controls by using ComponentContainer.
     * You can create a ComponentContainer instance that contains multiple components.
     *
     * @param componentContainer Indicates a set of customized components.
     */
    virtual void SetUIContent(const ComponentContainer &componentContainer);

    /**
     * @brief Inflates layout resources by using the layout resource ID.
     *
     * @param layoutRes Indicates the layout resource ID, which cannot be a negative number.
     */
    virtual void SetUIContent(int layoutRes) final;

    /**
     * Start other ability for result.
     *
     * @param want information of other ability
     * @param requestCode request code for abilityMS to return result
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbilityForResult(const Want &want, int requestCode) final;

    /**
     * Starts an ability with specific start settings and returns the execution result when the ability is destroyed.
     * When the ability is destroyed, onAbilityResult(int,int,ohos.aafwk.content.Want) is called and the returned
     * requestCode is transferred to the current method. The given requestCode is customized and cannot be a negative
     * number.
     *
     * @param want Indicates the ability to start.
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param abilityStartSetting Indicates the setting ability used to start.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbilityForResult(
        const Want &want, int requestCode, AbilityStartSetting abilityStartSetting) final;

    /**
     * Starts a new ability with specific start settings.
     * A Page or Service ability uses this method to start a specific ability.
     * The system locates the target ability from installed abilities based on
     * the value of the want parameter and then starts it. You can specify the
     * ability to start using the want parameter.
     *
     * @param want Indicates the ability to start.
     * @param abilityStartSetting Indicates the setting ability used to start.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(const Want &want, AbilityStartSetting abilityStartSetting);

    // lifecycle callback
    virtual void Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token);

    void AttachAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext);

    /**
     * @brief Called when this ability is started. You must override this function if you want to perform some
     *        initialization operations during ability startup.
     *
     * This function can be called only once in the entire lifecycle of an ability.
     * @param Want Indicates the {@link Want} structure containing startup information about the ability.
     */
    virtual void OnStart(const Want &want);

    /**
     * @brief Called when this ability enters the <b>STATE_STOP</b> state.
     *
     * The ability in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnStop();

    /**
     * @brief Called when this ability enters the <b>STATE_ACTIVE</b> state.
     *
     * The ability in the <b>STATE_ACTIVE</b> state is visible and has focus.
     * You can override this function to implement your own processing logic.
     *
     * @param Want Indicates the {@link Want} structure containing activation information about the ability.
     */
    virtual void OnActive();

    /**
     * @brief Called when this ability enters the <b>STATE_INACTIVE</b> state.
     *
     * <b>STATE_INACTIVE</b> is an instantaneous state. The ability in this state may be visible but does not have
     * focus.You can override this function to implement your own processing logic.
     */
    virtual void OnInactive();

    /**
     * @brief Called after instantiating WindowScene.
     *
     *
     * You can override this function to implement your own processing logic.
     */
    virtual void OnSceneCreated();

    /**
     * @brief Called after ability stoped.
     *
     *
     * You can override this function to implement your own processing logic.
     */
    virtual void onSceneDestroyed();

    /**
     * @brief Called after ability restored.
     *
     *
     * You can override this function to implement your own processing logic.
     */
    virtual void OnSceneRestored();

    /**
     * @brief Called when this ability enters the <b>STATE_FOREGROUND</b> state.
     *
     *
     * The ability in the <b>STATE_FOREGROUND</b> state is visible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnForeground(const Want &want);

    /**
     * @brief Called when this ability enters the <b>STATE_BACKGROUND</b> state.
     *
     *
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnBackground();

    /**
     * @brief Called when a key is pressed. When any component in the Ability gains focus, the key-down event for
     * the component will be handled first. This callback will not be invoked if the callback triggered for the
     * key-down event of the component returns true. The default implementation of this callback does nothing
     * and returns false.
     *
     * @param keyEvent Indicates the key-down event.
     *
     * @return Returns true if this event is handled and will not be passed further; returns false if this event
     * is not handled and should be passed to other handlers.
     */
    virtual void OnKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent);

    /**
     * @brief Called when a key is released. When any component in the Ability gains focus, the key-up event for
     * the component will be handled first. This callback will not be invoked if the callback triggered for the
     * key-up event of the component returns true. The default implementation of this callback does nothing and
     * returns false.
     *
     * @param keyEvent Indicates the key-up event.
     *
     * @return Returns true if this event is handled and will not be passed further; returns false if this event
     * is not handled and should be passed to other handlers.
     */
    virtual void OnKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent);

    /**
     * @brief Called when a touch event is dispatched to this ability. The default implementation of this callback
     * does nothing and returns false.
     *
     * @param event  Indicates information about the touch event.
     *
     * @return Returns true if the event is handled; returns false otherwise.
     */
    virtual void OnPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent);

    /**
     * @brief Called when this Service ability is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Service ability.
     * @return Returns a pointer to the <b>sid</b> of the connected Service ability.
     */
    virtual sptr<IRemoteObject> OnConnect(const Want &want);

    /**
     * @brief Called when all abilities connected to this Service ability are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    virtual void OnDisconnect(const Want &want);

    /**
     * @brief Inflates UI controls by using ComponentContainer.
     * You can create a ComponentContainer instance that contains multiple components.
     *
     * @param componentContainer Indicates the component layout defined by the user.
     * @param context Indicates the context to use.
     * @param typeFlag Indicates the window type.
     */
    virtual void SetUIContent(
        const ComponentContainer &componentContainer, std::shared_ptr<Context> &context, int typeFlag);

    /**
     * @brief Inflates layout resources by using the layout resource ID.
     *
     * @param layoutRes Indicates the layout resource ID, which cannot be a negative number.
     * @param context Indicates the context to use.
     * @param typeFlag Indicates the window type.
     */
    virtual void SetUIContent(int layoutRes, std::shared_ptr<Context> &context, int typeFlag);

    /**
     * @brief Called when this ability gains or loses window focus.
     *
     * @param hasFocus Specifies whether this ability has focus.
     */
    virtual void OnWindowFocusChanged(bool hasFocus);

    /**
     * @brief Called when this ability is moved to or removed from the top of the stack.
     *
     * @param topActive Specifies whether this ability is moved to or removed from the top of the stack. The value true
     * indicates that it is moved to the top, and false indicates that it is removed from the top of the stack.
     */
    virtual void OnTopActiveAbilityChanged(bool topActive);

    /**
     * @brief Inflates UI controls by using windowOption.
     *
     * @param windowOption Indicates the window option defined by the user.
     */
    virtual void InitWindow(Rosen::WindowType winType, int32_t displayId, sptr<Rosen::WindowOption> option);

    /**
     * @brief Get the window belong to the ability.
     *
     * @return Returns a Window object pointer.
     */
    virtual const sptr<Rosen::Window> GetWindow();

    /**
     * @brief get the scene belong to the ability.
     *
     * @return Returns a WindowScene object pointer.
     */
    std::shared_ptr<Rosen::WindowScene> GetScene();

    /**
     * @brief Checks whether the main window of this ability has window focus.
     *
     * @return Returns true if this ability currently has window focus; returns false otherwise.
     */
    bool HasWindowFocus();

    /**
    * @description: Obtains api version based on ability.
    * @return api version.
    */
    int GetCompatibleVersion();

    /**
    * @description: Set api version in an ability.
    * @param compatibleVersion api version
    * @return None.
    */
    void SetCompatibleVersion(int compatibleVersion);

    /**
     * @brief Called when a key is lone pressed.
     *
     * @param keyCode Indicates the code of the key long pressed.
     * @param keyEvent Indicates the key-long-press event.
     *
     * @return Returns true if this event is handled and will not be passed further; returns false if this event
     * is not handled and should be passed to other handlers.
     */
    virtual bool OnKeyPressAndHold(int keyCode, const std::shared_ptr<KeyEvent> &keyEvent);

    /**
     * @brief Called back after permissions are requested by using
     * AbilityContext.requestPermissionsFromUser(java.lang.String[],int).
     *
     * @param requestCode Indicates the request code passed to this method from
     * AbilityContext.requestPermissionsFromUser(java.lang.String[],int).
     * @param permissions Indicates the list of permissions requested by using
     * AbilityContext.requestPermissionsFromUser(java.lang.String[],int). This parameter cannot be null.
     * @param grantResults Indicates the granting results of the corresponding permissions requested using
     * AbilityContext.requestPermissionsFromUser(java.lang.String[],int). The value 0 indicates that a
     * permission is granted, and the value -1 indicates not.
     *
     */
    virtual void OnRequestPermissionsFromUserResult(
        int requestCode, const std::vector<std::string> &permissions, const std::vector<int> &grantResults);

    /**
     * @brief Called when this ability is about to leave the foreground and enter the background due to a user
     * operation, for example, when the user touches the Home key.
     *
     */
    virtual void OnLeaveForeground();

    /**
     * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
     * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
     *
     * @param uri Indicates the URI of the data.
     *
     * @return Returns the MIME type that matches the data specified by uri.
     */
    virtual std::string GetType(const Uri &uri);

    /**
     * @brief Inserts a data record into the database. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the position where the data is to insert.
     * @param value Indicates the data to insert.
     *
     * @return Returns the index of the newly inserted data record.
     */
    virtual int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value);

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    virtual void OnConfigurationUpdated(const Configuration &configuration);

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdatedNotify(const Configuration &configuration);

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param level Indicates the memory trim level, which shows the current memory usage status.
     *
     */
    virtual void OnMemoryLevel(int level);

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
     * @brief Updates one or more data records in the database. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the database table storing the data to update.
     * @param value Indicates the data to update. This parameter can be null.
     * @param predicates Indicates filter criteria. If this parameter is null, all data records will be updated by
     * default.
     *
     * @return Returns the number of data records updated.
     */
    virtual int Update(
        const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief get application witch the ability belong
     *
     * @return Returns the application ptr
     */
    std::shared_ptr<OHOSApplication> GetApplication();

    /**
     * @brief Obtains the class name in this ability name, without the prefixed bundle name.
     *
     * @return Returns the class name of this ability.
     */
    std::string GetAbilityName();

    /**
     * @brief OChecks whether the current ability is being destroyed.
     * An ability is being destroyed if you called terminateAbility() on it or someone else requested to destroy it.
     *
     * @return Returns true if the current ability is being destroyed; returns false otherwise.
     */
    bool IsTerminating();

    /**
     * @brief Called when startAbilityForResult(ohos.aafwk.content.Want,int) is called to start an ability and the
     * result is returned. This method is called only on Page abilities. You can start a new ability to perform some
     * calculations and use setResult (int,ohos.aafwk.content.Want) to return the calculation result. Then the system
     * calls back the current method to use the returned data to execute its own logic.
     *
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param resultCode Indicates the result code returned after the ability is started. You can define the result code
     * to identify an error.
     * @param resultData Indicates the data returned after the ability is started. You can define the data returned. The
     * value can be null.
     *
     */
    virtual void OnAbilityResult(int requestCode, int resultCode, const Want &resultData);

    /**
     * @brief Called back when the Back key is pressed.
     * The default implementation destroys the ability. You can override this method.
     *
     */
    virtual void OnBackPressed() override;

    /**
     * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch an
     * ability that has been at the top of the ability stack.
     *
     * @param want Indicates the new Want containing information about the ability.
     */
    virtual void OnNewWant(const Want &want);

    /**
     * @brief Restores data and states of an ability when it is restored by the system. This method should be
     * implemented by a Page ability. This method is called if an ability was destroyed at a certain time due to
     * resource reclaim or was unexpectedly destroyed and the onSaveAbilityState(ohos.utils.PacMap) method was called to
     * save its user data and states. Generally, this method is called after the onStart(ohos.aafwk.content.Want)
     * method.
     *
     *  @param inState Indicates the PacMap object used for storing data and states. This parameter can not be null.
     *
     */
    virtual void OnRestoreAbilityState(const PacMap &inState);

    /**
     * @brief Saves temporary data and states of this ability. This method should be implemented by a Page ability.
     * This method is called when the system determines that the ability may be destroyed in an unexpected situation,
     * for example, when the screen orientation changes or the user touches the Home key. Generally, this method is used
     * only to save temporary states.
     *
     *  @param outState Indicates the PacMap object used for storing user data and states. This parameter cannot be
     * null.
     *
     */
    virtual void OnSaveAbilityState(PacMap &outState);

    /**
     * @brief Called every time a key, touch, or trackball event is dispatched to this ability.
     * You can override this callback method if you want to know that the user has interacted with
     * the device in a certain way while this ability is running. This method, together with onLeaveForeground(),
     * is designed to help abilities intelligently manage status bar notifications. Specifically, they help
     * abilities determine when to cancel a notification.
     *
     */
    virtual void OnEventDispatch();

    /**
     * @brief Sets the want object that can be obtained by calling getWant().
     *
     * @param Want information of other ability
     */
    void SetWant(const AAFwk::Want &want);

    /**
     * @brief Obtains the Want object that starts this ability.
     *
     * @return Returns the Want object that starts this ability.
     */
    std::shared_ptr<AAFwk::Want> GetWant();

    /**
     * @brief Sets the result code and data to be returned by this Page ability to the caller.
     * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int, int, Want) method to
     * receive the result set in the current method. This method can be called only after the ability has been
     * initialized.
     *
     * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the result
     * code to identify an error.
     * @param resultData Indicates the data returned after the ability is destroyed. You can define the data returned.
     * This parameter can be null.
     */
    virtual void SetResult(int resultCode, const Want &resultData) final;

    /**
     * @brief Sets the type of audio whose volume will be adjusted by the volume button.
     *
     * @param volumeType Indicates the AudioManager.AudioVolumeType to set.
     */
    virtual void SetVolumeTypeAdjustedByKey(int volumeType);

    /**
     * @brief Called back when Service is started.
     * This method can be called only by Service. You can use the StartAbility(ohos.aafwk.content.Want) method to start
     * Service. Then the system calls back the current method to use the transferred want parameter to execute its own
     * logic.
     *
     * @param want Indicates the want of Service to start.
     * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by
     * 1 every time the ability is started. For example, if the ability has been started for six times, the value of
     * startId is 6.
     */
    virtual void OnCommand(const AAFwk::Want &want, bool restart, int startId);

    /**
     * @brief dump ability info
     *
     * @param extra dump ability info
     */
    virtual void Dump(const std::string &extra);

    /**
     * @brief dump ability info
     *
     * @param params dump params that indicate different dump targets
     * @param info dump ability info
     */
    virtual void Dump(const std::vector<std::string> &params, std::vector<std::string> &info);

    /**
     * @brief Keeps this Service ability in the background and displays a notification bar.
     * To use this method, you need to request the ohos.permission.KEEP_BACKGROUND_RUNNING permission from the system.
     * The ohos.permission.KEEP_BACKGROUND_RUNNING permission is of the normal level.
     * This method can be called only by Service abilities after the onStart(ohos.aafwk.content.Want) method is called.
     *
     * @param id Identifies the notification bar information.
     * @param notificationRequest Indicates the NotificationRequest instance containing information for displaying a
     * notification bar.
     */
    virtual void KeepBackgroundRunning(int id, const NotificationRequest &notificationRequest) final;

    /**
     * @brief Cancels background running of this ability to free up system memory.
     * This method can be called only by Service abilities when the onStop() method is called.
     *
     */
    virtual void CancelBackgroundRunning() final;

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
     * @brief Deletes one or more data records. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the database table storing the data to delete.
     * @param predicates Indicates filter criteria. If this parameter is null, all data records will be deleted by
     * default.
     *
     * @return Returns the number of data records deleted.
     */
    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Obtains the MIME type of files. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the path of the files to obtain.
     * @param mimeTypeFilter Indicates the MIME type of the files to obtain. This parameter cannot be set to null.
     * 1. * / *: Obtains all types supported by a Data ability.
     * 2. image/ *: Obtains files whose main type is image of any subtype.
     * 3. * /jpg: Obtains files whose subtype is JPG of any main type.
     *
     * @return Returns the MIME type of the matched files; returns null if there is no type that matches the Data
     * ability.
     */
    virtual std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter);

    /**
     * @brief Opens a file. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the open mode, which can be "r" for read-only access, "w" for write-only access
     * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
     * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
     * or "rwt" for read and write access that truncates any existing file.
     *
     * @return Returns the FileDescriptor object of the file descriptor.
     */
    virtual int OpenFile(const Uri &uri, const std::string &mode);

    /**
     * @brief Queries one or more data records in the database. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the database table storing the data to query.
     * @param columns Indicates the columns to be queried, in array, for example, {"name","age"}. You should define the
     * processing logic when this parameter is null.
     * @param predicates Indicates filter criteria. If this parameter is null, all data records will be queried by
     * default.
     *
     * @return Returns the queried data.
     */
    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Sets the main route for this ability.
     *
     * The main route, also called main entry, refers to the default <b>AbilitySlice</b> to present for this ability.
     * This function should be called only on Feature Abilities. If this function is not called in the
     * {@link OnStart(const Want &want)} function for a Feature Ability, the Feature Ability will fail to start.
     *
     * @param entry Indicates the main entry, which is the class name of the <b>AbilitySlice</b> instance to start.
     *
     * @return Returns the result of SetMainRoute
     */
    virtual void SetMainRoute(const std::string &entry) final;

    /**
     * @brief Migrates this ability to the given device on the same distributed network in a reversible way that allows
     * this ability to be migrated back to the local device through reverseContinueAbility(). The ability to migrate and
     * its ability slices must implement the IAbilityContinuation interface. Otherwise, an exception is thrown,
     * indicating that the ability does not support migration.
     *
     * @param deviceId Indicates the ID of the target device where this ability will be migrated to. If this parameter
     * is null, this method has the same effect as ContinueAbilityReversibly().
     *
     */
    virtual void ContinueAbilityReversibly(const std::string &deviceId) final;

    /**
     * @brief  Obtains the ID of the source device from which this ability is migrated.
     *
     * @return Returns the source device ID.
     */
    virtual std::string GetOriginalDeviceId() final;

    /**
     * @brief Obtains the migration state of this ability.
     * @return Returns the migration state.
     */
    virtual ContinuationState GetContinuationState() final;

    /**
     * @brief Obtains the singleton AbilityPackage object to which this ability belongs.
     *
     * @return Returns the singleton AbilityPackage object to which this ability belongs.
     */
    virtual std::shared_ptr<AbilityPackage> GetAbilityPackage();

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
     * @brief Reloads data in the database.
     *
     * @param uri Indicates the position where the data is to reload. This parameter is mandatory.
     * @param extras Indicates the PacMap object containing the additional parameters to be passed in this call. This
     * parameter can be null. If a custom Sequenceable object is put in the PacMap object and will be transferred across
     * processes, you must call BasePacMap.setClassLoader(ClassLoader) to set a class loader for the custom object.
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
     * @brief Obtains the type of audio whose volume is adjusted by the volume button.
     *
     * @return Returns the AudioManager.AudioVolumeType.
     */
    int GetVolumeTypeAdjustedByKey();

    /**
     * @brief Obtains the lifecycle state of this ability.
     *
     * @return Returns the lifecycle state of this ability.
     */
    virtual AbilityLifecycleExecutor::LifecycleState GetState() final;

    /**
     * @brief A Page or Service ability uses this method to start a specific ability. The system locates the target
     * ability from installed abilities based on the value of the want parameter and then starts it. You can specify
     * the ability to start using the want parameter.
     *
     * @param want Indicates the ability to start.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    using AbilityContext::StartAbility;
    virtual ErrCode StartAbility(const Want &want) final;

    /**
     * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
     *
     * @param want Indicates the want containing information about the ability to connect
     *
     * @param conn Indicates the callback object when the target ability is connected.
     *
     * @return True means success and false means failure
     */
    bool ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn) override;

    /**
     * @brief Disconnects the current ability from an ability
     *
     * @param conn Indicates the IAbilityConnection callback object passed by connectAbility after the connection
     *              is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    ErrCode DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn) override;

    /**
     * @brief Destroys another ability that uses the AbilityInfo.AbilityType.SERVICE template.
     * The current ability using either the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE
     * template can call this method to destroy another ability that uses the AbilityInfo.AbilityType.SERVICE
     * template. The current ability itself can be destroyed by calling the terminateAbility() method.
     *
     * @param want Indicates the Want containing information about the ability to destroy.
     *
     * @return Returns true if the ability is destroyed successfully; returns false otherwise.
     */
    bool StopAbility(const AAFwk::Want &want) override;

    /**
     * @brief Release the window and ability.
     */
    void Destroy();

    /**
     * @brief Posts a scheduled Runnable task to a new non-UI thread.
     * The task posted via this method will be executed in a new thread, which allows you to perform certain
     * time-consuming operations. To use this method, you must also override the supportHighPerformanceUI() method.
     * Additionally, the usage of this method must comply with the following constraints: 1、This method can only be
     * used to initialize the component tree in parallel mode. 2、The task can only be processed during system cold
     * start. 3、If the parallel loading mechanism is used, the component tree-related operations to be performed in
     * onActive() and onStop() of the ability slice must also be added to the parallel thread. 4、You must run
     * setUIContent(ohos.agp.components.ComponentContainer) to unlock the UI thread and wait until the component tree is
     * ready. 5、Other time-consuming operations, such as I/O and network processing, cannot be added to the parallel
     * task queue.
     *
     * @param task Indicates the Runnable task to post.
     *
     * @param delayTime Indicates the number of milliseconds after which the task will be executed.
     *
     * @return -
     */
    void PostTask(std::function<void()> task, long delayTime);

    /**
     * @brief Called to set caller information for the application. The default implementation returns null.
     *
     * @return Returns the caller information.
     */
    virtual Uri OnSetCaller();

    /**
     * @brief Call this when your ability should be closed and the mission should be completely removed as a part of
     * finishing the root ability of the mission.
     */
    void TerminateAndRemoveMission() override;

    /**
     * @brief Create a PostEvent timeout task. The default delay is 5000ms
     *
     * @return Return a smart pointer to a timeout object
     */
    std::shared_ptr<AbilityPostEventTimeout> CreatePostEventTimeouter(std::string taskstr);

    /**
     * Releases an obtained form by its ID.
     *
     * <p>After this method is called, the form won't be available for use by the application, but the Form Manager
     * Service still keeps the cache information about the form, so that the application can quickly obtain it based on
     * the {@code formId}.</p>
     * <p><b>Permission: </b>{@link ohos.security.SystemPermission#REQUIRE_FORM}</p>
     *
     * @param formId Indicates the form ID.
     * @return Returns {@code true} if the form is successfully released; returns {@code false} otherwise.
     *
     * <ul>
     * <li>The passed {@code formId} is invalid. Its value must be larger than 0.</li>
     * <li>The specified form has not been added by the application.</li>
     * <li>An error occurred when connecting to the Form Manager Service.</li>
     * <li>The application is not granted with the {@link ohos.security.SystemPermission#REQUIRE_FORM} permission.</li>
     * <li>The form has been obtained by another application and cannot be released by the current application.</li>
     * <li>The form is being restored.</li>
     * </ul>
     */
    bool ReleaseForm(const int64_t formId);

    /**
     * @brief Releases an obtained form by its ID.
     *
     * <p>After this method is called, the form won't be available for use by the application, if isReleaseCache is
     * false, this method is same as {@link #releaseForm(int)}, otherwise the Form Manager Service still store this
     * form in the cache.</p>
     * <p><b>Permission: </b>{@link ohos.security.SystemPermission#REQUIRE_FORM}</p>
     *
     * @param formId Indicates the form ID.
     * @param isReleaseCache Indicates whether to clear cache in service.
     * @return Returns {@code true} if the form is successfully released; returns {@code false} otherwise.
     *
     * <ul>
     * <li>The passed {@code formId} is invalid. Its value must be larger than 0.</li>
     * <li>The specified form has not been added by the application.</li>
     * <li>An error occurred when connecting to the Form Manager Service.</li>
     * <li>The application is not granted with the {@link ohos.security.SystemPermission#REQUIRE_FORM} permission.</li>
     * <li>The form has been obtained by another application and cannot be released by the current application.</li>
     * <li>The form is being restored.</li>
     * </ul>
     */
    bool ReleaseForm(const int64_t formId, const bool isReleaseCache);

    /**
     * @brief Deletes an obtained form by its ID.
     *
     * <p>After this method is called, the form won't be available for use by the application and the Form Manager
     * Service no longer keeps the cache information about the form.</p>
     * <p><b>Permission: </b>{@link ohos.security.SystemPermission#REQUIRE_FORM}</p>
     *
     * @param formId Indicates the form ID.
     * @return Returns {@code true} if the form is successfully deleted; returns {@code false} otherwise.
     *
     * <ul>
     * <li>The passed {@code formId} is invalid. Its value must be larger than 0.</li>
     * <li>The specified form has not been added by the application.</li>
     * <li>An error occurred when connecting to the Form Manager Service.</li>
     * <li>The application is not granted with the {@link ohos.security.SystemPermission#REQUIRE_FORM} permission.</li>
     * <li>The form has been obtained by another application and cannot be deleted by the current application.</li>
     * <li>The form is being restored.</li>
     * </ul>
     */
    bool DeleteForm(const int64_t formId);

    /**
     * @brief The form callback.
     */
    class FormCallback {
    public:
        static const int32_t OHOS_FORM_ACQUIRE_SUCCESS = 0;
        static const int32_t OHOS_FORM_UPDATE_SUCCESS = 0;
        static const int32_t OHOS_FORM_PREVIEW_FAILURE = 1;
        static const int32_t OHOS_FORM_RESTORE_FAILURE = 2;

        /**
         * @brief Called to notify the application that the {@code FormJsInfo} instance has been obtained after
         * the application called the asynchronous method {@link Ability#acquireForm(Want, FormCallback)}.
         * The application must present the form information on a specific page in this callback.
         *
         * @param result Specifies whether the asynchronous form acquisition process is successful.
         *               {@link FormCallback#OHOS_FORM_ACQUIRE_SUCCESS} indicates that the form
         *               is successfully obtained, and other values indicate that the process fails.
         * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
         */
        virtual void OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const = 0;

        /**
         * @brief Called to notify the application that the {@code FormJsInfo} instance has been obtained after
         * the application called the asynchronous method {@link Ability#acquireForm(Want, FormCallback)}.
         * The application must present the form information on a specific page in this callback.
         *
         * @param result Specifies whether the asynchronous form acquisition process is successful.
         *               {@link FormCallback#OHOS_FORM_UPDATE_SUCCESS} indicates that the form is
         *               successfully obtained, and other values indicate that the process fails.
         * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
         */
        virtual void OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const = 0;

        /**
         * @brief Called to notify the application that the {@code Form} provider has been uninstalled and the
         * corresponding
         * {@code Form} instance is no longer available.
         *
         * @param formId Indicates the ID of the {@code Form} instance provided by the uninstalled form provider.
         */
        virtual void OnFormUninstall(const int64_t formId) const = 0;
    };

    /**
     * @brief Obtains a specified form that matches the application bundle name, module name, form name, and
     * other related information specified in the passed {@code Want}.
     *
     * <p>This method is asynchronous. After the {@link FormJsInfo} instance is obtained.
     *
     * @param formId Indicates the form ID.
     * @param want Indicates the detailed information about the form to be obtained, including the bundle name,
     *        module name, ability name, form name, form id, tempForm flag, form dimension, and form customize data.
     * @param callback Indicates the callback to be invoked whenever the {@link FormJsInfo} instance is obtained.
     * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
     */
    bool AcquireForm(const int64_t formId, const Want &want, const std::shared_ptr<FormCallback> callback);

    /**
     * @brief Updates the content of a specified JS form.
     *
     * <p>This method is called by a form provider to update JS form data as needed.
     *
     * @param formId Indicates the form ID.
     * @param formProviderData The data used to update the JS form displayed on the client.
     * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
     */
    bool UpdateForm(const int64_t formId, const FormProviderData &formProviderData);

    /**
     * @brief Cast temp form with formId.
     *
     * @param formId Indicates the form's ID.
     *
     * @return Returns {@code true} if the form is successfully casted; returns {@code false} otherwise.
     */
    bool CastTempForm(const int64_t formId);

    /**
     * @brief Sends a notification to the form framework to make the specified forms visible.
     *
     * <p>After this method is successfully called, {@link Ability#OnVisibilityChanged(std::map<int64_t, int>)}
     * will be called to notify the form provider of the form visibility change event.</p>
     *
     * @param formIds Indicates the IDs of the forms to be made visible.
     * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
     */
    bool NotifyVisibleForms(const std::vector<int64_t> &formIds);

    /**
     * @brief Sends a notification to the form framework to make the specified forms invisible.
     *
     * <p>After this method is successfully called, {@link Ability#OnVisibilityChanged(std::map<int64_t, int>)}
     * will be called to notify the form provider of the form visibility change event.</p>
     *
     * @param formIds Indicates the IDs of the forms to be made invisible.
     * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
     */
    bool NotifyInvisibleForms(const std::vector<int64_t> &formIds);

    /**
     * @brief Set form next refresh time.
     *
     * <p>This method is called by a form provider to set refresh time.
     *
     * @param formId Indicates the ID of the form to set refresh time.
     * @param nextTime Indicates the next time gap now in seconds, can not be litter than 300 seconds.
     * @return Returns {@code true} if seting succeed; returns {@code false} otherwise.
     */

    bool SetFormNextRefreshTime(const int64_t formId, const int64_t nextTime);

    /**
     * @brief Update form.
     *
     * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
     */
    void ProcessFormUpdate(const FormJsInfo &formJsInfo) override;

    /**
     * @brief Uninstall form.
     *
     * @param formId Indicates the ID of the form to uninstall.
     */
    void ProcessFormUninstall(const int64_t formId) override;

    /**
     * @brief Called to reacquire form and update the form host after the death callback is received.
     *
     */
    void OnDeathReceived() override;

    /**
     * @brief Called to return a FormProviderInfo object.
     *
     * <p>You must override this method if your ability will serve as a form provider to provide a form for clients.
     * The default implementation returns nullptr. </p>
     *
     * @param want   Indicates the detailed information for creating a FormProviderInfo.
     *               The Want object must include the form ID, form name of the form,
     *               which can be obtained from Ability#PARAM_FORM_IDENTITY_KEY,
     *               Ability#PARAM_FORM_NAME_KEY, and Ability#PARAM_FORM_DIMENSION_KEY,
     *               respectively. Such form information must be managed as persistent data for further form
     *               acquisition, update, and deletion.
     *
     * @return Returns the created FormProviderInfo object.
     */
    virtual FormProviderInfo OnCreate(const Want &want);

    /**
     * @brief Called to notify the form provider that a specified form has been deleted. Override this method if
     * you want your application, as the form provider, to be notified of form deletion.
     *
     * @param formId Indicates the ID of the deleted form.
     * @return None.
     */
    virtual void OnDelete(const int64_t formId);

    /**
     * @brief Called when the form provider is notified that a temporary form is successfully converted to
     * a normal form.
     *
     * @param formId Indicates the ID of the form.
     * @return None.
     */
    virtual void OnCastTemptoNormal(const int64_t formId);

    /**
     * @brief Called to notify the form provider to update a specified form.
     *
     * @param formId Indicates the ID of the form to update.
     * @return none.
     */
    virtual void OnUpdate(const int64_t formId);

    /**
     * @brief Called when the form provider receives form events from the fms.
     *
     * @param formEventsMap Indicates the form events occurred. The key in the Map object indicates the form ID,
     *                      and the value indicates the event type, which can be either FORM_VISIBLE
     *                      or FORM_INVISIBLE. FORM_VISIBLE means that the form becomes visible,
     *                      and FORM_INVISIBLE means that the form becomes invisible.
     * @return none.
     */
    virtual void OnVisibilityChanged(const std::map<int64_t, int32_t> &formEventsMap);
    /**
     * @brief Called to notify the form provider to update a specified form.
     *
     * @param formId Indicates the ID of the form to update.
     * @param message Form event message.
     */
    virtual void OnTriggerEvent(const int64_t formId, const std::string &message);
    /**
     * @brief Requests for form data update.
     *
     * This method must be called when the application has detected that a system setting item (such as the language,
     * resolution, or screen orientation) being listened for has changed. Upon receiving the update request, the form
     * provider automatically updates the form data (if there is any update) through the form framework, with the update
     * process being unperceivable by the application.
     *
     * @param formId Indicates the ID of the form to update.
     * @return Returns true if the update request is successfully initiated, returns false otherwise.
     */
    bool RequestForm(const int64_t formId);

    /**
     * @brief Requests for form data update, by passing a set of parameters (using Want) to the form provider.
     *
     * This method must be called when the application has detected that a system setting item (such as the language,
     * resolution, or screen orientation) being listened for has changed. Upon receiving the update request, the form
     * provider automatically updates the form data (if there is any update) through the form framework, with the update
     * process being unperceivable by the application.
     *
     * @param formId Indicates the ID of the form to update.
     * @param want Indicates a set of parameters to be transparently passed to the form provider.
     * @return Returns true if the update request is successfully initiated, returns false otherwise.
     */
    bool RequestForm(const int64_t formId, const Want &want);
    /**
     * @brief Enable form update.
     *
     * @param formIds formIds of hostclient.
     */
    bool EnableUpdateForm(const std::vector<int64_t> &formIds);

    /**
     * @brief Disable form update.
     *
     * @param formIds formIds of hostclient.
     */
    bool DisableUpdateForm(const std::vector<int64_t> &formIds);

    /**
     * @brief Check form manager service ready.
     *
     * @return Returns true if form manager service ready; returns false otherwise.
     */
    bool CheckFMSReady();

    /**
     * @brief Get All FormsInfo.
     *
     * @param formInfos Returns the forms' information of all forms provided.
     * @return Returns true if the request is successfully initiated; returns false otherwise.
     */
    bool GetAllFormsInfo(std::vector<FormInfo> &formInfos);

    /**
     * @brief Get forms info by application name.
     *
     * @param bundleName Application name.
     * @param formInfos Returns the forms' information of the specify application name.
     * @return Returns true if the request is successfully initiated; returns false otherwise.
     */
    bool GetFormsInfoByApp(std::string &bundleName, std::vector<FormInfo> &formInfos);

    /**
     * @brief Get forms info by application name and module name.
     *
     * @param bundleName Application name.
     * @param moduleName Module name of hap.
     * @param formInfos Returns the forms' information of the specify application name and module name.
     * @return Returns true if the request is successfully initiated; returns false otherwise.
     */
    bool GetFormsInfoByModule(std::string &bundleName, std::string &moduleName, std::vector<FormInfo> &formInfos);

    /**
     * @brief Acquire a bundle manager, if it not existed,
     * @return returns the bundle manager ipc object, or nullptr for failed.
     */
    sptr<IBundleMgr> GetBundleMgr();

    /**
     * @brief Add the bundle manager instance for debug.
     * @param bundleManager the bundle manager ipc object.
     */
    void SetBundleManager(const sptr<IBundleMgr> &bundleManager);

    /**
     * @brief You can use the IContinuationRegisterManager object to interact with the Device+ control center,
     * including registering and unregistering the ability to migrate, updating the device connection state, and
     * showing the list of devices that can be selected for ability migration.
     *
     * @return Returns true if the migration request is successful; returns false otherwise.
     */
    std::weak_ptr<IContinuationRegisterManager> GetContinuationRegisterManager();

    /**
     * @brief Prepare user data of local Ability.
     *
     * @param wantParams Indicates the user data to be saved.
     * @return If the ability is willing to continue and data saved successfully, it returns true;
     * otherwise, it returns false.
     */
    virtual bool OnContinue(WantParams &wantParams);

    /**
     * @brief Get page ability stack info.
     *
     * @return A string represents page ability stack info, empty if failed;
     */
    virtual std::string GetContentInfo();

    /**
     * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
     * ability slices must implement the IAbilityContinuation interface.
     *
     * @param deviceId Indicates the ID of the target device where this ability will be migrated to.
     *
     */
    virtual void ContinueAbilityWithStack(const std::string &deviceId) final;

    /**
     * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
     * ability slices must implement the IAbilityContinuation interface.
     *
     * @param deviceId Indicates the ID of the target device where this ability will be migrated to. If this parameter
     * is null, this method has the same effect as continueAbility().
     *
     */
    virtual void ContinueAbility(const std::string &deviceId) final;

    /**
     * @brief Callback function to ask the user whether to start the migration .
     *
     * @return If the user allows migration, it returns true; otherwise, it returns false.
     */
    virtual bool OnStartContinuation() override;

    /**
     * @brief Performs batch operations on the database.
     *
     * @param operations Indicates a list of database operations on the database.
     * @return Returns the result of each operation, in array.
     */
    virtual std::vector<std::shared_ptr<DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<DataAbilityOperation>> &operations);

    /**
     * @brief Executes an operation among the batch operations to be executed.
     *
     * @param operation Indicates the operation to execute.
     * @param results Indicates a set of results of the batch operations.
     * @param index Indicates the index of the current operation result in the batch operation results.
     */
    void ExecuteOperation(std::shared_ptr<DataAbilityOperation> &operation,
        std::vector<std::shared_ptr<DataAbilityResult>> &results, int index);

    /**
     * @brief Save user data of local Ability generated at runtime.
     *
     * @param saveData Indicates the user data to be saved.
     * @return If the data is saved successfully, it returns true; otherwise, it returns false.
     */
    virtual bool OnSaveData(WantParams &saveData) override;

    /**
     * @brief After creating the Ability on the remote device,
     *      immediately restore the user data saved during the migration of the Ability on the remote device.
     * @param restoreData Indicates the user data to be restored.
     * @return If the data is restored successfully, it returns true; otherwise, it returns false .
     */
    virtual bool OnRestoreData(WantParams &restoreData) override;

    /**
     * @brief This function can be used to implement the processing logic after the migration is completed.
     *
     * @param result Migration result code. 0 means the migration was successful, -1 means the migration failed.
     * @return None.
     */
    virtual void OnCompleteContinuation(int result) override;

    /**
     * @brief Used to notify the local Ability that the remote Ability has been destroyed.
     *
     * @return None.
     */
    virtual void OnRemoteTerminated() override;

    /**
     * @brief Set WindowScene listener
     *
     * @param listener WindowScene listener
     * @return None.
     */
    void SetSceneListener(const sptr<Rosen::IWindowLifeCycle> &listener);

	/**
     * @brief request a remote object of callee from this ability.
     * @return Returns the remote object of callee.
     */
    virtual sptr<IRemoteObject> CallRequest();

    /**
     * @brief Called back at ability context.
     */
    virtual int GetCurrentWindowMode() override;

    uint32_t sceneFlag_ = 0;
protected:
    /**
     * @brief Acquire a form provider remote object.
     * @return Returns form provider remote object.
     */
    sptr<IRemoteObject> GetFormRemoteObject();

    /**
     * @brief Acquire the launch parameter.
     * @return launch parameter.
     */
    const AAFwk::LaunchParam& GetLaunchParam() const;

    /**
     * @brief process when foreground executed.
     *
     * You can override this function to implement your own processing logic
     */
    virtual void DoOnForeground(const Want& want);

    /**
     * @brief Acquire the window option.
     * @return window option.
     */
    sptr<Rosen::WindowOption> GetWindowOption(const Want &want);

    /**
     * @brief judge where invoke resoreWindowStage in continuation
     * @return true if invoked resoreWindowStage in continuation.
     */
    bool IsRestoredInContinuation() const;

    /**
     * @brief Notify continuation
     *
     * @param want the want param.
     * @param success whether continuation success.
     */
    void NotityContinuationResult(const Want& want, bool success);

protected:
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_ = nullptr;
    std::shared_ptr<Rosen::WindowScene> scene_ = nullptr;
    std::shared_ptr<AbilityStartSetting> setting_ = nullptr;
    sptr<Rosen::IWindowLifeCycle> sceneListener_ = nullptr;
    LaunchParam launchParam_;

private:
    std::shared_ptr<NativeRdb::DataAbilityPredicates> ParsePredictionArgsReference(
        std::vector<std::shared_ptr<DataAbilityResult>> &results, std::shared_ptr<DataAbilityOperation> &operation,
        int numRefs);

    std::shared_ptr<NativeRdb::ValuesBucket> ParseValuesBucketReference(
        std::vector<std::shared_ptr<DataAbilityResult>> &results, std::shared_ptr<DataAbilityOperation> &operation,
        int numRefs);

    int ChangeRef2Value(std::vector<std::shared_ptr<DataAbilityResult>> &results, int numRefs, int index);

    bool CheckAssertQueryResult(std::shared_ptr<NativeRdb::AbsSharedResultSet> &queryResult,
        std::shared_ptr<NativeRdb::ValuesBucket> &&valuesBucket);

    void DispatchLifecycleOnForeground(const Want &want);

    friend class AbilityImpl;
    bool VerifySupportForContinuation();
    void HandleCreateAsContinuation(const Want &want);
    bool IsFlagExists(unsigned int flag, unsigned int flagSet);
    /**
     * @brief Set the start ability setting.
     * @param setting the start ability setting.
     */
    void SetStartAbilitySetting(std::shared_ptr<AbilityStartSetting> setting);

    /**
     * @brief Set the launch param.
     *
     * @param launchParam the launch param.
     */
    void SetLaunchParam(const AAFwk::LaunchParam &launchParam);

private:
    std::shared_ptr<ContinuationHandler> continuationHandler_ = nullptr;
    std::shared_ptr<ContinuationManager> continuationManager_ = nullptr;
    std::shared_ptr<ContinuationRegisterManager> continuationRegisterManager_ = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo_ = nullptr;
    std::shared_ptr<AbilityHandler> handler_ = nullptr;
    std::shared_ptr<LifeCycle> lifecycle_ = nullptr;
    std::shared_ptr<AbilityLifecycleExecutor> abilityLifecycleExecutor_ = nullptr;
    std::shared_ptr<OHOSApplication> application_ = nullptr;
    std::vector<std::string> types_;
    std::shared_ptr<AbilityWindow> abilityWindow_ = nullptr;
    std::shared_ptr<AAFwk::Want> setWant_ = nullptr;
    sptr<IRemoteObject> reverseContinuationSchedulerReplica_ = nullptr;

    bool bWindowFocus_ = false;
    int compatibleVersion_ = 0;

    static const std::string SYSTEM_UI;
    static const std::string STATUS_BAR;
    static const std::string NAVIGATION_BAR;
    static const std::string KEYGUARD;
    sptr<IRemoteObject> providerRemoteObject_ = nullptr;
    // Keep consistent with DMS defines. Used to callback to DMS.
    static const std::string DMS_SESSION_ID;

    // The originating deviceId passed by DMS using want param.
    static const std::string DMS_ORIGIN_DEVICE_ID;

    // If session id cannot get from want, assign it as default.
    static const int DEFAULT_DMS_SESSION_ID;

    std::vector<int64_t> lostedByReconnectTempForms_;
    std::map<int64_t, std::shared_ptr<FormCallback>> appCallbacks_;
    std::map<int64_t, Want> userReqParams_;
    sptr<IBundleMgr> iBundleMgr_;

    static const int32_t OHOS_FORM_ACQUIRE_FORM = 0;
    static const int32_t OHOS_FORM_UPDATE_FORM = 1;

    static const int32_t DELETE_FORM = 3;
    static const int32_t ENABLE_FORM_UPDATE = 5;
    static const int32_t DISABLE_FORM_UPDATE = 6;
    static const int32_t RELEASE_FORM = 8;
    static const int32_t RELEASE_CACHED_FORM = 9;
    static const int64_t MIN_NEXT_TIME = 5;

private:
    /**
     * @brief Delete or release form with formId.
     *
     * @param formId Indicates the form's ID.
     * @param deleteType Indicates the type of delete or release.
     * @return Returns {@code true} if the form is successfully deleted; returns {@code false} otherwise.
     */
    bool DeleteForm(const int64_t formId, const int32_t deleteType);

    /**
     * @brief Clean form resource with formId.
     *
     * @param formId Indicates the form's ID.
     */
    void CleanFormResource(const int64_t formId);

    /**
     * @brief Handle acquire result of the obtained form instance.
     *
     * @param want Indicates the detailed information about the form to be obtained, including the bundle name,
     *        module name, ability name, form name, form id, tempForm flag, form dimension, and form customize data.
     * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
     * @param callback Indicates the callback to be invoked whenever the {@link FormJsInfo} instance is obtained.
     */
    void HandleAcquireResult(
        const Want &want,
        const FormJsInfo &formJsInfo,
        const std::shared_ptr<FormCallback> callback
        );

    /**
     * @brief Handle acquire message of the obtained form instance.
     *
     * @param msgCode Indicates the code of message type.
     * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
     */
    void HandleFormMessage(const int32_t msgCode, const FormJsInfo &formJsInfo);

    /**
     * @brief Notify the forms visibility change event.
     *
     * @param formIds Indicates the IDs of the forms to be made visible or invisible.
     * @param eventType Indicates the form events occurred. FORM_VISIBLE means that the form becomes visible,
     *                  and FORM_INVISIBLE means that the form becomes invisible.
     * @return none.
     */
    bool NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, int32_t eventType);

    /**
     * @brief Check the param of want.
     *
     * @param formId Indicates the form's ID.
     * @param want Indicates the detailed information about the form to be obtained, including the bundle name,
     *        module name, ability name, form name, form id, tempForm flag, form dimension, and form customize data.
     * @return Returns {@code true} if the check result is ok; returns {@code false} ng.
     */
    bool CheckWantValid(const int64_t formId, const Want &want);

    /**
     * @brief Handle enable/disable form update.
     *
     * @param formIds Indicates the IDs of the forms to be made visible.
     * @param updateType Update type.
     * @return Returns true if the result is ok; returns false otherwise.
     */
    bool LifecycleUpdate(std::vector<int64_t> formIds, int32_t updateType);

    /**
     * @brief Reacquire a specified form when the death callback is received.
     *
     * @param formId Indicates the form ID.
     * @param want Indicates the detailed information about the form to be obtained.
     * @return Returns true if the request is successfully initiated; returns false otherwise.
     */
    bool ReAcquireForm(const int64_t formId, const Want &want);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_H
