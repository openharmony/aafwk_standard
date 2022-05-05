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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORMMGR_FORM_SUPPLY_CALLBACK_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORMMGR_FORM_SUPPLY_CALLBACK_H

#include "form_supply_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormSupplyStub
 * FormSupplyStub.
 */
class MockFormSupplyCallback : public FormSupplyStub {
public:
    MockFormSupplyCallback() = default;
    virtual ~MockFormSupplyCallback() = default;
    static sptr<MockFormSupplyCallback> GetInstance();

    /**
     * onAcquire
     *
     * @param providerFormInfo providerFormInfo
     * @param want data
     */
    virtual int OnAcquire(const FormProviderInfo &formInfo, const Want& want) override;

    /**
     * onEventHandle
     *
     * @param want data
     */
    virtual int OnEventHandle(const Want& want) override;

    /**
     * OnAcquireStateResult
     *
     * @param state Form state
     * @param provider provider info.
     * @param wantArg The want of onAcquireFormState.
     * @param want input data
     */
    virtual int OnAcquireStateResult(FormState state, const std::string &provider, const Want &wantArg,
                                     const Want &want) override;

private:
    static std::mutex mutex;
    static sptr<MockFormSupplyCallback> instance;

    DISALLOW_COPY_AND_MOVE(MockFormSupplyCallback);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORMMGR_FORM_SUPPLY_CALLBACK_H
