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

#include "app_launch_data.h"

#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
void AppLaunchData::SetApplicationInfo(const ApplicationInfo &info)
{
    applicationInfo_ = info;
}

void AppLaunchData::SetProfile(const Profile &profile)
{
    profile_ = profile;
}

void AppLaunchData::SetProcessInfo(const ProcessInfo &info)
{
    processInfo_ = info;
}

void AppLaunchData::SetRecordId(const int32_t recordId)
{
    recordId_ = recordId;
}

void AppLaunchData::SetUId(const int32_t uId)
{
    uId_ = uId;
}

void AppLaunchData::SetUserTestInfo(const UserTestRecord &record)
{
    userTestRecord_ = record;
}

bool AppLaunchData::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteParcelable(&applicationInfo_) && parcel.WriteParcelable(&profile_) &&
            parcel.WriteParcelable(&processInfo_) && parcel.WriteInt32(recordId_) &&
            parcel.WriteInt32(uId_) && parcel.WriteParcelable(&userTestRecord_));
}

bool AppLaunchData::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<ApplicationInfo> applicationInfoRead(parcel.ReadParcelable<ApplicationInfo>());
    if (!applicationInfoRead) {
        APP_LOGE("failed, applicationInfoRead is nullptr");
        return false;
    }
    applicationInfo_ = *applicationInfoRead;

    std::unique_ptr<Profile> profileRead(parcel.ReadParcelable<Profile>());
    if (!profileRead) {
        APP_LOGE("failed, profileRead is nullptr");
        return false;
    }
    profile_ = *profileRead;

    std::unique_ptr<ProcessInfo> processInfoRead(parcel.ReadParcelable<ProcessInfo>());
    if (!processInfoRead) {
        APP_LOGE("failed, processInfoRead is nullptr");
        return false;
    }
    processInfo_ = *processInfoRead;

    recordId_ = parcel.ReadInt32();
    uId_ = parcel.ReadInt32();

    std::unique_ptr<UserTestRecord> userTestRecord(parcel.ReadParcelable<UserTestRecord>());
    if (!userTestRecord) {
        APP_LOGE("failed, userTestRecord is nullptr");
        return false;
    }
    userTestRecord_ = *userTestRecord;
    return true;
}

AppLaunchData *AppLaunchData::Unmarshalling(Parcel &parcel)
{
    AppLaunchData *appLaunchData = new (std::nothrow) AppLaunchData();
    if (appLaunchData && !appLaunchData->ReadFromParcel(parcel)) {
        APP_LOGW("failed, because ReadFromParcel failed");
        delete appLaunchData;
        appLaunchData = nullptr;
    }
    return appLaunchData;
}

bool UserTestRecord::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(&want)) {
        APP_LOGE("Failed to write want");
        return false;
    }

    auto valid = observer ? true : false;
    if (!parcel.WriteBool(valid)) {
        APP_LOGE("Failed to write the flag which indicate whether observer is null");
        return false;
    }

    if (valid) {
        if (!parcel.WriteParcelable(observer)) {
            APP_LOGE("Failed to write observer");
            return false;
        }
    }
    return true;
}

UserTestRecord *UserTestRecord::Unmarshalling(Parcel &parcel)
{
    UserTestRecord *userTestRecord = new (std::nothrow) UserTestRecord();
    if (userTestRecord && !userTestRecord->ReadFromParcel(parcel)) {
        APP_LOGW("failed, because ReadFromParcel failed");
        delete userTestRecord;
        userTestRecord = nullptr;
    }
    return userTestRecord;
}

bool UserTestRecord::ReadFromParcel(Parcel &parcel)
{
    AAFwk::Want *wantPtr = parcel.ReadParcelable<AAFwk::Want>();
    if (wantPtr == nullptr) {
        APP_LOGE("wantPtr is nullptr");
        return ERR_INVALID_VALUE;
    }
    want = *wantPtr;
    delete wantPtr;

    auto valid = parcel.ReadBool();
    if (valid) {
        observer = parcel.ReadParcelable<IRemoteObject>();
        if (!observer) {
            APP_LOGE("observer is nullptr");
            return false;
        }
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
