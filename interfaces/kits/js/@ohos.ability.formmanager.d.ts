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
import { AsyncCallback } from './.basic';
import { Want } from './ability/want';
import { FormInfo } from './ability/forminfo';

/**
 * Provides utilities for system application components to access ability form
 * on the OHOS OS.
 * @name formManager
 * @since 7
 * @sysCap AAFwk
 * @devices phone, tablet
 * @permission N/A
 */
declare namespace formManager {
  /**
   * user need to force refresh form.
   *
   * <p>You can use this method to ask for newly form from service.</p>
   *
   * @param formId the specify form id.
   * @since 7
   */
  function requestForm(formId: number, callback: AsyncCallback<void>): void;
  function requestForm(formId: number): Promise<void>;

  /**
   * delete forms.
   *
   * <p>You can use this method to delete ability form.</p>
   *
   * @param formId Indicates the form to be deleted.
   * @since 7
   */
  function deleteForm(formId: number, callback: AsyncCallback<void>): void;
  function deleteForm(formId: number): Promise<void>;

  /**
   * release form.
   *
   * <p>You can use this method to release ability form, it does not delete form cache in
   * form manager.</p>
   *
   * @param formId Indicates the form to be released.
   * @param isReleaseCache Indicates whether to delete cache in service.
   * @since 7
   */
  function releaseForm(formId: number, isReleaseCache: boolean, callback: AsyncCallback<void>): void;
  function releaseForm(formId: number, isReleaseCache: boolean): Promise<void>;

  /**
   * Sends a notification to the form framework to make the specified forms visible.
   *
   * @param formIds Indicates the IDs of the forms to be made visible.
   * @since 7
   */
  function notifyVisibleForms(formIds: Array<number>, callback: AsyncCallback<void>): void;
  function notifyVisibleForms(formIds: Array<number>): Promise<void>;

  /**
   * Sends a notification to the form framework to make the specified forms invisible.
   *
   * @param formIds Indicates the IDs of the forms to be made invisible.
   * @since 7
   */
  function notifyInvisibleForms(formIds: Array<number>, callback: AsyncCallback<void>): void;
  function notifyInvisibleForms(formIds: Array<number>): Promise<void>;

  /**
   * set form refresh state to true.
   *
   * <p>You can use this method to set form refresh state to true, the form can receive new
   * update from service.</p>
   *
   * @param formIds the specify form id.
   * @since 7
   */
  function enableFormsUpdate(formIds: Array<number>, callback: AsyncCallback<void>): void;
  function enableFormsUpdate(formIds: Array<number>): Promise<void>;

  /**
   * set form refresh state to false.
   *
   * <p>You can use this method to set form refresh state to false, the form do not receive
     * new update from service.</p>
   *
   * @param formIds the specify form id.
   * @since 7
   */
  function disableFormsUpdate(formIds: Array<number>, callback: AsyncCallback<void>): void;
  function disableFormsUpdate(formIds: Array<number>): Promise<void>;

  /**
   * Check form manager service ready.
   *
   * <p>You can use this method to check if form manager service is ready.</p>
   *
   * @return Returns {@code true} form manager service ready; returns {@code false} otherwise.
   * @since 7
   */
  function checkFMSReady(callback: AsyncCallback<boolean>): void;
  function checkFMSReady(): Promise<boolean>;

  /**
   * Cast temp form to narmal form.
   *
   * <p>You can use this method to cast a temp form to normal form.</p>
   *
   * @param formId the specify form id to be casted.
   * @since 7
   */
  function castTempForm(formId: number, callback: AsyncCallback<void>): void;
  function castTempForm(formId: number): Promise<void>;

  /**
   * Checks for and deletes invalid forms of the application in the Form Manager Service based on the list of valid
   * form IDs passed.
   *
   * <p>If an empty list is passed to this method, the Form Manager Service will delete all forms of the
   * application.</p>
   *
   * @param persistedIds Indicates the list of valid forms with persisted IDs.
   * @since 7
   */
  function checkAndDeleteInvalidForms(persistedIds: Array<number>, callback: AsyncCallback<void>): void;
  function checkAndDeleteInvalidForms(persistedIds: Array<number>): Promise<void>;
  
   /**
   * Updates the content of a specified JS form.
   *
   * <p>This method is called by a form provider to update JS form data as needed.
   *
   * @param formId Indicates the ID of the JS form to update.
   * @param formBindingData Indicates the object used to update the JS form displayed
   *                        on the client.
   * @return Returns {@code true} if the update is successful; returns {@code false} otherwise.
   * @throws FormException Throws this exception if the form fails to be obtained due to any of the following reasons:
   * <ul>
   * <li>The passed {@code formID} or {@code component} is invalid. The value of {@code formID} must be larger than 0,
   * and {@code component} must not be null.</li>
   * <li>An error occurred when connecting to the Form Manager Service.</li>
   * <li>The specified form ID does not exist. </li>
   * <li>The form has been obtained by another application and cannot be updated by the current application.</li>
   * <li>The form is being restored.</li>
   * </ul>
   * @since 7
   */
  function updateForm(formid: number, data: FormBindingData, callback: AsyncCallback<boolean>): void;
  function updateForm(formid: number, data: FormBindingData): Promise<boolean>;
  
  /**
     * Set next refresh time since now.
     *
     * <p>This method is called by a form provider to set refresh time.
     *
     * @param formId Indicates the ID of the form to set refreshTime.
     * @param bundleName Indicates the bundleName of current form.
     * @param nextTime Indicates the next time gap now in seconds, can not be litter than 5 mins.
     * @return Returns {@code true} if seting succeed; returns {@code false} otherwise.
     * @throws FormException Throws this exception if the form fails to be obtained due to any of the following reasons:
     * <ul>
     * <li>The passed {@code formId} or {@code nextTime} is invalid. The value of {@code formId} must be larger
     * than 0, and {@code nextTime} must at least be 120 (5min).</li>
     * <li>An error occurred when connecting to the Form Manager Service.</li>
     * <li>The specified form ID does not exist. </li>
     * <li>The form has been obtained by another application and cannot be updated by the current application.</li>
     * <li>The form is being restored.</li>
     * </ul>
     * @since 7
     */
  function setFormNextRefreshTime(formid: number, nextTime: number, callback: AsyncCallback<boolean>): boolean;
  function setFormNextRefreshTime(formid: number, nextTime: number): Promise<boolean>;

  /**
   * get all forms info.
   *
   * <p>You can use this method to get all forms info.</p>
   *
   * @return Returns the forms' information of all forms provided
   * @since 7
   */
  function getAllFormsInfo(callback: AsyncCallback<Array<FormInfo>>): void;
  function getAllFormsInfo(): Promise<Array<FormInfo>>;

  /**
   * get forms info by application name.
   *
   * <p>You can use this method to get all forms info of the specify application name.</p>
   *
   * @param bundleName application name.
   * @return Returns the forms' information of the specify application name.
   * @since 7
   */
  function getFormsInfoByApp(bundleName: string, callback: AsyncCallback<FormInfo>): void;
  function getFormsInfoByApp(bundleName: string): Promise<FormInfo>;

  /**
   * get forms info by application name and module name.
   *
   * <p>You can use this method to get all forms info of the specify application name and module name.</p>
   *
   * @param bundleName application name.
   * @param moduleName module name of hap
   * @return Returns the forms' information of the specify application name and module name
   * @since 7
   */
  function getFormsInfoByModule(bundleName: string, moduleName: string, callback: AsyncCallback<FormInfo>): void;
  function getFormsInfoByModule(bundleName: string, moduleName: string): Promise<FormInfo>;

  on(type: "formUninstalled", formID: number, callback: AsyncCallback<void>): void;
  off(type: "formUninstalled", formID: number, callback?: AsyncCallback<void>): void;

  on(type: "getAnimation", callback: AsyncCallback<AnimatorOption>): void;
  off(type: "getAnimation", callback?: AsyncCallback<AnimatorOption>): void;

  export enum FromParam {
    /**
     * Indicates the key specifying the ID of the form to be obtained, which is represented as
     * {@code intent.setParam(PARAM_FORM_IDENTITY_KEY, 1L)}.
     */
    IDENTITY_KEY = "ohos.extra.param.key.form_identity",
    /**
     * Indicates the form dimension, now value support 1,2,3,4.
     */
    DIMENSION_KEY = "ohos.extra.param.key.form_dimension",
    /**
     * Indicates the form name.
     */
    NAME_KEY = "ohos.extra.param.key.form_name",
    /**
     * Indicates the module name of the form.
     */
    NAME_KEY = "ohos.extra.param.key.module_name",
    /**
     * Indicates the form view width.
     */
    WIDTH_KEY = "ohos.extra.param.key.form_width",
    /**
     * Indicates the form view height.
     */
    HEIGHT_KEY = "ohos.extra.param.key.form_height",
    /**
     * Indicates the temporary flag of form to be obtained
     */
    TEMPORARY_KEY = "ohos.extra.param.key.form_temporary"
  }

  export enum FormError {
    ERR_CODE_COMMON = 1,
    ERR_PERMISSION_DENY = 2,
    ERR_GET_INFO_FAILED = 4,
    ERR_GET_BUNDLE_FAILED = 5,
    ERR_GET_LAYOUT_FAILED = 6,
    ERR_ADD_INVALID_PARAM = 7,
    ERR_CFG_NOT_MATCH_ID = 8,
    ERR_NOT_EXIST_ID = 9,
    ERR_BIND_PROVIDER_FAILED = 10,
    ERR_MAX_SYSTEM_FORMS = 11,
    ERR_MAX_INSTANCES_PER_FORM = 12,
    ERR_OPERATION_FORM_NOT_SELF = 13,
    ERR_PROVIDER_DEL_FAIL = 14,
    ERR_MAX_FORMS_PER_CLIENT = 15,
    ERR_MAX_SYSTEM_TEMP_FORMS = 16,
    ERR_FORM_NO_SUCH_MODULE = 17,
    ERR_FORM_NO_SUCH_ABILITY = 18,
    ERR_FORM_NO_SUCH_DIMENSION = 19,
    ERR_FORM_FA_NOT_INSTALLED = 20,

    // error code in sdk
    ERR_GET_FMS_RPC = 30,
    ERR_FORM_DUPLICATE_ADDED = 31,
    ERR_SEND_FMS_MSG = 32,
    ERR_GET_BMS_RPC = 33,
    ERR_SEND_BMS_MSG = 34,
    ERR_START_ABILITY = 35,
    ERR_IN_RECOVER = 36
  }
}
export default formManager;