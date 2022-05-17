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

#ifndef FOUNDATION_APPEXECFWK_ABILITY_THREAD_H
#define FOUNDATION_APPEXECFWK_ABILITY_THREAD_H

#include "want.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "ability.h"
#include "ability_local_record.h"
#include "context.h"
#include "extension_impl.h"
#include "ohos_application.h"
#include "ability_scheduler_stub.h"
#include "pac_map.h"
#include "ipc_singleton.h"

namespace OHOS {
namespace AppExecFwk {
using AbilitySchedulerStub = OHOS::AAFwk::AbilitySchedulerStub;
using LifeCycleStateInfo = OHOS::AAFwk::LifeCycleStateInfo;
class AbilityImpl;
class Ability;
class AbilityHandler;
class AbilityLocalRecord;
class ApplicationImpl;
class OHOSApplication;
class AbilityHandler;
class AbilityThread : public AbilitySchedulerStub {
public:
    /**
     * @brief Default constructor used to create a AbilityThread instance.
     */
    AbilityThread();
    ~AbilityThread();

    /**
     * @description: Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     * @param mainRunner The runner which main_thread holds.
     * @param appContext the AbilityRuntime context
     */
    static void AbilityThreadMain(std::shared_ptr<OHOSApplication> &application,
        const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner,
        const std::shared_ptr<AbilityRuntime::Context> &appContext);

    /**
     * @description: Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     * @param appContext the AbilityRuntime context
     */
    static void AbilityThreadMain(
        std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AbilityRuntime::Context> &appContext);

    /**
     * @description: Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     * @param mainRunner The runner which main_thread holds.
     */
    void Attach(std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<EventRunner> &mainRunner, const std::shared_ptr<AbilityRuntime::Context> &appContext);

    /**
     * @description: Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     * @param mainRunner The runner which main_thread holds.
     */
    void AttachExtension(std::shared_ptr<OHOSApplication> &application,
        const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<EventRunner> &mainRunner);

    /**
     * @description: Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     * @param mainRunner The runner which main_thread holds.
     */
    void AttachExtension(std::shared_ptr<OHOSApplication> &application,
        const std::shared_ptr<AbilityLocalRecord> &abilityRecord);

    void InitExtensionFlag(const std::shared_ptr<AbilityLocalRecord> &abilityRecord);

    /**
     * @description: Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     */
    void Attach(
        std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AbilityRuntime::Context> &appContext);

    /**
     * @description:  Provide operating system AbilityTransaction information to the observer
     * @param want Indicates the structure containing Transaction information about the ability.
     * @param lifeCycleStateInfo Indicates the lifecycle state.
     */
    void ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState);

    /**
     * @description:  Provide operating system ConnectAbility information to the observer
     * @param  want Indicates the structure containing connect information about the ability.
     */
    void ScheduleConnectAbility(const Want &want);

    /**
     * @description: Provide operating system ConnectAbility information to the observer
     * @return  None
     */
    void ScheduleDisconnectAbility(const Want &want);

    /**
     * @description: Provide operating system CommandAbility information to the observer
     *
     * @param want The Want object to command to.
     *
     * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     *
     * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by
     * 1 every time the ability is started. For example, if the ability has been started for six times, the value of
     * startId is 6.
     */
    void ScheduleCommandAbility(const Want &want, bool restart, int startId);

    /**
     * @description: Provide operating system SaveabilityState information to the observer
     */
    void ScheduleSaveAbilityState();

    /**
     * @description:  Provide operating system RestoreAbilityState information to the observer
     * @param state Indicates resotre ability state used to dispatchRestoreAbilityState.
     */
    void ScheduleRestoreAbilityState(const PacMap &state);

    /**
     * @brief ScheduleUpdateConfiguration, scheduling update configuration.
     */
    void ScheduleUpdateConfiguration(const Configuration &config);

    /**
     * @brief Send the result code and data to be returned by this Page ability to the caller.
     * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int, int, Want) method to
     * receive the result set in the current method. This method can be called only after the ability has been
     * initialized.
     *
     * @param requestCode Indicates the request code for send.
     * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the result
     * code to identify an error.
     * @param want Indicates the data returned after the ability is destroyed. You can define the data returned. This
     * parameter can be null.
     */
    void SendResult(int requestCode, int resultCode, const Want &resultData);

    /**
     * @brief Obtains the MIME types of files supported.
     *
     * @param uri Indicates the path of the files to obtain.
     * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
     *
     * @return Returns the matched MIME types. If there is no match, null is returned.
     */
    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter);

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
    int OpenFile(const Uri &uri, const std::string &mode);

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
    int OpenRawFile(const Uri &uri, const std::string &mode);

    /**
     * @brief Inserts a single data record into the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
     *
     * @return Returns the index of the inserted data record.
     */
    int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value);

    std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap);

    /**
     * @brief Updates data records in the database.
     *
     * @param uri Indicates the path of data to update.
     * @param value Indicates the data to update. This parameter can be null.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     *
     * @return Returns the number of data records updated.
     */
    int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Deletes one or more data records from the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     *
     * @return Returns the number of data records deleted.
     */
    int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Deletes one or more data records from the database.
     *
     * @param uri Indicates the path of data to query.
     * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     *
     * @return Returns the query result.
     */
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
     * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
     *
     * @param uri Indicates the URI of the data.
     *
     * @return Returns the MIME type that matches the data specified by uri.
     */
    std::string GetType(const Uri &uri);

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
    bool Reload(const Uri &uri, const PacMap &extras);

    /**
     * @brief Inserts multiple data records into the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param values Indicates the data records to insert.
     *
     * @return Returns the number of data records inserted.
     */
    int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values);

    /**
     * @brief continue ability to target device.
     *
     * @param deviceId target deviceId
     * @param versionCode Target bundle version.
     */
    void ContinueAbility(const std::string& deviceId, uint32_t versionCode);

    /**
     * @brief notify this ability continuation result.
     *
     * @param result: Continuation result
     */
    void NotifyContinuationResult(int32_t result);

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
    Uri NormalizeUri(const Uri &uri);

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
    Uri DenormalizeUri(const Uri &uri);

    /**
     * @brief Registers an observer to DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     */
    bool HandleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);

    /**
     * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     */
    bool HandleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);

    /**
     * @brief Notifies the registered observers of a change to the data resource specified by Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     */
    bool HandleNotifyChange(const Uri &uri);

    /**
     * @brief Registers an observer to DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     */
    bool ScheduleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);

    /**
     * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     */
    bool ScheduleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);

    /**
     * @brief Notifies the registered observers of a change to the data resource specified by Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     */
    bool ScheduleNotifyChange(const Uri &uri);

    /**
     * @brief Access authority verification.
     *
     * @return Returns true on success, others on failure.
     */
    bool CheckObsPermission();

	/**
     * @brief Dump ability runner info.
     *
     * @param runnerInfo ability runner info.
     */
    void DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info);

    sptr<IRemoteObject> CallRequest();

    std::vector<std::shared_ptr<DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<DataAbilityOperation>> &operations);
private:
    void DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info);
    void DumpOtherInfo(std::vector<std::string> &info);
    /**
     * @description: Create the abilityname.
     *
     * @param abilityRecord Indicates the abilityRecord.
     * @param application Indicates the application.
     * @return Returns the abilityname.
     */
    std::string CreateAbilityName(const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
        std::shared_ptr<OHOSApplication> &application);

    /**
     * @description: Create and init contextDeal.
     *
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     * @param abilityObject Indicates the abilityObject.
     *
     * @return Returns the contextDeal.
     *
     */
    std::shared_ptr<ContextDeal> CreateAndInitContextDeal(std::shared_ptr<OHOSApplication> &application,
        const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<Context> &abilityObject);

    /**
     * @description:  Handle the life cycle of Ability.
     * @param want  Indicates the structure containing lifecycle information about the ability.
     * @param lifeCycleStateInfo  Indicates the lifeCycleStateInfo.
     */
    void HandleAbilityTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo);

    /**
     * @description:  Handle the life cycle of Extension.
     * @param want  Indicates the structure containing lifecycle information about the extension.
     * @param lifeCycleStateInfo  Indicates the lifeCycleStateInfo.
     */
    void HandleExtensionTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo);

    /**
     * @description:  Handle the current connection of Ability.
     * @param want  Indicates the structure containing connection information about the ability.
     */
    void HandleConnectAbility(const Want &want);

    /**
     * @description:  Handle the current disconnection of Ability.
     */
    void HandleDisconnectAbility(const Want &want);

    /**
     * @brief Handle the current command of Ability.
     *
     * @param want The Want object to command to.
     *
     * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     *
     * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by
     * 1 every time the ability is started. For example, if the ability has been started for six times, the value of
     * startId is 6.
     */
    void HandleCommandAbility(const Want &want, bool restart, int startId);

    /**
     * @description:  Handle the current connection of Extension.
     * @param want  Indicates the structure containing connection information about the extension.
     */
    void HandleConnectExtension(const Want &want);

    /**
     * @description:  Handle the current disconnection of Extension.
     */
    void HandleDisconnectExtension(const Want &want);

    /**
     * @brief Handle the current command of Extension.
     *
     * @param want The Want object to command to.
     *
     * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     *
     * @param startId Indicates the number of times the Service extension has been started. The startId is incremented
     * by 1 every time the extension is started. For example, if the extension has been started for six times, the
     * value of startId is 6.
     */
    void HandleCommandExtension(const Want &want, bool restart, int startId);

    /**
     * @description: Handle the restoreAbility state.
     * @param state  Indicates save ability state used to dispatchRestoreAbilityState.
     */
    void HandleRestoreAbilityState(const PacMap &state);

    /*
     * @brief Handle the scheduling update configuration.
     */
    void HandleUpdateConfiguration(const Configuration &config);

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int BlockAbility();
    #endif

    /**
     * @brief Handle the scheduling update configuration of extension.
     *
     * @param config Configuration
     */
    void HandleExtensionUpdateConfiguration(const Configuration &config);

    std::shared_ptr<AbilityRuntime::AbilityContext> BuildAbilityContext(
        const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<OHOSApplication> &application,
        const sptr<IRemoteObject> &token, const std::shared_ptr<AbilityRuntime::Context> &stageContext);

    std::shared_ptr<AbilityImpl> abilityImpl_ = nullptr;
    sptr<IRemoteObject> token_;
    std::shared_ptr<Ability> currentAbility_ = nullptr;
    std::shared_ptr<AbilityRuntime::ExtensionImpl> extensionImpl_ = nullptr;
    std::shared_ptr<AbilityRuntime::Extension> currentExtension_ = nullptr;
    std::shared_ptr<AbilityHandler> abilityHandler_ = nullptr;
    std::shared_ptr<EventRunner> runner_ = nullptr;
    bool isExtension_ = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_ABILITY_THREAD_H
