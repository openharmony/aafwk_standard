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

#include "remote_object_wrapper.h"

namespace OHOS {
namespace AAFwk {
IINTERFACE_IMPL_1(RemoteObjectWrap, Object, IRemoteObjectWrap);

RemoteObjectWrap::RemoteObjectWrap(sptr<IRemoteObject> remoteObject) : remoteObject_(remoteObject) {}

bool RemoteObjectWrap::Equals(IObject &other)
{
    RemoteObjectWrap *otherObj = static_cast<RemoteObjectWrap *>(IRemoteObjectWrap::Query(&other));
    return otherObj != nullptr && otherObj->remoteObject_ == remoteObject_;
}

sptr<IRemoteObjectWrap> RemoteObjectWrap::Box(sptr<IRemoteObject> remoteObject)
{
    return new RemoteObjectWrap(remoteObject);
}

sptr<IRemoteObject> RemoteObjectWrap::UnBox(IRemoteObjectWrap *object)
{
    return object->GetValue();
}

sptr<IRemoteObject> RemoteObjectWrap::GetValue()
{
    return this->remoteObject_;
}
}  // namespace AAFwk
}  // namespace OHOS