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

#ifndef FOUNDATION_APPEXECFWK_OHOS_MOCK_PAGE_ABILITY_H
#define FOUNDATION_APPEXECFWK_OHOS_MOCK_PAGE_ABILITY_H

#include "ability.h"
#include <gtest/gtest.h>

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class MockPageAbility : public Ability {
public:
    MockPageAbility() = default;
    virtual ~MockPageAbility() = default;

    enum Event { ON_ACTIVE = 0, ON_BACKGROUND, ON_FOREGROUND, ON_INACTIVE, ON_START, ON_STOP, UNDEFINED };

    void OnKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnKeyDown called";
    }

    void OnKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnKeyUp called";
    }

    void OnPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnTouchEvent called";
    }

    void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnAbilityResult called";
        state_ = ON_ACTIVE;
    }

    void OnNewWant(const Want &want)
    {
        onNewWantCalled_ = true;
        GTEST_LOG_(INFO) << "MockPageAbility::OnNewWant called";
    }

    void OnStart(const Want &want)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnStart called";
        state_ = ON_START;
    }

    void OnStop()
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnStop called";
        state_ = ON_STOP;
    }

    void OnActive()
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnActive called";
        state_ = ON_ACTIVE;
    }

    void OnInactive()
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnInactive called";
        state_ = ON_INACTIVE;
    }

    void OnForeground(const Want &want)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnForeground called";
        state_ = ON_FOREGROUND;
    }

    void OnBackground()
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnBackground called";
        state_ = ON_BACKGROUND;
    }

    sptr<IRemoteObject> OnConnect(const Want &want)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnConnect called";
        state_ = ON_ACTIVE;
        return nullptr;
    }

    void OnCommand(const AAFwk::Want &want, bool restart, int startId)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OnBackground called";
        state_ = ON_ACTIVE;
    }

    int OpenFile(const Uri &uri, const std::string &mode)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::OpenFile called";
        state_ = ON_ACTIVE;
        return 1;
    }

    int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::Insert called";
        state_ = ON_ACTIVE;
        return -1;
    }

    int Update(const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::Update called";
        state_ = ON_ACTIVE;
        return -1;
    }

    int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::Delete called";
        state_ = ON_ACTIVE;
        return -1;
    }

    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::Query called";
        state_ = ON_ACTIVE;
        return nullptr;
    }

    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
    {
        GTEST_LOG_(INFO) << "MockPageAbility::GetFileTypes called";

        value.push_back(mimeTypeFilter);
        return value;
    }

    void OnRestoreAbilityState(const PacMap &inState)
    {
        GTEST_LOG_(INFO) << "Mock Ability::OnRestoreAbilityState called";
    }

    void OnSaveAbilityState(PacMap &outState)
    {
        GTEST_LOG_(INFO) << "Mock Ability::OnSaveAbilityState called";
    }

    MockPageAbility::Event state_ = UNDEFINED;
    bool onNewWantCalled_ = false;
    std::vector<std::string> value;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_MOCK_PAGE_ABILITY_H