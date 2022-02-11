/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

import { AsyncCallback } from "./basic";
import { ResultSet } from './data/rdb/resultSet';
import ExtensionContext from "./application/ExtensionContext";
import Want from './@ohos.application.Want';
import dataAbility from './@ohos.data.dataAbility';
import rdb from './@ohos.data.rdb';
/**
 * class of datashare extension ability.
 *
 * @since 8
 * @sysCap AAFwk
 * @devices phone, tablet, tv, wearable, car
 * @systemapi hide for inner use.
 */
export default class DataShareExtAbility {
    /**
     * Indicates datashare extension ability context.
     *
     * @since 8
     * @sysCap AAFwk
     * @systemapi hide for inner use.
     */
    context?: ExtensionContext;

    /**
     * Called back when a datashare extension ability is started for initialization.
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param want Indicates connection information about the datashare extension ability.
     * @systemapi hide for inner use.
     * @return -
     */
    onCreate?(want: Want): void;

    /**
     * Obtains the MIME type of files. This method should be implemented by a data share.
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param uri Indicates the path of the files to obtain.
     * @param mimeTypeFilter Indicates the MIME type of the files to obtain. This parameter cannot be set to {@code
     *     null}.
     *     <p>1. "&ast;/*": Obtains all types supported by a data share.
     *     <p>2. "image/*": Obtains files whose main type is image of any subtype.
     *     <p>3. "&ast;/jpg": Obtains files whose subtype is JPG of any main type.
     * @return Returns the MIME type of the matched files; returns null if there is no type that matches the Data
     */
    getFileTypes?(uri: string, mimeTypeFilter:string, callback: AsyncCallback<Array<string>>): void;

    /**
     * Opens a file. This method should be implemented by a data share.
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the open mode, which can be "r" for read-only access, "w" for write-only access (erasing
     *     whatever data is currently in the file), "wt" for write access that truncates any existing file,
     *     "wa" for write-only access to append to any existing data, "rw" for read and write access on any
     *     existing data, or "rwt" for read and write access that truncates any existing file.
     * @return Returns the file descriptor.
     */
    openFile?(uri: string, mode: string, callback: AsyncCallback<number>): void;

    /**
     * Inserts a data record into the database. This method should be implemented by a data share.
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param uri Indicates the position where the data is to insert.
     * @param valueBucket Indicates the data to insert.
     * @return Returns the index of the newly inserted data record.
     */
    insert?(uri: string, values: rdb.ValuesBucket, callback: AsyncCallback<number>): void;

    /**
     * Updates one or more data records in the database. This method should be implemented by a data share.
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param uri Indicates the database table storing the data to update.
     * @param valueBucket Indicates the data to update. This parameter can be null.
     * @param predicates Indicates filter criteria. If this parameter is null, all data records will be updated by
     *                   default.
     * @return Returns the number of data records updated.
     * @return -
     */
    update?(uri: string, values: rdb.ValuesBucket, predicates: dataAbility.DataAbilityPredicates,
        callback: AsyncCallback<number>): void;

    /**
     * Deletes one or more data records. This method should be implemented by a data share.
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param uri Indicates the database table storing the data to delete.
     * @param predicates Indicates filter criteria. If this parameter is null, all data records will be deleted by
     *     default.
     * @return Returns the number of data records deleted.
     */
    delete?(uri: string, predicates: dataAbility.DataAbilityPredicates, callback: AsyncCallback<number>): void;

    /**
     * Queries one or more data records in the database. This method should be implemented by a data share.
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param uri Indicates the database table storing the data to query.
     * @param columns Indicates the columns to be queried, in array, for example, {"name","age"}. You should define
     *                the processing logic when this parameter is null.
     * @param predicates Indicates filter criteria. If this parameter is null, all data records will be queried by
     *                   default.
     * @return Returns the queried data.
     */
    query?(uri: string, columns: Array<String>, predicates: dataAbility.DataAbilityPredicates,
        callback: AsyncCallback<ResultSet>): void;

    /**
     * Obtains the MIME type matching the data specified by the uri of the data share. This method should be
     * implemented by a data share.
     *
     * <p>Data abilities supports general data types, including text, HTML, and JPEG.</p>
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param uri Indicates the uri of the data.
     * @return Returns the MIME type that matches the data specified by {@code uri}.
     */
    getType?(uri: string, callback: AsyncCallback<string>): void;

    /**
     * Inserts multiple data records into the database. This method should be implemented by a data share.
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param uri Indicates the position where the data is to insert.
     * @param valueBuckets Indicates the data to insert.
     * @return Returns the number of data records inserted.
     */
    batchInsert?(uri: string, values: Array<rdb.ValuesBucket>, callback: AsyncCallback<number>): void;

    /**
     * Converts the given {@code uri} that refer to the data share into a normalized uri. A normalized uri can be
     * used across devices, persisted, backed up, and restored. It can refer to the same item in the data share
     * even if the context has changed.
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param uri Indicates the uri to normalize.
     * @return Returns the normalized uri if the data share supports uri normalization;
     */
    normalizeUri?(uri: string, callback: AsyncCallback<string>): void;

    /**
     * Converts the given normalized {@code uri} generated by {@link #normalizeUri(uri)} into a denormalized one.
     * The default implementation of this method returns the original uri passed to it.
     *
     * @devices phone, tablet, tv, wearable, car
     * @since 8
     * @sysCap AAFwk
     * @param uri Indicates the uri to denormalize.
     * @return Returns the denormalized {@code uri} object if the denormalization is successful; returns the original
     * {@code uri} passed to this method if there is nothing to do; returns {@code null} if the data identified by
     * the original {@code uri} cannot be found in the current environment.
     */
    denormalizeUri?(uri: string, callback: AsyncCallback<string>): void;
}