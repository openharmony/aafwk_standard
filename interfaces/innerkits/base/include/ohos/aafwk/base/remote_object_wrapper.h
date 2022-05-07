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

#ifndef OHOS_AAFWK_REMOTE_OBJECT_WRAP_H
#define OHOS_AAFWK_REMOTE_OBJECT_WRAP_H

#include "ohos/aafwk/base/base_object.h"
#include "iremote_object.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
class RemoteObjectWrap final : public Object, public IRemoteObjectWrap {
public:
    inline RemoteObjectWrap(sptr<IRemoteObject> remoteObject);

    inline ~RemoteObjectWrap()
    {}

    IINTERFACE_DECL();

    bool Equals(IObject &other) override; /* [in] */

    static sptr<IRemoteObjectWrap> Box(sptr<IRemoteObject> remoteObject);

    static sptr<IRemoteObject> UnBox(IRemoteObjectWrap *object);

    sptr<IRemoteObject> GetValue() override;

private:
    sptr<IRemoteObject> remoteObject_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_REMOTE_OBJECT_WRAP_H
