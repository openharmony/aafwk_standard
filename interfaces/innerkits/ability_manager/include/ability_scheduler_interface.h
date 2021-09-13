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

#ifndef OHOS_AAFWK_ABILITY_SCHEDULER_INTERFACE_H
#define OHOS_AAFWK_ABILITY_SCHEDULER_INTERFACE_H

#include <iremote_broker.h>

#include "dummy_values_bucket.h"
#include "dummy_data_ability_predicates.h"
#include "dummy_result_set.h"
#include "aafwk_dummy_configuration.h"
#include "lifecycle_state_info.h"
#include "pac_map.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
using OHOS::AppExecFwk::DataAbilityPredicates;
using OHOS::AppExecFwk::PacMap;
using OHOS::AppExecFwk::ResultSet;
using OHOS::AppExecFwk::ValuesBucket;

/**
 * @class IAbilityScheduler
 * IAbilityScheduler is used to schedule ability kit lifecycle.
 */
class IAbilityScheduler : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.AbilityScheduler");

    /*
     * ScheduleAbilityTransaction,  schedule ability to transform life state.
     *
     * @param Want, Special Want for service type's ability.
     * @param targetState, The lifecycle state to be transformed
     */
    virtual void ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState) = 0;

    /*
     * SendResult, Send result to app when ability is terminated with result want.
     *
     * @param requestCode, the requestCode of the ability to start.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the want of the ability to terminate.
     */
    virtual void SendResult(int requestCode, int resultCode, const Want &resultWant) = 0;

    /*
     * ScheduleConnectAbility,  schedule service ability to connect.
     *
     * @param Want, Special Want for service type's ability.
     */
    virtual void ScheduleConnectAbility(const Want &want) = 0;

    /*
     * ScheduleDisconnectAbility, schedule service ability to disconnect.
     */
    virtual void ScheduleDisconnectAbility(const Want &want) = 0;

    /*
     * ScheduleCommandAbility, schedule service ability to command.
     */
    virtual void ScheduleCommandAbility(const Want &want, bool restart, int startId) = 0;

    /*
     * ScheduleSaveAbilityState, scheduling save ability state.
     */
    virtual void ScheduleSaveAbilityState(PacMap &outState) = 0;

    /*
     * ScheduleRestoreAbilityState, scheduling restore ability state.
     */
    virtual void ScheduleRestoreAbilityState(const PacMap &inState) = 0;

    /*
     * ScheduleUpdateConfiguration, scheduling update configuration.
     */
    virtual void ScheduleUpdateConfiguration(const DummyConfiguration &config) = 0;

    /**
     * @brief Obtains the MIME types of files supported.
     *
     * @param uri Indicates the path of the files to obtain.
     * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
     *
     * @return Returns the matched MIME types. If there is no match, null is returned.
     */
    virtual std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) = 0;

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
    virtual int OpenFile(const Uri &uri, const std::string &mode) = 0;

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
    virtual int OpenRawFile(const Uri &uri, const std::string &mode) = 0;

    /**
     * @brief Inserts a single data record into the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
     *
     * @return Returns the index of the inserted data record.
     */
    virtual int Insert(const Uri &uri, const ValuesBucket &value) = 0;

    /**
     * @brief Updates data records in the database.
     *
     * @param uri Indicates the path of data to update.
     * @param value Indicates the data to update. This parameter can be null.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     *
     * @return Returns the number of data records updated.
     */
    virtual int Update(const Uri &uri, const ValuesBucket &value, const DataAbilityPredicates &predicates) = 0;

    /**
     * @brief Deletes one or more data records from the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     *
     * @return Returns the number of data records deleted.
     */
    virtual int Delete(const Uri &uri, const DataAbilityPredicates &predicates) = 0;

    /**
     * @brief Deletes one or more data records from the database.
     *
     * @param uri Indicates the path of data to query.
     * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     *
     * @return Returns the query result.
     */
    virtual std::shared_ptr<ResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns, const DataAbilityPredicates &predicates) = 0;

    /**
     * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
     * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
     *
     * @param uri Indicates the URI of the data.
     *
     * @return Returns the MIME type that matches the data specified by uri.
     */
    virtual std::string GetType(const Uri &uri) = 0;

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
    virtual bool Reload(const Uri &uri, const PacMap &extras) = 0;

    /**
     * @brief Inserts multiple data records into the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param values Indicates the data records to insert.
     *
     * @return Returns the number of data records inserted.
     */
    virtual int BatchInsert(const Uri &uri, const std::vector<ValuesBucket> &values) = 0;

    /**
     * @brief notify multi window mode changed.
     *
     * @param winModeKey Indicates ability Window display mode.
     * @param flag Indicates this ability has been enter this mode.
     */
    virtual void NotifyMultiWinModeChanged(int32_t winModeKey, bool flag) = 0;

    /**
     * @brief notify this ability is top active ability.
     *
     * @param flag true: Indicates this ability is top active ability
     */
    virtual void NotifyTopActiveAbilityChanged(bool flag) = 0;

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
    virtual Uri NormalizeUri(const Uri &uri) = 0;

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
    virtual Uri DenormalizeUri(const Uri &uri) = 0;

    enum {
        // ipc id for scheduling ability to a state of life cycle
        SCHEDULE_ABILITY_TRANSACTION = 0,

        // ipc id for sending result to caller
        SEND_RESULT,

        // ipc id for scheduling service ability to connect
        SCHEDULE_ABILITY_CONNECT,

        // ipc id for scheduling service ability to disconnect
        SCHEDULE_ABILITY_DISCONNECT,

        // ipc id for scheduling service ability to command
        SCHEDULE_ABILITY_COMMAND,

        // ipc id for scheduling save ability state
        SCHEDULE_SAVE_ABILITY_STATE,

        // ipc id for scheduling restore ability state
        SCHEDULE_RESTORE_ABILITY_STATE,

        // ipc id for scheduling getFileTypes
        SCHEDULE_GETFILETYPES,

        // ipc id for scheduling openFile
        SCHEDULE_OPENFILE,

        // ipc id for scheduling openRawFile
        SCHEDULE_OPENRAWFILE,

        // ipc id for scheduling insert
        SCHEDULE_INSERT,

        // ipc id for scheduling update
        SCHEDULE_UPDATE,

        // ipc id for scheduling delete
        SCHEDULE_DELETE,

        // ipc id for scheduling query
        SCHEDULE_QUERY,

        // ipc id for scheduling getType
        SCHEDULE_GETTYPE,

        // ipc id for scheduling Reload
        SCHEDULE_RELOAD,

        // ipc id for scheduling BatchInsert​
        SCHEDULE_BATCHINSERT,

        // ipc id for scheduling multi window changed
        MULTI_WIN_CHANGED,

        // ipc id for scheduling update configuration
        SCHEDULE_UPDATE_CONFIGURATION,

        // ipc id for notify this ability is top active
        TOP_ACTIVE_ABILITY_CHANGED,
        // ipc id for scheduling NormalizeUri
        SCHEDULE_NORMALIZEURI,

        // ipc id for scheduling DenormalizeUri
        SCHEDULE_DENORMALIZEURI,
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_SCHEDULER_INTERFACE_H
