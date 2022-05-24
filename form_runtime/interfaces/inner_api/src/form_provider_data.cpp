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

#include "form_provider_data.h"

#include <cinttypes>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
const std::string JSON_EMPTY_STRING = "{}";
const std::string JSON_IMAGES_STRING = "formImages";

/**
 * @brief Constructor.
 */
FormProviderData::FormProviderData()
{
    jsonFormProviderData_.clear();
}

/**
 * @brief A constructor used to create a {@code FormProviderData} instance with data of
 * the {@code nlohmann::json} type specified.
 * @param jsonData Indicates the data to be carried in the new {@code FormProviderData} instance,
 *             in {@code nlohmann::json} format.
 */
FormProviderData::FormProviderData(nlohmann::json &jsonData)
{
    jsonFormProviderData_ = jsonData;
    ParseImagesData();
}

/**
 * @brief A constructor used to create a {@code FormProviderData} instance with data of the {@code String} type
 * specified.
 * @param jsonDataString Indicates the data to be carried in the new {@code FormProviderData} instance,
 * in JSON string format.
 */
FormProviderData::FormProviderData(std::string jsonDataString)
{
    if (jsonDataString.empty()) {
        jsonDataString = JSON_EMPTY_STRING;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(jsonDataString, nullptr, false);
    if (jsonObject.is_discarded()) {
        HILOG_ERROR("failed to parse jsonDataString: %{public}s.", jsonDataString.c_str());
        return;
    }
    jsonFormProviderData_ = jsonObject;
    ParseImagesData();
}

/**
 * @brief Updates form data in this {@code FormProviderData} object.
 * @param jsonData Indicates the new data to use, in {@code ZSONObject} format.
 */
void FormProviderData::UpdateData(nlohmann::json &jsonData)
{
    jsonFormProviderData_ = jsonData;
}
/**
 * @brief Obtains the form data stored in this {@code FormProviderData} object.
 * @return Returns json data
 */
nlohmann::json FormProviderData::GetData() const
{
    return jsonFormProviderData_;
}
/**
 * @brief Obtains the form data stored in this {@code FormProviderData} object.
 * @return Returns json string format
 */
std::string FormProviderData::GetDataString() const
{
    HILOG_INFO("%{public}s called", __func__);
    std::string dataStr = jsonFormProviderData_.empty() ? "" : jsonFormProviderData_.dump();
    HILOG_INFO("%{public}s, data: %{public}s", __func__, dataStr.c_str());
    return dataStr;
}

/**
 * @brief Adds an image to this {@code FormProviderData} instance.
 * @param picName Indicates the name of the image to add.
 * @param data Indicates the binary data of the image content.
 */
void FormProviderData::AddImageData(std::string picName, char *data, int32_t size)
{
    if ((picName.length() == 0) || (sizeof(data) == 0)) {
        HILOG_ERROR("input param is NULL!");
        return;
    }

    auto iterator = rawImageBytesMap_.find(picName);
    if (iterator != rawImageBytesMap_.end()) {
        delete[] iterator->second.first;
    }
    rawImageBytesMap_[picName] = std::make_pair(data, size);

    imageDataState_ = IMAGE_DATA_STATE_ADDED;
}

/**
 * @brief Adds an image to this {@code FormProviderData} instance.
 * @param picName Indicates the name of the image to add.
 * @param data Indicates the binary data of the image content.
 */
void FormProviderData::AddImageData(std::string picName, int fd)
{
    HILOG_INFO("%{public}s called. fd is %{public}d", __func__, fd);
    if (fd < 0) {
        HILOG_ERROR("fd is invalid.");
        return;
    }

    int32_t size = lseek(fd, 0L, SEEK_END);
    if (size <= 0) {
        HILOG_ERROR("Get file size failed, errno is %{public}d", errno);
        return;
    }
    HILOG_INFO("File size is %{public}d", size);
    if (lseek(fd, 0L, SEEK_SET) == -1) {
        return;
    }
    char *data = new char[size];
    if (read(fd, data, size) < 0) {
        HILOG_ERROR("Read failed, errno is %{public}d", errno);
        return;
    }
    AddImageData(picName, data, size);
    HILOG_INFO("%{public}s called end.", __func__);
}

void FormProviderData::ParseImagesData()
{
    if (jsonFormProviderData_ == nullptr) {
        HILOG_ERROR("Form provider json data is nullptr.");
        return;
    }
    if (!jsonFormProviderData_.contains(JSON_IMAGES_STRING)) {
        return;
    }
    nlohmann::json jsonImages = jsonFormProviderData_.at(JSON_IMAGES_STRING);
    for (auto iter = jsonImages.begin(); iter != jsonImages.end(); iter++) {
        if (iter->is_number_integer()) {
            AddImageData(iter.key(), iter.value());
        } else {
            HILOG_ERROR("fd is not number integer.");
        }
    }
}

/**
 * @brief Removes data of an image with the specified {@code picName} from this {@code FormProviderData} instance.
 * @param picName Indicates the name of the image to remove.
 */
void FormProviderData::RemoveImageData(std::string picName)
{
    rawImageBytesMap_.erase(picName);
}

/**
 * @brief Set the form data stored from string string.
 * @param Returns string string.
 */
void FormProviderData::SetDataString(std::string &jsonDataString)
{
    HILOG_INFO("%{public}s called", __func__);
    if (jsonDataString.empty()) {
        jsonDataString = JSON_EMPTY_STRING;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(jsonDataString, nullptr, false);
    if (jsonObject.is_discarded()) {
        HILOG_ERROR("failed to parse jsonDataString: %{public}s.", jsonDataString.c_str());
        return;
    }
    jsonFormProviderData_ = jsonObject;
}
/**
 * @brief Merge new data to FormProviderData.
 * @param addJsonData data to merge to FormProviderData
 */
void FormProviderData::MergeData(nlohmann::json &addJsonData)
{
    HILOG_INFO("%{public}s called", __func__);
    if (addJsonData.empty()) {
        return;
    }

    if (jsonFormProviderData_.empty()) {
        jsonFormProviderData_ = addJsonData;
        return;
    }

    for (auto && [key, value] : addJsonData.items()) {
        jsonFormProviderData_[key] = value;
    }
}

/**
 * @brief Obtains the imageDataMap stored in this {@code FormProviderData} object.
 * @return Returns the map that contains shared image data.
 */
std::map<std::string, std::pair<sptr<FormAshmem>, int32_t>> FormProviderData::GetImageDataMap() const
{
    return imageDataMap_;
}

/**
 * @brief Obtains the add/remove state stored in this {@code FormProviderData} object.
 * @return Returns the add/remove state of shared image data.
 */
int32_t FormProviderData::GetImageDataState() const
{
    return imageDataState_;
}

/**
 * @brief Updates imageDataState in this {@code FormProviderData} object.
 * @param imageDataState Indicates the imageDataState to update.
 */
void FormProviderData::SetImageDataState(int32_t imageDataState)
{
    imageDataState_ = imageDataState;
}

/**
 * @brief Updates imageDataMap in this {@code FormProviderData} object.
 * @param imageDataMap Indicates the imageDataMap to update.
 */
void FormProviderData::SetImageDataMap(std::map<std::string, std::pair<sptr<FormAshmem>, int32_t>> imageDataMap)
{
    imageDataMap_ = imageDataMap;
    if (!imageDataMap.empty()) {
        imageDataState_ = IMAGE_DATA_STATE_ADDED;
    } else {
        imageDataState_ = IMAGE_DATA_STATE_NO_OPERATION;
    }
}

/**
 * Read this {@code FormProviderData} object from a Parcel.
 * @param parcel the parcel
 * eturn Returns {@code true} if the marshalling is successful; returns {@code false} otherwise.
 */
bool FormProviderData::ReadFromParcel(Parcel &parcel)
{
    auto jsonDataString = Str16ToStr8(parcel.ReadString16());
    nlohmann::json jsonObject = nlohmann::json::parse(jsonDataString, nullptr, false);
    if (jsonObject.is_discarded()) {
        HILOG_ERROR("failed to parse jsonDataString: %{public}s.", jsonDataString.c_str());
        return false;
    }
    jsonFormProviderData_ = jsonObject;

    imageDataState_ = parcel.ReadInt32();
    HILOG_INFO("%{public}s imageDateState is %{public}d", __func__, imageDataState_);
    switch (imageDataState_) {
        case IMAGE_DATA_STATE_ADDED: {
            int32_t imageDataNum = parcel.ReadInt32();
            for (int32_t i = 0; i < imageDataNum; i++) {
                sptr<FormAshmem> formAshmem = parcel.ReadParcelable<FormAshmem>();
                if (formAshmem == nullptr) {
                    HILOG_ERROR("%{public}s failed, ashmem is nullptr", __func__);
                    return false;
                }
                int32_t len = parcel.ReadInt32();
                std::pair<sptr<FormAshmem>, int32_t> imageDataRecord = std::make_pair(formAshmem, len);
                auto picName = Str16ToStr8(parcel.ReadString16());
                imageDataMap_[picName] = imageDataRecord;
            }
            break;
        }
        case IMAGE_DATA_STATE_NO_OPERATION:
        case IMAGE_DATA_STATE_REMOVED:
            break;
        default:
            HILOG_WARN("%{public}s failed, unexpected imageDataState_ %{public}d", __func__, imageDataState_);
            break;
    }
    return true;
}

/**
 * @brief Marshals this {@code FormProviderData} object into a {@link ohos.utils.Parcel} object.
 * @param parcel Indicates the {@code Parcel} object for marshalling.
 * @return Returns {@code true} if the marshalling is successful; returns {@code false} otherwise.
 */
bool FormProviderData::Marshalling(Parcel &parcel) const
{
    HILOG_INFO("%{public}s called, jsonFormProviderData_: %{public}s", __func__, jsonFormProviderData_.dump().c_str());
    if (!parcel.WriteString16(Str8ToStr16(jsonFormProviderData_.empty() ? JSON_EMPTY_STRING : jsonFormProviderData_.dump()))) {
        return false;
    }

    parcel.WriteInt32(imageDataState_);
    HILOG_INFO("%{public}s imageDateState is %{public}d", __func__, imageDataState_);
    switch (imageDataState_) {
        case IMAGE_DATA_STATE_ADDED: {
            parcel.WriteInt32(rawImageBytesMap_.size()); // firstly write the number of shared image to add
            for (auto &entry : rawImageBytesMap_) {
                if (!WriteImageDataToParcel(parcel, entry.first, entry.second.first, entry.second.second)) {
                    HILOG_ERROR("%{public}s failed, the picture name is %{public}s", __func__, entry.first.c_str());
                    return false;
                }
                parcel.WriteInt32(sizeof(entry.second));
                parcel.WriteString16(Str8ToStr16(entry.first));
            }
            break;
        }
        case IMAGE_DATA_STATE_NO_OPERATION:
        case IMAGE_DATA_STATE_REMOVED:
            break;
        default:
            HILOG_WARN("%{public}s failed, unexpected imageDataState_ %{public}d", __func__, imageDataState_);
            break;
    }
    return true;
}

/**
 * @brief Unmarshals this {@code FormProviderData} object from a {@link ohos.utils.Parcel} object.
 * @param parcel Indicates the {@code Parcel} object for unmarshalling.
 * @return FormProviderData.
 */
FormProviderData* FormProviderData::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<FormProviderData> formProviderData = std::make_unique<FormProviderData>();
    if (formProviderData && !formProviderData->ReadFromParcel(parcel)) {
        formProviderData = nullptr;
    }
    return formProviderData.release();
}

/**
 * @brief Clear imageDataMap, rawImageBytesMap_, imageDataState_ and jsonFormProviderData_.
 */
void FormProviderData::ClearData()
{
    jsonFormProviderData_.clear();
}

bool FormProviderData::WriteImageDataToParcel(Parcel &parcel, std::string picName, char *data, int32_t size) const
{
    FormAshmem formAshmem;
    if (!formAshmem.WriteToAshmem(picName, data, size)) {
        return false;
    }

    // write formAshmem
    if (!parcel.WriteParcelable(&formAshmem)) {
        HILOG_ERROR("write FormAshmem fail, the picture name is %{public}s", picName.c_str());
        return false;
    }

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
