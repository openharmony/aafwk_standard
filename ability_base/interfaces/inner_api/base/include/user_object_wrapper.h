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
#ifndef OHOS_AAFWK_USER_OBJECT_WRAPPER_H
#define OHOS_AAFWK_USER_OBJECT_WRAPPER_H
#include <memory>
#include <functional>
#include "base_object.h"
#include "refbase.h"
#include "user_object_base.h"

namespace OHOS {
namespace AAFwk {

class UserObject final : public Object, public IUserObject {
public:
    inline UserObject(const std::shared_ptr<UserObjectBase> &value) : value_(value)
    {}

    virtual ~UserObject() = default;

    IINTERFACE_DECL();

    ErrCode GetValue(std::shared_ptr<UserObjectBase> &value) override; /* [out] */

    bool Equals(IObject &other) override; /* [in] */

    std::string ToString() override;

    static sptr<IUserObject> Box(const std::shared_ptr<UserObjectBase> &value); /* [in] */

    static std::shared_ptr<UserObjectBase> Unbox(IUserObject *object); /* [in] */

    static sptr<IUserObject> Parse(const std::string &str); /* [in] */

public:
    static constexpr char SIGNATURE = 'O';

private:
    std::shared_ptr<UserObjectBase> value_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_USER_OBJECT_WRAPPER_H
