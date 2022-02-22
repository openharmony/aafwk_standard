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

#ifndef OHOS_AAFWK_URI_PERMISSION_MANAGER_SERVICE_H
#define OHOS_AAFWK_URI_PERMISSION_MANAGER_SERVICE_H

#include "singleton.h"
#include "system_ability.h"
#include "uri_permission_manager_stub_impl.h"

namespace OHOS {
namespace AAFwk {
class UriPermissionManagerService : public SystemAbility {
    DECLARE_DELAYED_SINGLETON(UriPermissionManagerService);
    DECLARE_SYSTEM_ABILITY(UriPermissionManagerService);
public:
    void OnStart() override;
    void OnStop() override;

    /**
     * @brief Check whether if the uri permission manager service is ready.
     *
     * @return Returns true if the uri permission manager service is ready; returns false otherwise.
     */
    bool IsServiceReady() const;

private:
    bool Init();
    void SelfClean();

private:
    bool ready_ = false;
    bool registerToService_ = false;
    sptr<UriPermissionManagerStubImpl> impl_;

    DISALLOW_COPY_AND_MOVE(UriPermissionManagerService);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_URI_PERMISSION_MANAGER_SERVICE_H
