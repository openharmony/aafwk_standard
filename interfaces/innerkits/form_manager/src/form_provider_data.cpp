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

#include <cinttypes>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "form_provider_data.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
const std::string JSON_EMPTY_STRING = "{}";

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
    jsonFormProviderData_ = nlohmann::json::parse(jsonDataString);
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
    HILOG_INFO("%{public}s called", __func__);
    if (fd <= 0) {
        return;
    }
    std::string fdPath = "/proc/self/fd/" + std::to_string(fd);
    char fileName[PATH_MAX + 1] = {0};
    int ret = readlink(fdPath.c_str(), fileName, PATH_MAX);
    if (ret < 0 || ret > PATH_MAX) {
        HILOG_ERROR("Get fileName failed, ret is: %{public}d!", ret);
        return;
    }
    fileName[ret] = '\0';

    std::ifstream file(fileName, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        HILOG_INFO("open failed");
        return;
    }

    file.seekg(0, std::ios::end);
    int32_t size = file.tellg();
    HILOG_INFO("File size %{public}d", size);
    if (size < 0) {
        return;
    }
    file.seekg(0, std::ios::beg);
    char *data = new char[size];
    file.read(data, size);
    file.close();
    AddImageData(picName, data, size);
    HILOG_INFO("%{public}s called end.", __func__);
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
    jsonFormProviderData_ = nlohmann::json::parse(jsonDataString);
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
std::map<std::string, std::pair<sptr<Ashmem>, int32_t>> FormProviderData::GetImageDataMap() const
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
void FormProviderData::SetImageDataMap(std::map<std::string, std::pair<sptr<Ashmem>, int32_t>> imageDataMap)
{
    imageDataMap_ = imageDataMap;
}

/**
 * Read this {@code FormProviderData} object from a Parcel.
 * @param parcel the parcel
 * eturn Returns {@code true} if the marshalling is successful; returns {@code false} otherwise.
 */
bool FormProviderData::ReadFromParcel(Parcel &parcel)
{
    jsonFormProviderData_ = nlohmann::json::parse(Str16ToStr8(parcel.ReadString16()));

    imageDataState_ = parcel.ReadInt32();
    HILOG_INFO("%{public}s imageDateState is %{public}d", __func__, imageDataState_);
    switch (imageDataState_) {
        case IMAGE_DATA_STATE_ADDED: {
            int32_t imageDataNum = parcel.ReadInt32();
            for (int32_t i = 0; i < imageDataNum; i++) {
                MessageParcel* messageParcel = (MessageParcel*)&parcel;
                sptr<Ashmem> ashmem = messageParcel->ReadAshmem();
                if (ashmem == nullptr) {
                    HILOG_ERROR("%{public}s failed, ashmem is nullptr", __func__);
                    return false;
                }

                int32_t len = parcel.ReadInt32();
                std::pair<sptr<Ashmem>, int32_t> imageDataRecord = std::make_pair(ashmem, len);
                imageDataMap_.emplace(Str16ToStr8(parcel.ReadString16()), imageDataRecord);
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
    sptr<Ashmem> ashmem = Ashmem::CreateAshmem(picName.c_str(), size);
    if (ashmem == nullptr) {
        HILOG_ERROR("create shared memory fail");
        return false;
    }

    bool ret = ashmem->MapReadAndWriteAshmem();
    if (!ret) {
        HILOG_ERROR("map shared memory fail");
        return false;
    }

    ret = ashmem->WriteToAshmem(data, size, 0);
    if (!ret) {
        HILOG_ERROR("write image data to shared memory fail");
        return false;
    }

    ashmem->UnmapAshmem();

    MessageParcel* messageParcel = (MessageParcel*)&parcel;
    ret = messageParcel->WriteAshmem(ashmem);

    ashmem->CloseAshmem(); // close ashmem after writeAshmem because writeAshmem will dup fd
    if (!ret) {
        HILOG_ERROR("writeAshmem fail, the picture name is %{public}s", picName.c_str());
        return false;
    }

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
