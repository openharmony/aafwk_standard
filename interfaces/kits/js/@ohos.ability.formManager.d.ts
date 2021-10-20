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
import { AsyncCallback } from './basic';
import { Want } from './ability/want';

/**
 * Client to communication with FormManagerService.
 * @name formManager
 * @since 7
 * @sysCap AAFwk
 * @devices phone, tablet
 * @import import formManager from '@ohos.ability.formManager'
 */
declare namespace formManager {
  /**
   * Requests for form data update.
   * <p>This method must be called when the application has detected that a system setting item (such as the language,
   * resolution, or screen orientation) being listened for has changed. Upon receiving the update request, the form
   * provider automatically updates the form data (if there is any update) through the form framework, with the update
   * process being unperceivable by the application.</p>
   * <p><b>Permission: </b>ohos.permission.REQUIRE_FORM</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param formId Indicates the ID of the form to update.
   * @return -
   * @systemapi hide for inner use.
   */
  function requestForm(formId: string, callback: AsyncCallback<void>): void;
  function requestForm(formId: string): Promise<void>;

  /**
   * Deletes an obtained form by its ID.
   *
   * <p>After this method is called, the form won't be available for use by the application and the Form Manager
   * Service no longer keeps the cache information about the form.</p>
   * <p><b>Permission: </b>ohos.permission.REQUIRE_FORM</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param formId Indicates the form ID of the form to delete.
   * @return -
   * @systemapi hide for inner use.
   */
  function deleteForm(formId: string, callback: AsyncCallback<void>): void;
  function deleteForm(formId: string): Promise<void>;

  /**
   * Releases an obtained form by its ID.
   *
   * <p>After this method is called, the form won't be available for use by the application, but the Form Manager
   * Service still keeps the cache information about the form, so that the application can quickly obtain it based on
   * the {@code formId}.</p>
   * <p><b>Permission: </b>ohos.permission.REQUIRE_FORM</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param formId Indicates the form ID of the form to release.
   * @return -
   * @systemapi hide for inner use.
   */
  function releaseForm(formId: string, callback: AsyncCallback<void>): void;
  function releaseForm(formId: string, isReleaseCache: boolean, callback: AsyncCallback<void>): void;
  function releaseForm(formId: string, isReleaseCache?: boolean): Promise<void>;

  /**
   * Sends a notification to the form framework to make the specified forms visible.
   *
   * <p>After this method is successfully called, Ability.onEventNotify(Map<Long, int>)
   * will be called to notify the form provider of the form visibility change event.</p>
   * <p><b>Permissions: </b>ohos.permission.REQUIRE_FORM</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param formIds Indicates the IDs of the forms to be made visible.
   * @return -
   * @systemapi hide for inner use.
   */
  function notifyVisibleForms(formIds: Array<string>, callback: AsyncCallback<void>): void;
  function notifyVisibleForms(formIds: Array<string>): Promise<void>;

  /**
   * Sends a notification to the form framework to make the specified forms invisible.
   *
   * <p>After this method is successfully called, Ability.onEventNotify(Map<Long, int>)
   * will be called to notify the form provider of the form visibility change event.</p>
   * <p><b>Permissions: </b>ohos.permission.REQUIRE_FORM</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param formIds Indicates the IDs of the forms to be made invisible.
   * @return -
   * @systemapi hide for inner use.
   */
  function notifyInvisibleForms(formIds: Array<string>, callback: AsyncCallback<void>): void;
  function notifyInvisibleForms(formIds: Array<string>): Promise<void>;

  /**
   * set form refresh state to true.
   *
   * <p>You can use this method to set form refresh state to true, the form can receive new
   * update from service.</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param formIds the specify form id.
   * @return -
   * @systemapi hide for inner use.
   */
  function enableFormsUpdate(formIds: Array<string>, callback: AsyncCallback<void>): void;
  function enableFormsUpdate(formIds: Array<string>): Promise<void>;

  /**
   * set form refresh state to false.
   *
   * <p>You can use this method to set form refresh state to false, the form do not receive
   * new update from service.</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param formIds the specify form id.
   * @return -
   * @systemapi hide for inner use.
   */
  function disableFormsUpdate(formIds: Array<string>, callback: AsyncCallback<void>): void;
  function disableFormsUpdate(formIds: Array<string>): Promise<void>;

  /**
   * obtain the system is ready.
   *
   * <p>You can use this method to obtain the system is ready
   * new update from service.</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param -
   * @return - is system ready
   * @systemapi hide for inner use.
   */
  function isSystemReady(callback: AsyncCallback<boolean>): void;
  function isSystemReady(): Promise<boolean>;

  /**
   * cast a temp form to normal form.
   *
   * <p>You can use this method to cast a temp form to normal form
   * new update from service.</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param formId the specify form id.
   * @return -
   * @systemapi hide for inner use.
   */
  function castTempForm(formId: string, callback: AsyncCallback<void>): void;
  function castTempForm(formId: string): Promise<void>;

  /**
   * delete invalid forms of the application in the Form Manager Service based on the list of.
   *
   * <p>You can use this method to delete invalid forms of the application
   * new update from service.</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param formIds the specify form id.
   * @return - the number of invalid forms deleted by the Form Manager Service
   * @systemapi hide for inner use.
   */
  function deleteInvalidForms(formIds: Array<string>, callback: AsyncCallback<number>): void;
  function deleteInvalidForms(formIds: Array<string>): Promise<number>;

  /**
   * obtains the Form state.
   *
   * <p>You can use this method to obtains the form state
   * new update from service.</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param want of the form.
   * @return - form state
   * @systemapi hide for inner use.
   */
  function acquireFormState(want: Want, callback: AsyncCallback<FormStateInfo>): void;
  function acquireFormState(want: Want): Promise<FormStateInfo>;

  /**
   * listen to the event of uninstall form.
   *
   * <p>You can use this method to listen to the event of uninstall form
   * new update from service.</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param event type.
   * @return - formIds of the uninstalled form.
   * @systemapi hide for inner use.
   */
  function on(type: "formUninstall", callback: AsyncCallback<void>): void;

  /**
   * cancel listening to the event of uninstall form.
   *
   * <p>You can use this method to cancel listening to the event of uninstall form
   * new update from service.</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param event type.
   * @return -
   * @systemapi hide for inner use.
   */
  function off(type: "formUninstall", callback?: AsyncCallback<void>): void;

  /**
   * Obtains the FormInfo objects provided by all ohos applications on the device.
   *
   * <p><b>Permission: </b>ohos.permission.GET_BUNDLE_INFO_PRIVILEGED</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @return -
   * @systemapi hide for inner use.
   */
  function getAllFormsInfo(callback: AsyncCallback<Array<FormInfo>>): void;
  function getAllFormsInfo(): Promise<Array<FormInfo>>;

  /**
   * Obtains the FormInfo objects provided by a specified application module on the device.
   *
   * <p><b>Permission: </b>ohos.permission.GET_BUNDLE_INFO_PRIVILEGED</p>
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param bundleName Indicates the bundle name of the ohos application. Specifically, it is the value of the
   * {@code bundleName} attribute defined in the {@code app} tag of the <b>config.json</b> file for the Harmony
   * application.
   * @param moduleName Indicates the module name of the ohos application. Specifically, it is the value of the
   * {@code moduleName} sub-attribute of the {@code distro} attribute defined in the {@code module} tag of the
   * <b>config.json</b> file for the Harmony application. Optional parameter, if not set, search is performed based on
   * bundleName only.
   * @return -
   * @systemapi hide for inner use.
   */
  function getFormsInfo(bundleName: string, callback: AsyncCallback<Array<FormInfo>>): void;
  function getFormsInfo(bundleName: string, moduleName: string, callback: AsyncCallback<Array<FormInfo>>): void;
  function getFormsInfo(bundleName: string, moduleName?: string): Promise<Array<FormInfo>>;

  /**
  * Provides information about a form.
  * @name FormInfo
  * @since 7
  */
  interface FormInfo {
    /**
     * Obtains the bundle name of the application to which this form belongs.
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     */
    bundleName: string;

    /**
     * Obtains the name of the application module to which this form belongs.
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     */
    moduleName: string;

    /**
     * Obtains the class name of the ability to which this form belongs.
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     */
    abilityName: string;

    /**
     * Obtains the name of this form.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    name: string;

    /**
     * Obtains the description of this form.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    description?: string;

    /**
     * Obtains the type of this form. Currently, Java and JS forms are supported.
     * a constant value, one of [FormType.JAVA, FormType.JS].
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    type: FormType;

    /**
     * Obtains the JS component name of this JS form.
     * <p>This method is valid only when the type of this form is {@code JS}.</p>
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    jsComponentName: string;

    /**
     * Obtains the color mode of this form.
     * A constant value, one of ColorMode.MODE_AUTO, ColorMode.MODE_DARK, ColorMode.MODE_LIGHT.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    colorMode: ColorMode;

    /**
     * Checks whether this form is a default form.
     * <p>A form is considered a default form if no form name is specified. Specifically, the system determines
     * whether a form is a default form based on the value of the {@code isDefault} attribute defined under
     * {@code forms} in the <b>config.json</b> file of the Harmony application.</p>
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    isDefault: boolean;

    /**
     * Obtains the updateEnabled.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     * @systemapi hide for inner use.
     */
    updateEnabled: boolean;

    /**
     * Obtains whether notify visible of this form.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    formVisibleNotify: boolean;

    /**
     * Obtains the bundle relatedBundleName of the application to which this form belongs.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     * @systemapi hide for inner use.
     */
    relatedBundleName: string;

    /**
     * Obtains the scheduledUpdateTime.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     * @systemapi hide for inner use
     */
    scheduledUpdateTime: string;

    /**
     * Obtains the form config ability about this form.
     * <p>A form config ability is a user-defined ability.</p>
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    formConfigAbility: string;

    /**
     * Obtains the updateDuration.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     * @systemapi hide for inner use
     */
    updateDuration: number;

    /**
     * Obtains the default grid style of this form.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    defaultDimension: number;

    /**
     * Obtains the grid styles supported by this form.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    supportDimensions: Array<number>;

    /**
     * Obtains the custom data defined in this form.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    customizeData: {[key: string]: [value: string]};

    /**
     * Indicates the compatibility object.
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @default -
     */
    jsonObject: object;
  }

    /**
    * Provides state information about a form.
    * @name FormStateInfo
    * @since 7
    */
  interface FormStateInfo {
    /**
     * Obtains the form state.
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     */
    formState: FormState;

    /**
     * Obtains the want form .
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     */
    want: Want;
  }

  /**
  * Provides state about a form.
  * @name FormState
  * @since 7
  */
  enum FormState {
      /**
       * Indicates that the form status is unknown due to an internal error.
       *
       * @since 7
       */
      UNKNOWN = -1,
      /**
       * Indicates that the form is in the default state.
       *
       * @since 7
       */
      DEFAULT = 0,
      /**
       * Indicates that the form is ready.
       *
       * @since 7
       */
      READY = 1,
  }

  enum FormType {
    /**
     * Java form.
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     */
    JAVA = 0,

    /**
     * Js form.
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     */
    JS = 1
  }

  enum FormParam {
    /**
     * Indicates the key specifying the ID of the form to be obtained, which is represented as
     * want: {
     *   "parameters": {
     *       IDENTITY_KEY: 1L
     *    }
     * }.
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @systemapi hide for inner use.
     */
    IDENTITY_KEY = "ohos.extra.param.key.form_identity",

    /**
     * Indicates the key specifying the grid style of the form to be obtained, which is represented as
     * want: {
     *   "parameters": {
     *       DIMENSION_KEY: 1
     *    }
     * }.
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @systemapi hide for inner use.
     */
    DIMENSION_KEY = "ohos.extra.param.key.form_dimension",

    /**
     * Indicates the key specifying the name of the form to be obtained, which is represented as
     * want: {
     *   "parameters": {
     *       NAME_KEY: "formName"
     *    }
     * }.
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @systemapi hide for inner use.
     */
    NAME_KEY = "ohos.extra.param.key.form_name",

    /**
     * Indicates the key specifying the name of the module to which the form to be obtained belongs, which is
     * represented as
     * want: {
     *   "parameters": {
     *       MODULE_NAME_KEY: "formEntry"
     *    }
     * }
     * This constant is mandatory.
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @systemapi hide for inner use.
     */
    MODULE_NAME_KEY = "ohos.extra.param.key.module_name",

    /**
     * Indicates the key specifying the width of the form to be obtained, which is represented as
     * want: {
     *   "parameters": {
     *       WIDTH_KEY: 800
     *    }
     * }
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @systemapi hide for inner use.
     */
    WIDTH_KEY = "ohos.extra.param.key.form_width",

    /**
     * Indicates the key specifying the height of the form to be obtained, which is represented as
     * want: {
     *   "parameters": {
     *       HEIGHT_KEY: 400
     *    }
     * }
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @systemapi hide for inner use.
     */
    HEIGHT_KEY = "ohos.extra.param.key.form_height",

    /**
     * Indicates the key specifying whether a form is temporary, which is represented as
     * want: {
     *   "parameters": {
     *       TEMPORARY_KEY: true
     *    }
     * }
     * @default -
     * @devices phone, tablet
     * @since 7
     * @sysCap AAFwk
     * @systemapi hide for inner use.
     */
    TEMPORARY_KEY = "ohos.extra.param.key.form_temporary"
  }

  enum FormError {
    /**
     * A common internal error occurs during form processing.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_COMMON = 1,

    /**
     * The application does not have permission to use forms.
     * Ensure that the application is granted with the ohos.permission.REQUIRE_FORM
     * and ohos.permission.GET_BUNDLE_INFO_PRIVILEGED permissions.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_PERMISSION_DENY = 2,

    /**
     * Failed to obtain the configuration information about the form specified by the
     * request parameters. Ensure that the parameters of the form to be added are
     * consistent with those provided by the form provider.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_GET_INFO_FAILED = 4,

    /**
     * Failed to obtain the bundle to which the form belongs based on the request parameters.
     * Ensure that the bundle to which the form to be added belongs is available.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_GET_BUNDLE_FAILED = 5,

    /**
     * Failed to initialize the form layout based on the request parameters.
     * Ensure that the grid style of the form is supported by the form provider.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_GET_LAYOUT_FAILED = 6,

    /**
     * Invalid input parameter during form operation. Ensure that all input
     * parameters are valid.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_ADD_INVALID_PARAM = 7,

    /**
     * The form configuration to be obtained using an existing form ID is
     * different from that obtained for the first time.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_CFG_NOT_MATCH_ID = 8,

    /**
     * The ID of the form to be operated does not exist in the Form Manager Service.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_NOT_EXIST_ID = 9,

    /**
     * Failed to bind the Form Manager Service to the provider service.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_BIND_PROVIDER_FAILED = 10,

    /**
     * The total number of added forms exceeds the maximum allowed by the system.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_MAX_SYSTEM_FORMS = 11,

    /**
     * The number of form instances generated using the same form configuration
     * exceeds the maximum allowed by the system.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_MAX_INSTANCES_PER_FORM = 12,

    /**
     * The form being requested was added by other applications and cannot be
     * operated by the current application.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_OPERATION_FORM_NOT_SELF = 13,

    /**
     * The Form Manager Service failed to instruct the form provider to delete the form.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_PROVIDER_DEL_FAIL = 14,

    /**
     * The total number of added forms exceeds the maximum per client.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_MAX_FORMS_PER_CLIENT = 15,

    /**
     * The total number of added temp forms exceeds the maximum in system.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_MAX_SYSTEM_TEMP_FORMS = 16,

    /**
     * The module can not be find in system.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_FORM_NO_SUCH_MODULE = 17,

    /**
     * The ability can not be find in system.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_FORM_NO_SUCH_ABILITY = 18,

    /**
     * The dimension is not exist in the form.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_FORM_NO_SUCH_DIMENSION = 19,

    /**
     * The ability is not installed.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_FORM_FA_NOT_INSTALLED = 20,

    /**
     * Failed to obtain the RPC object of the Form Manager Service because
     * the service is not started.Please try again after the service is started.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_SYSTEM_RESPONSES_FAILED = 30,

    /**
     * Failed to obtain the form requested by the client because another form
     * with the same form ID is in use. Forms in use cannot have the same ID.
     * To obtain and display a form that has the same configuration as an in-use
     * form in the same application, you are advised to set the form ID to 0 in
     * the request parameters.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_FORM_DUPLICATE_ADDED = 31,

    /**
     * The form is being restored. Perform operations on the form only after
     * the restoration is complete.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     */
    ERR_IN_RECOVERY = 36
  }

  enum ColorMode {
    /**
     * Automatic mode.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     * @systemapi hide for inner use.
     */
    MODE_AUTO = -1,

    /**
     * Dark mode.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     * @systemapi hide for inner use.
     */
     MODE_DARK = 0,

    /**
     * Light mode.
     * @since 7
     * @sysCap AAFwk
     * @devices phone, tablet
     * @systemapi hide for inner use.
     */
    MODE_LIGHT = 1
  }
}
export default formManager;