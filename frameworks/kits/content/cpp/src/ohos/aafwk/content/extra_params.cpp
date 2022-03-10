
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
#include "extra_params.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief A default constructor used to create an empty {@code ExtraParams} instance.
 * @param none
 */
ExtraParams::ExtraParams()
{}
/**
 * @brief A copy constructor used to create an empty {@code ExtraParams} instance.
 * @param other indicates object instance.
 */
ExtraParams::ExtraParams(const ExtraParams &other)
{
    devType_ = other.devType_;
    targetBundleName_ = other.targetBundleName_;
    description_ = other.description_;
    jsonParams_ = other.jsonParams_;
}
/**
 * @brief Overloading assignment operators to create the same object instance.
 * @param other Indicates object instance.
 */
ExtraParams &ExtraParams::operator=(const ExtraParams &other)
{
    if (this != &other) {
        devType_ = other.devType_;
        targetBundleName_ = other.targetBundleName_;
        description_ = other.description_;
        jsonParams_ = other.jsonParams_;
    }
    return *this;
}
/**
 * @brief Judge whether the parameter of extraparam instance is equal to that of other instance parameter
 * @param other Indicates object instance.
 * @return returns true the parameter of extraparam instance is equal to that of other instance parameter
 * otherwise false
 */
bool ExtraParams::operator==(const ExtraParams &other) const
{
    if (targetBundleName_ == other.targetBundleName_ && description_ == other.description_ &&
        jsonParams_ == other.jsonParams_) {

        if (devType_.size() != other.devType_.size()) {
            return false;
        } else if (devType_.size() > 0 && other.devType_.size() > 0 && devType_.size() == other.devType_.size()) {
            std::vector<string>::const_iterator it1;
            for (it1 = devType_.cbegin(); it1 != devType_.cend(); it1++) {
                std::vector<string>::const_iterator it2;
                it2 = std::find(other.devType_.cbegin(), other.devType_.cend(), *it1);
                if (it2 == other.devType_.cend()) {
                    return false;
                }
            }
            return true;
        }
        return true;
    }
    return false;
}
/**
 * @brief A constructor used to create an {@code ExtraParams} instance based on the input parameters{@code devType},
 * {@code targetBundleName}, and {@code description}.
 *
 * @param devType Indicates the type of devices to be matched. This parameter can be any combination of
 * {@link #DEVICETYPE_SMART_PHONE}, {@link #DEVICETYPE_SMART_PAD}, {@link #DEVICETYPE_SMART_WATCH}, and
 * {@link #DEVICETYPE_SMART_TV}.
 *
 * @param targetBundleName Indicates the bundle name of the target application where the ability will be migrated.
 *
 * @param description Indicates the description used for device filtering.
 *
 * @param jsonParams Indicates the custom JSON parameters to be used as filter conditions.
 *
 * @return none
 */
ExtraParams::ExtraParams(const std::vector<string> &devType, const string &targetBundleName, const string &description,
    const string &jsonParams)
{
    devType_ = devType;
    targetBundleName_ = targetBundleName;
    description_ = description;
    jsonParams_ = jsonParams;
}

/**
 * @brief A destructor release an empty {@code ExtraParams} instance.
 * @param none
 */
ExtraParams::~ExtraParams()
{}

/**
 * @brief Sets the list of device types.
 *
 * @param devType Indicates the type of devices to be matched. This parameter can be any combination of
 * {@link #DEVICETYPE_SMART_PHONE}, {@link #DEVICETYPE_SMART_PAD}, {@link #DEVICETYPE_SMART_WATCH}, and
 * {@link #DEVICETYPE_SMART_TV}.
 *
 * @return none
 */
void ExtraParams::SetDevType(const std::vector<string> &devType)
{
    devType_ = devType;
}

/**
 * @brief Obtains the list of device types.
 *
 * @param none
 *
 * @return Returns the list of device types.
 */
std::vector<string> ExtraParams::GetDevType() const
{
    return devType_;
}

/**
 * @brief Sets the bundle name of the target application where ability will be migrated.
 *
 * @param targetBundleName Indicates the bundle name of the target application to set.
 *
 * @return none
 */
void ExtraParams::SetTargetBundleName(const string &targetBundleName)
{
    targetBundleName_ = targetBundleName;
}

/**
 * @brief Obtains the bundle name of the target application where the ability will be migrated.
 *
 * @return Returns the bundle name of the target application.
 */
string ExtraParams::GetTargetBundleName() const
{
    return targetBundleName_;
}
/**
 * @brief Sets the description used for device filtering.
 *
 * @param jsonParams Indicates the device description to set.
 *
 * @return none
 */
void ExtraParams::SetJsonParams(const string &jsonParams)
{
    jsonParams_ = jsonParams;
}

/**
 * @brief Obtains the custom JSON parameters used as filter conditions.
 *
 * @param none
 *
 * @return Returns the custom JSON parameters.
 */
string ExtraParams::GetJsonParams() const
{
    return jsonParams_;
}

/**
 * @brief Sets the custom JSON parameters to be used as filter conditions.
 *
 * @param description Indicates the custom JSON parameters to set.
 *
 */
void ExtraParams::SetDescription(const string &description)
{
    description_ = description;
}

/**
 * @brief Obtains the description used for device filtering.
 *
 * @param none
 *
 * @return Returns the description used for device filtering.
 */
string ExtraParams::GetDescription() const
{
    return description_;
}

/**
 * @brief Marshals this {@code ExtraParams} object into a {@link ohos.utils.Parcel} object.
 *
 * @param parcel Indicates the {@code Parcel} object for marshalling.
 *
 * @return Returns {@code true} if the marshalling is successful; returns {@code false} otherwise.
 */
bool ExtraParams::Marshalling(Parcel &parcel) const
{
    bool ret = true;
    // devType
    bool ret1 = parcel.WriteStringVector(devType_);

    // targetBundleName
    bool ret2 = parcel.WriteString16(Str8ToStr16(targetBundleName_));

    // description
    bool ret3 = parcel.WriteString16(Str8ToStr16(description_));

    // jsonParams
    bool ret4 = parcel.WriteString16(Str8ToStr16(jsonParams_));

    ret = (ret1 && ret2 && ret3 && ret4) ? true : false;
    return ret;
}

/**
 * @brief Unmarshals this {@code ExtraParams} object from a {@link ohos.utils.Parcel} object.
 *
 * @param parcel Indicates the {@code Parcel} object for unmarshalling.
 *
 * @return Returns {@code true} if the unmarshalling is successful; returns {@code false} otherwise.
 */
ExtraParams *ExtraParams::Unmarshalling(Parcel &parcel)
{
    // devType.
    std::vector<string> devtype;
    parcel.ReadStringVector(&devtype);

    // targetBundleName
    string targetBundleName = Str16ToStr8(parcel.ReadString16());

    // description
    string description = Str16ToStr8(parcel.ReadString16());

    // jsonParams
    string jsonParams = Str16ToStr8(parcel.ReadString16());

    ExtraParams *extraParams = new (std::nothrow) ExtraParams(devtype, targetBundleName, description, jsonParams);

    if (extraParams == nullptr) {
        return nullptr;
    }
    return extraParams;
}
}  // namespace AppExecFwk
}  // namespace OHOS