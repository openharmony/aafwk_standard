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

#ifndef OHOS_AAFWK_INTERFACES_INNERKITS_CONFIGURATION_H
#define OHOS_AAFWK_INTERFACES_INNERKITS_CONFIGURATION_H

#include <string>

#include "nocopyable.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {

class DummyConfiguration : public Parcelable {
public:
    DummyConfiguration() = default;
    explicit DummyConfiguration(const std::string &name);
    virtual ~DummyConfiguration() = default;

    /**
     * @brief Obtains the name of the Configuration.
     *
     * @return Returns the Configuration name.
     */
    inline const std::string &GetName() const
    {
        return testInfostr_;
    }

    unsigned int Differ(const std::shared_ptr<DummyConfiguration> config) const;

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
    static DummyConfiguration *Unmarshalling(Parcel &parcel);

private:
    std::string testInfostr_;
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_INTERFACES_INNERKITS_CONFIGURATION_H