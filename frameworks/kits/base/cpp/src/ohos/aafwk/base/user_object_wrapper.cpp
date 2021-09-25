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
#include "ohos/aafwk/base/user_object_wrapper.h"
#include "ohos/aafwk/base/user_object_base.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string SPLIT = "#";
};
IINTERFACE_IMPL_1(UserObject, Object, IUserObject);

ErrCode UserObject::GetValue(std::shared_ptr<UserObjectBase> &value)
{
    value = value_;
    return ERR_OK;
}
bool UserObject::Equals(IObject &other)
{
    if (value_ == nullptr) {
        return false;
    }

    UserObject *otherObj = static_cast<UserObject *>(IUserObject::Query(&other));
    if (otherObj == nullptr) {
        return false;    
    }
    if (value_->GetClassName() == otherObj->value_->GetClassName()) {
        return otherObj->value_->Equals(value_);
    }
    return false;
}

std::string UserObject::ToString()
{
    if (value_ == nullptr) {
        return std::string("");
    }
    return value_->GetClassName() + SPLIT + value_->ToString();
}

sptr<IUserObject> UserObject::Box(const std::shared_ptr<UserObjectBase> &value)
{
    if (value != nullptr) {
        sptr<IUserObject> object = new (std::nothrow) UserObject(value);
        return object;
    } else {
        return nullptr;
    }
}

std::shared_ptr<UserObjectBase> UserObject::Unbox(IUserObject *object)
{
    std::shared_ptr<UserObjectBase> value = nullptr;
    if (object == nullptr) {
        return nullptr;
    }

    object->GetValue(value);
    return value;
}

sptr<IUserObject> UserObject::Parse(const std::string &str)
{
    std::size_t len = str.length();
    if (len < 1) {
        return nullptr;
    }
    std::size_t splitPos = str.find(SPLIT);
    if (splitPos == std::string::npos) {
        return nullptr;
    }
    std::string className = str.substr(0, splitPos);
    std::string content = str.substr(className.length() + 1, len - 1);
    if (className.length() + SPLIT.length() + content.length() != len) {
        return nullptr;
    }

    UserObjectBase *userObjectBase =
        static_cast<UserObjectBase *>(UserObjectBaseLoader::GetInstance().GetUserObjectByName(className));
    if (userObjectBase != nullptr) {
        userObjectBase->Parse(content);
        sptr<IUserObject> ret = new UserObject(std::shared_ptr<UserObjectBase>(userObjectBase));
        return ret;
    }
    return nullptr;
}

UserObjectBaseLoader &UserObjectBaseLoader::GetInstance(void)
{
    static UserObjectBaseLoader gUserObjectBaseLoader;
    return gUserObjectBaseLoader;
}

/**
 * @brief Registered user-defined serialization class.
 * @param objectName The name of the custom class.
 * @param createFun Function object that creates an instance of a custom class.
 */
void UserObjectBaseLoader::RegisterUserObject(const std::string &objectName, const CreateUserObjectBase &createFun)
{
    register_class_list_.emplace(objectName, createFun);
}

/**
 * @brief Represents obtaining an instance of an object of a registered serialization class.
 * @param className The name of the custom class.
 *
 * @return Returns an instance of the object, or nullptr on failure.
 */
UserObjectBase *UserObjectBaseLoader::GetUserObjectByName(const std::string &className)
{
    auto iter = register_class_list_.find(className);
    if (iter != register_class_list_.end()) {
        return iter->second();
    } else {
        return nullptr;
    }
}

}  // namespace AAFwk
}  // namespace OHOS