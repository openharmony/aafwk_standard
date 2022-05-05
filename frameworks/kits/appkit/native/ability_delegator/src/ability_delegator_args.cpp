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

#include "ability_delegator_args.h"

namespace OHOS {
namespace AppExecFwk {
const std::string AbilityDelegatorArgs::KEY_TEST_BUNDLE_NAME {"-b"};
const std::string AbilityDelegatorArgs::KEY_TEST_PACKAGE_NAME {"-p"};
const std::string AbilityDelegatorArgs::KEY_TEST_MODULE_NAME {"-m"};
const std::string AbilityDelegatorArgs::KEY_TEST_RUNNER_CLASS {"-s unittest"};
const std::string AbilityDelegatorArgs::KEY_TEST_CASE {"-s class"};
const std::string AbilityDelegatorArgs::KEY_TEST_WAIT_TIMEOUT {"-w"};

const std::string AbilityDelegatorArgs::KEY_TEST_DEBUG {"-D"};
const std::string AbilityDelegatorArgs::VALUE_TEST_DEBUG {"true"};

AbilityDelegatorArgs::AbilityDelegatorArgs()
{}

AbilityDelegatorArgs::AbilityDelegatorArgs(const AAFwk::Want &want)
{
    bundleName_ = want.GetStringParam(AbilityDelegatorArgs::KEY_TEST_BUNDLE_NAME);

    auto wantParams(want.GetParams());
    if (wantParams.HasParam("debugApp")) {
        wantParams.Remove("debugApp");
    }

    std::set<std::string> keys = wantParams.KeySet();
    for (auto key : keys) {
        params_[key] = want.GetStringParam(key);
    }
}

AbilityDelegatorArgs::~AbilityDelegatorArgs()
{}

std::string AbilityDelegatorArgs::GetTestBundleName() const
{
    return bundleName_;
}

std::string AbilityDelegatorArgs::GetTestPackageName() const
{
    return GetParamValue(AbilityDelegatorArgs::KEY_TEST_PACKAGE_NAME);
}

std::string AbilityDelegatorArgs::GetTestModuleName() const
{
    return GetParamValue(AbilityDelegatorArgs::KEY_TEST_MODULE_NAME);
}

std::string AbilityDelegatorArgs::GetTestRunnerClassName() const
{
    return GetParamValue(AbilityDelegatorArgs::KEY_TEST_RUNNER_CLASS);
}

std::string AbilityDelegatorArgs::GetTestCaseName() const
{
    return GetParamValue(AbilityDelegatorArgs::KEY_TEST_CASE);
}

std::map<std::string, std::string> AbilityDelegatorArgs::GetTestParam() const
{
    return params_;
}

bool AbilityDelegatorArgs::FindDebugFlag()
{
    auto pos = params_.find(KEY_TEST_DEBUG);
    if (pos == params_.end()) {
        return false;
    }

    return !pos->second.compare(VALUE_TEST_DEBUG);
}

std::string AbilityDelegatorArgs::GetParamValue(const std::string &key) const
{
    auto target = params_.find(key);
    return (target != params_.end()) ? target->second : std::string();
}
}  // namespace AppExecFwk
}  // namespace OHOS
