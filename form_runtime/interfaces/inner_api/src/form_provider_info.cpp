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

#include "form_provider_info.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
bool FormProviderInfo::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<FormProviderData> bindingData(parcel.ReadParcelable<FormProviderData>());
    jsBindingData_ = *bindingData;
    return true;
}

FormProviderInfo *FormProviderInfo::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<FormProviderInfo> formProviderInfo = std::make_unique<FormProviderInfo>();
    if (formProviderInfo && !formProviderInfo->ReadFromParcel(parcel)) {
        formProviderInfo = nullptr;
    }
    return formProviderInfo.release();
}

bool FormProviderInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(&jsBindingData_)) {
        return false;
    }
    return true;
}
void FormProviderInfo::SetFormDataString(std::string &dataString)
{
    jsBindingData_.SetDataString(dataString);
}
/**
 * @brief Updates imageDataMap in this {@code FormProviderData} object.
 * @param imageDataMap Indicates the imageDataMap to update.
 */
void FormProviderInfo::SetImageDataMap(std::map<std::string, std::pair<sptr<FormAshmem>, int32_t>> imageDataMap)
{
    jsBindingData_.SetImageDataMap(imageDataMap);
}

/**
 * @brief Obtains the imageDataMap stored in this {@code FormProviderData} object.
 * @return Returns the map that contains shared image data.
 */
std::map<std::string, std::pair<sptr<FormAshmem>, int32_t>> FormProviderInfo::GetImageDataMap() const
{
    return jsBindingData_.GetImageDataMap();
}
/**
 * @brief Merge new data to FormProviderData.
 * @param addJsonData data to merge to FormProviderData
 */
void FormProviderInfo::MergeData(nlohmann::json &addJsonData)
{
    jsBindingData_.MergeData(addJsonData);
}
}  // namespace AppExecFwk
}  // namespace OHOS
