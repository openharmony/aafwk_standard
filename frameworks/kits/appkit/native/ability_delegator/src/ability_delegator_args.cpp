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

#include "ability_delegator_args.h"

namespace OHOS {
namespace AppExecFwk {
const std::string AbilityDelegatorArgs::KEY_TEST_RUNNER_CLASS {"-s unittest"};
const std::string AbilityDelegatorArgs::KEY_TEST_CASE {"-s class"};

AbilityDelegatorArgs::AbilityDelegatorArgs()
{}

AbilityDelegatorArgs::AbilityDelegatorArgs(const AAFwk::Want &want)
{
    bundleName_ = want.GetStringParam("-p");
    params_["-p"] = want.GetStringParam("-p");
    params_["-s unittest"] = want.GetStringParam("-s unittest");
    params_["-s class"] = want.GetStringParam("-s class");
    if (!want.GetStringParam("-w").empty()) {
        params_["-w"] = want.GetStringParam("-w");
    }
}

AbilityDelegatorArgs::~AbilityDelegatorArgs()
{}

void AbilityDelegatorArgs::SetTestBundleName(const std::string &bundleName)
{
    bundleName_ = bundleName;
}

std::string AbilityDelegatorArgs::GetTestBundleName() const
{
    return bundleName_;
}

std::string AbilityDelegatorArgs::GetTestRunnerClassName() const
{
    auto target = params_.find(AbilityDelegatorArgs::KEY_TEST_RUNNER_CLASS);
    if (target != params_.end()) {
        return target->second;
    }

    return {};
}

std::string AbilityDelegatorArgs::GetTestCaseName() const
{
    auto target = params_.find(AbilityDelegatorArgs::KEY_TEST_CASE);
    if (target != params_.end()) {
        return target->second;
    }

    return {};
}

void AbilityDelegatorArgs::SetTestParam(const std::map<std::string, std::string> &params)
{
    params_ = params;
}

std::map<std::string, std::string> AbilityDelegatorArgs::GetTestParam() const
{
    return params_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
