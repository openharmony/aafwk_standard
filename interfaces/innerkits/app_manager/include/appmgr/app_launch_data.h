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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_APPMGR_INCLUDE_APP_LAUNCH_DATA_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_APPMGR_INCLUDE_APP_LAUNCH_DATA_H

#include <string>

#include "iremote_object.h"
#include "parcel.h"

#include "application_info.h"
#include "process_info.h"
#include "profile.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
struct UserTestRecord : public Parcelable {
    AAFwk::Want want;
    sptr<IRemoteObject> observer;
    bool isFinished;
    int32_t userId;
    UserTestRecord() : observer(nullptr), isFinished(false), userId(-1)
    {}

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static UserTestRecord *Unmarshalling(Parcel &parcel);
};

class AppLaunchData : public Parcelable {
public:
    /**
     * @brief setting information for the application.
     *
     * @param ApplicationInfo&, the current application info.
     */
    void SetApplicationInfo(const ApplicationInfo &);

    /**
     * @brief Setting information for the profile.
     *
     * @param Profile&, the current profile.
     */
    void SetProfile(const Profile &);

    /**
     * @brief Setting information for the process.
     *
     * @param Profile&, the current process info.
     */
    void SetProcessInfo(const ProcessInfo &);

    /**
     * @brief Setting id for app record.
     *
     * @param int32_t, the current app record id.
     */
    void SetRecordId(const int32_t);

    /**
     * @brief Setting id for User.
     *
     * @param int32_t, the current app User.
     */
    void SetUId(const int32_t);

    /**
     * @brief set user test info.
     *
     * @param UserTestRecord, user test info.
     */
    void SetUserTestInfo(const std::shared_ptr<UserTestRecord> &record);

    /**
     * @brief Obtains the info of the application.
     *
     * @return Returns the current application info.
     */
    inline const ApplicationInfo &GetApplicationInfo() const
    {
        return applicationInfo_;
    }

    /**
     * @brief Obtains the profile.
     *
     * @return Returns the current profile.
     */
    inline const Profile &GetProfile() const
    {
        return profile_;
    }

    /**
     * @brief Obtains the info of the process.
     *
     * @return Returns the current process info.
     */
    inline const ProcessInfo &GetProcessInfo() const
    {
        return processInfo_;
    }

    /**
     * @brief Obtains the id of the app record.
     *
     * @return Returns the current appRecord id.
     */
    inline int32_t GetRecordId() const
    {
        return recordId_;
    }

    /**
     * @brief Obtains the id of the User.
     *
     * @return Returns the current User id.
     */
    inline int32_t GetUId() const
    {
        return uId_;
    }

    /**
     * @brief get user test info.
     *
     * @return Returns user test info.
     */
    inline std::shared_ptr<UserTestRecord> GetUserTestInfo() const
    {
        return userTestRecord_;
    }

    /**
     * @brief read this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     * @return Returns true if read successed; returns false otherwise.
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * @brief Marshals this Sequenceable object into a Parcel.
     *
     * @param outParcel Indicates the Parcel object to which the Sequenceable object will be marshaled.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshals this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     */
    static AppLaunchData *Unmarshalling(Parcel &parcel);

private:
    ApplicationInfo applicationInfo_;
    Profile profile_;
    ProcessInfo processInfo_;
    int32_t recordId_ = 0;
    int32_t uId_ = 0;
    std::shared_ptr<UserTestRecord> userTestRecord_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_APPMGR_INCLUDE_APP_LAUNCH_DATA_H
