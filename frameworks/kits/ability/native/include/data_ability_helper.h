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

#ifndef FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_HELPER_H
#define FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_HELPER_H

#include <mutex>
#include <map>
#include <string>

#include "context.h"
#include "uri.h"

using Uri = OHOS::Uri;

namespace OHOS {
namespace NativeRdb {
class AbsSharedResultSet;
class DataAbilityPredicates;
class ValuesBucket;
}  // namespace NativeRdb
namespace AppExecFwk {
using string = std::string;
class DataAbilityResult;
class DataAbilityOperation;
class PacMap;
class IDataAbilityObserver;
class DataAbilityHelper final : public std::enable_shared_from_this<DataAbilityHelper> {
public:
    ~DataAbilityHelper() = default;

    /**
     * @brief Creates a DataAbilityHelper instance without specifying the Uri based on the given Context.
     *
     * @param context Indicates the Context object on OHOS.
     *
     * @return Returns the created DataAbilityHelper instance where Uri is not specified.
     */
    static std::shared_ptr<DataAbilityHelper> Creator(const std::shared_ptr<Context> &context);

    /**
     * @brief Creates a DataAbilityHelper instance with the Uri specified based on the given Context.
     *
     * @param context Indicates the Context object on OHOS.
     * @param uri Indicates the database table or disk file to operate.
     *
     * @return Returns the created DataAbilityHelper instance with a specified Uri.
     */
    static std::shared_ptr<DataAbilityHelper> Creator(
        const std::shared_ptr<Context> &context, const std::shared_ptr<Uri> &uri);

    /**
     * @brief You can use this method to specify the Uri of the data to operate and set the binding relationship
     * between the ability using the Data template (Data ability for short) and the associated client process in
     * a DataAbilityHelper instance.
     *
     * @param context Indicates the Context object on OHOS.
     * @param uri Indicates the database table or disk file to operate.
     * @param tryBind Specifies whether the exit of the corresponding Data ability process causes the exit of the
     * client process.
     *
     * @return Returns the created DataAbilityHelper instance.
     */
    static std::shared_ptr<DataAbilityHelper> Creator(
        const std::shared_ptr<Context> &context, const std::shared_ptr<Uri> &uri, const bool tryBind);

    /**
     * @brief Creates a DataAbilityHelper instance without specifying the Uri based.
     *
     * @param token Indicates the System token.
     *
     * @return Returns the created DataAbilityHelper instance where Uri is not specified.
     */
    static std::shared_ptr<DataAbilityHelper> Creator(const sptr<IRemoteObject> &token);

    /**
     * @brief You can use this method to specify the Uri of the data to operate and set the binding relationship
     * between the ability using the Data template (Data ability for short) and the associated client process in
     * a DataAbilityHelper instance.
     *
     * @param token Indicates the System token.
     * @param uri Indicates the database table or disk file to operate.
     *
     * @return Returns the created DataAbilityHelper instance.
     */
    static std::shared_ptr<DataAbilityHelper> Creator(
        const sptr<IRemoteObject> &token, const std::shared_ptr<Uri> &uri);

    /**
     * @brief Releases the client resource of the Data ability.
     * You should call this method to releases client resource after the data operations are complete.
     *
     * @return Returns true if the resource is successfully released; returns false otherwise.
     */
    bool Release();
    /**
     * @brief Obtains the MIME types of files supported.
     *
     * @param uri Indicates the path of the files to obtain.
     * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
     *
     * @return Returns the matched MIME types. If there is no match, null is returned.
     */
    std::vector<std::string> GetFileTypes(Uri &uri, const std::string &mimeTypeFilter);

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
    int OpenFile(Uri &uri, const std::string &mode);

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
    int OpenRawFile(Uri &uri, const std::string &mode);

    /**
     * @brief Inserts a single data record into the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
     *
     * @return Returns the index of the inserted data record.
     */
    int Insert(Uri &uri, const NativeRdb::ValuesBucket &value);

    /**
     * @brief Updates data records in the database.
     *
     * @param uri Indicates the path of data to update.
     * @param value Indicates the data to update. This parameter can be null.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     *
     * @return Returns the number of data records updated.
     */
    int Update(Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Deletes one or more data records from the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     *
     * @return Returns the number of data records deleted.
     */
    int Delete(Uri &uri, const NativeRdb::DataAbilityPredicates &predicates);

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
        Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
     * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
     *
     * @param uri Indicates the URI of the data.
     *
     * @return Returns the MIME type that matches the data specified by uri.
     */
    std::string GetType(Uri &uri);

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
    bool Reload(Uri &uri, const PacMap &extras);

    /**
     * @brief Inserts multiple data records into the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param values Indicates the data records to insert.
     *
     * @return Returns the number of data records inserted.
     */
    int BatchInsert(Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values);

    /**
     * @brief Registers an observer to DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     */
    void RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);

    /**
     * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     */
    void UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);

    /**
     * @brief Notifies the registered observers of a change to the data resource specified by Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     */
    void NotifyChange(const Uri &uri);

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
    Uri NormalizeUri(Uri &uri);

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
    Uri DenormalizeUri(Uri &uri);

    /**
     * @brief Performs batch operations on the database.
     *
     * @param uri Indicates the path of data to operate.
     * @param operations Indicates a list of database operations on the database.
     * @return Returns the result of each operation, in array.
     */
    std::vector<std::shared_ptr<DataAbilityResult>> ExecuteBatch(
        const Uri &uri, const std::vector<std::shared_ptr<DataAbilityOperation>> &operations);

private:
    DataAbilityHelper(const std::shared_ptr<Context> &context, const std::shared_ptr<Uri> &uri,
        const sptr<AAFwk::IAbilityScheduler> &dataAbilityProxy, bool tryBind = false);
    DataAbilityHelper(const std::shared_ptr<Context> &context);
    DataAbilityHelper(const sptr<IRemoteObject> &token, const std::shared_ptr<Uri> &uri,
        const sptr<AAFwk::IAbilityScheduler> &dataAbilityProxy);
    DataAbilityHelper(const sptr<IRemoteObject> &token);

    void AddDataAbilityDeathRecipient(const sptr<IRemoteObject> &token);
    void OnSchedulerDied(const wptr<IRemoteObject> &remote);

    bool CheckUriParam(const Uri &uri);
    bool CheckOhosUri(const Uri &uri);

    sptr<IRemoteObject> token_;
    std::weak_ptr<Context> context_;
    std::shared_ptr<Uri> uri_ = nullptr;
    bool tryBind_ = false;
    bool isSystemCaller_ = false;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy_ = nullptr;
    std::mutex lock_;
    static std::mutex oplock_;

    sptr<IRemoteObject::DeathRecipient> callerDeathRecipient_ = nullptr;  // caller binderDied Recipient
    
    std::map<sptr<AAFwk::IDataAbilityObserver>, sptr<AAFwk::IAbilityScheduler>> registerMap_;
    
	std::map<sptr<AAFwk::IDataAbilityObserver>, std::string> uriMap_;

};

class DataAbilityDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;

    explicit DataAbilityDeathRecipient(RemoteDiedHandler handler);

    virtual ~DataAbilityDeathRecipient();

    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    RemoteDiedHandler handler_;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_HELPER_H