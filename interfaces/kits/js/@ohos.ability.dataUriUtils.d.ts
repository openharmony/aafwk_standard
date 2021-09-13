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
declare namespace dataUriUtils {
  /**
   * Obtains the ID attached to the end of the path component of the given URI.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   * @param dataUri Indicates the Uri object from which the ID is to be obtained.
   * @return Returns the ID attached to the end of the path component;
   *         returns -1 if the given dataUri does not contain a path component.
   */
  function getIdSync(URI: string): number;

  /**
   * Attaches the given ID to the end of the path component of the given URI.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   * @param dataUri Indicates the Uri object to which the ID is to be attached.
   * @param id Indicates the ID to attach.
   * @return Returns the Uri object with the given ID attached.
   */
   function attachIdSync(URI: string, id: number): string;

  /**
   * Deletes the ID from the end of the path component of the given URI.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   * @param dataUri Indicates the Uri object from which the ID is to be deleted.
   * @return Returns the Uri object with the ID deleted.
   */
   function deleteIdSync(URI: string): string;

  /**
   * Updates the ID in the specified dataUri.
   * @devices phone, tablet
   * @since 7
   * @SysCap AAFwk
   * @param dataUri Indicates the Uri object to be updated.
   * @param id Indicates the new ID.
   * @return Returns the updated Uri object.
   */
   function updateIdSync(URI: string, id: number): string;
}

export default dataUriUtils;