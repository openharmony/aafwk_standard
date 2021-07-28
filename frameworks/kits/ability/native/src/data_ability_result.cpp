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

#include "data_ability_result.h"
#include <string>
#include <memory>
#include "parcel_macro.h"

namespace OHOS {
namespace AppExecFwk {  

/**
 * @brief A constructor used to create a DataAbilityResult instance 
 * with the input parameter count specified. 
 */
DataAbilityResult::DataAbilityResult(int count) : uri_("")
{
    count_ = count;
}

/**
 * @brief A constructor used to create a DataAbilityResult instance 
 * with a Parcel object specified.
 */
DataAbilityResult::DataAbilityResult(Parcel &parcel) : uri_(""), count_(0)
{
   ReadFromParcel(parcel);
}

/**
 * @brief A constructor used to create a DataAbilityResult instance 
 * with the input parameter uri specified
 */
DataAbilityResult::DataAbilityResult(const Uri &uri) : uri_(uri.ToString()), count_(0)
{}

/**
 * @brief A constructor used to create a DataAbilityResult instance 
 * with input parameters uri, count, and failure specified. 
 */
DataAbilityResult::DataAbilityResult(const Uri &uri, int count) : uri_(uri.ToString())
{
    count_ = count;
}

DataAbilityResult::~DataAbilityResult()
{}

/**
 * @brief Obtains the Uri object corresponding to the operation. 
 * @return Obtains the Uri object corresponding to the operation. 
 */
Uri DataAbilityResult::GetUri()
{
    return uri_;
}

/**
 * @brief Obtains the number of rows affected by the operation. 
 * @return Returns the number of rows affected by the operation.
 */
int DataAbilityResult::GetCount()
{
    return count_;
}

/**
 * @brief Creates a DataAbilityResult instance based on the given Parcel object. 
 * Used to transfer DataAbilityResult object using Parcel. 
 * @param parcel Indicates the Parcel object.
 * @return Returns the DataAbilityResult object.
 */
DataAbilityResult *DataAbilityResult::CreateFromParcel(Parcel &parcel)
{
    DataAbilityResult *dataAbilityResult = new (std::nothrow) DataAbilityResult(parcel);
    return dataAbilityResult;
}

/**
 * @brief Prints out a string containing the class object information. 
 * @return Returns object information.
 */
std::string DataAbilityResult::ToString()
{
    std::string stringBuilder = "DataAbilityResult(";
    stringBuilder.append("uri=").append(uri_.ToString()).append(" ");   
    stringBuilder.append("count=").append(std::to_string(count_)).append(" ");
    stringBuilder.erase(stringBuilder.length() - 1 , 1);
    stringBuilder.append(")");
    return stringBuilder;
}

/**
 * @brief Marshals a DataAbilityResult object into a Parcel. 
 * @param parcel Indicates the Parcel object for marshalling.
 * @return Returns true if the marshalling is successful; returns false otherwise.
 */
bool DataAbilityResult::Marshalling(Parcel &parcel) const
{
    //uri_
    if (uri_.ToString().empty()) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, VALUE_NULL);    
    } else {
        if (!parcel.WriteInt32(VALUE_OBJECT)) {   
            return false;
        }
        if (!parcel.WriteParcelable(&uri_)) {
            return false;
        }
    }

    //count_
    if (!parcel.WriteInt32(count_)) {  
        return false;
    }
    
    return true;
}

/**
 * @brief Unmarshals a DataAbilityResult object from a Parcel. 
 * @param parcel Indicates the Parcel object for unmarshalling.
 * @return Returns true if the unmarshalling is successful; returns false otherwise.
 */
DataAbilityResult *DataAbilityResult::Unmarshalling(Parcel &parcel)
{
    DataAbilityResult *dataAbilityResult = new (std::nothrow) DataAbilityResult(0);
    if (dataAbilityResult != nullptr ) {  
        if(!dataAbilityResult->ReadFromParcel(parcel)){
            delete dataAbilityResult;
            dataAbilityResult = nullptr;   
        }
    }

    return dataAbilityResult;
}

bool DataAbilityResult::ReadFromParcel(Parcel &parcel)
{
    //uri_
    int32_t empty = VALUE_NULL;   
    if (!parcel.ReadInt32(empty)) {
        return false;
    }

    if (empty == VALUE_OBJECT) {  
        auto uri = parcel.ReadParcelable<Uri>();
        if (uri != nullptr) {
            uri_ = *uri;
            delete uri;
            uri = nullptr;
        } else {
            return false;
        }
    }
    
    //count_
    if (!parcel.ReadInt32(count_)) {    
        return false;
    }

    return true;
}

}  // namespace AppExecFwk
}  // namespace OHOS
