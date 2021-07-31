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

#ifndef FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_RESULT_H
#define FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_RESULT_H

#include "uri.h"     
#include "parcel.h"

using Uri = OHOS::Uri;

namespace OHOS {
namespace AppExecFwk {
class DataAbilityResult final : public Parcelable {
public:
    /**
     * @brief A constructor used to create a DataAbilityResult instance 
     * with the input parameter count specified. 
     */
    DataAbilityResult(int count);
    
    /**
     * @brief A constructor used to create a DataAbilityResult instance 
     * with the input parameter uri specified
     */    
    DataAbilityResult(const Uri &uri);
    
    /**
     * @brief A constructor used to create a DataAbilityResult instance 
     * with a Parcel object specified.
     */
    DataAbilityResult(Parcel &parcel);

    /**
     * @brief A constructor used to create a DataAbilityResult instance 
     * with input parameters uri, count, and failure specified. 
     */
    DataAbilityResult(const Uri &uri, int count);

    ~DataAbilityResult();

    /**
     * @brief Obtains the Uri object corresponding to the operation. 
     * @return Obtains the Uri object corresponding to the operation. 
     */
    Uri GetUri();

    /**
     * @brief Obtains the number of rows affected by the operation. 
     * @return Returns the number of rows affected by the operation.
     */
    int GetCount();

    /**
     * @brief Prints out a string containing the class object information. 
     * @return Returns object information.
     */
    std::string ToString();

    /**
     * @brief Marshals a DataAbilityResult object into a Parcel. 
     * @param parcel Indicates the Parcel object for marshalling.
     * @return Returns true if the marshalling is successful; returns false otherwise.
     */
    virtual bool Marshalling(Parcel &parcel) const;

    /**
     * @brief Unmarshals a DataAbilityResult object from a Parcel. 
     * @param parcel Indicates the Parcel object for unmarshalling.
     * @return Returns true if the unmarshalling is successful; returns false otherwise.
     */
    static DataAbilityResult *Unmarshalling(Parcel &parcel);

    /**
     * @brief Creates a DataAbilityResult instance based on the given Parcel object. 
     * Used to transfer DataAbilityResult object using Parcel. 
     * @param parcel Indicates the Parcel object.
     * @return Returns the DataAbilityResult object.
     */
    static DataAbilityResult *CreateFromParcel(Parcel &parcel);

private:
    Uri uri_;
    int count_;    

    bool ReadFromParcel(Parcel &parcel);
    // no object in parcel
    static constexpr int VALUE_NULL = -1;
    // object exist in parcel
    static constexpr int VALUE_OBJECT = 1;

};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_RESULT_H