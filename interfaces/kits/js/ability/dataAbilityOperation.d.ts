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
import { ValuesBucket, DataAbilityPredicates } from './../@ohos.data.rdb';

/**
 * Performs an operation on the database.
 * @devices phone, tablet
 *
 * @since 7
 */
export interface DataAbilityOperation {
  /**
   * Obtains the data path of the operation.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  uri: string;
  /**
   * Creates a DataAbilityOperation instance.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  type: DataAbilityOperationType;
  /**
   * Sets the data records to be inserted or updated.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  valuesBucket: ValuesBucket;
  /**
   * Sets filter criteria used for deleting updating or assert query data.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  predicates: DataAbilityPredicates;
  /**
   * Sets the expected number of rows to update ,delete or assert query.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  expectedCount: number;
  /**
   * Back reference to be used as a filter criterion in setPredicates(DataAbilityPredicates).
   * This method can be used only for update, delete, and assert operations.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  PredicatesBackReferences: {[key: number]: any};
  /**
   * Sets an interrupt flag bit for a batch operation, which can be insert, update, delete, or assert.
   *
   * interrupted Specifies whether a batch operation can be interrupted.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  interrupted: boolean;
}

export enum DataAbilityOperationType {
  /**
   * Indicates an insert operation.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  TYPE_INSERT = 1,
  /**
   * Indicates an update operation.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  TYPE_UPDATE = 2,
  /**
   * Indicates a delete operation.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  TYPE_DELETE = 3,
  /**
   * Indicates an assert operation.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   */
  TYPE_ASSERT = 4
}