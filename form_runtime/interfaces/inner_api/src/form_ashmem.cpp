/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "form_ashmem.h"

#include "hilog_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AppExecFwk {
FormAshmem::~FormAshmem()
{
    if (ashmem_ != nullptr) {
        ashmem_->CloseAshmem();
    }
}

bool FormAshmem::Marshalling(Parcel &parcel) const
{
    MessageParcel* messageParcel = (MessageParcel*)&parcel;
    if (!messageParcel->WriteAshmem(ashmem_)) {
        return false;
    }
    return true;
}

bool FormAshmem::ReadFromParcel(Parcel &parcel)
{
    MessageParcel* messageParcel = (MessageParcel*)&parcel;
    ashmem_ = messageParcel->ReadAshmem();
    if (ashmem_ == nullptr) {
        return false;
    }
    HILOG_INFO("fd: %{public}d, size: %{public}d", ashmem_->GetAshmemFd(), ashmem_->GetAshmemSize());
    return true;
}

FormAshmem* FormAshmem::Unmarshalling(Parcel &parcel)
{
    FormAshmem* formAshmem = new FormAshmem();
    if (formAshmem != nullptr && !formAshmem->ReadFromParcel(parcel)) {
        delete formAshmem;
        formAshmem = nullptr;
    }
    return formAshmem;
}

bool FormAshmem::WriteToAshmem(std::string name, char *data, int32_t size)
{
    if (size <= 0) {
        HILOG_ERROR("%{public}s: Parameter is invalid, size= %{public}d", __func__, size);
    }

    ashmem_ = Ashmem::CreateAshmem(name.c_str(), size);
    if (ashmem_ == nullptr) {
        HILOG_ERROR("create shared memory fail");
        return false;
    }

    bool ret = ashmem_->MapReadAndWriteAshmem();
    if (!ret) {
        HILOG_ERROR("map shared memory fail");
        return false;
    }

    ret = ashmem_->WriteToAshmem(data, size, 0);
    if (!ret) {
        ashmem_->UnmapAshmem();
        HILOG_ERROR("write image data to shared memory fail");
        return false;
    }

    ashmem_->UnmapAshmem();
    return true;
}

int32_t FormAshmem::GetAshmemSize()
{
    if (ashmem_ == nullptr) {
        return 0;
    }
    return ashmem_->GetAshmemSize();
}

int32_t FormAshmem::GetAshmemFd()
{
    if (ashmem_ == nullptr) {
        return -1;
    }
    return ashmem_->GetAshmemFd();
}
}  // namespace AppExecFwk
}  // namespace OHOS
