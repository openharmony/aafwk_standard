/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "form_cache_mgr.h"
#include "hilog_wrapper.h"
#include "form_dump_mgr.h"

namespace OHOS {
namespace AppExecFwk {
const std::string LINE_SEPARATOR = "\n";

FormDumpMgr::FormDumpMgr(){}
FormDumpMgr::~FormDumpMgr(){}
/**
 * @brief Dump all of form storage infos.
 * @param storageInfos Form storage infos
 * @param formInfos Form storage dump info.
 */
void FormDumpMgr::DumpStorageFormInfos(const std::vector<FormDBInfo> &storageInfos, std::string &formInfos) const
{
    for (const auto &info : storageInfos) {
        formInfos += "  FormId #" + std::to_string(info.formId) + "\n";
        formInfos += "    formName [" + info.formName + "]\n";
        formInfos += "    userId [" + std::to_string(info.userId) + "]\n";
        formInfos += "    bundleName [" + info.bundleName + "]\n";
        formInfos += "    moduleName [" + info.moduleName + "]\n";
        formInfos += "    abilityName [" + info.abilityName + "]\n";
        formInfos += "    formUserUids [";
        for (auto &uId : info.formUserUids) {
            formInfos += " Uid[" + std::to_string(uId) + "] ";
        }
        formInfos += "]\n" + LINE_SEPARATOR;
    }
}
/**
 * @brief Dump form infos.
 * @param formRecordInfos Form record infos.
 * @param formInfos Form dump infos.
 */
void FormDumpMgr::DumpFormInfos(const std::vector<FormRecord> &formRecordInfos, std::string &formInfos) const
{
    HILOG_INFO("%{public}s called.", __func__);
    for (const auto &info : formRecordInfos) {
        formInfos += "  FormId [" + std::to_string(info.formId)  + "]\n";
        formInfos += "    formName [" + info.formName + "]\n";
        formInfos += "    bundleName [" + info.bundleName + "]\n";
        formInfos += "    moduleName [" + info.moduleName + "]\n";
        formInfos += "    abilityName [" + info.abilityName + "]\n";
        formInfos += "    isInited [" + std::to_string(info.isInited) + "]\n";
        formInfos += "    needRefresh [" + std::to_string(info.needRefresh) + "]\n";
        formInfos += "    isEnableUpdate [" + std::to_string(info.isEnableUpdate) + "]\n";
        formInfos += "    isCountTimerRefresh [" + std::to_string(info.isCountTimerRefresh) + "]\n";
        formInfos += "    specification [" + std::to_string(info.specification) + "]\n";
        formInfos += "    updateDuration [" + std::to_string(info.updateDuration) + "]\n";
        formInfos += "    updateAtHour [" + std::to_string(info.updateAtHour) + "]\n";
        formInfos += "    updateAtMin [" + std::to_string(info.updateAtMin) + "]\n";
        formInfos += "    formTempFlg [" + std::to_string(info.formTempFlg) + "]\n";
        formInfos += "    formVisibleNotify [" + std::to_string(info.formVisibleNotify) + "]\n";
        formInfos += "    formVisibleNotifyState [" + std::to_string(info.formVisibleNotifyState) + "]\n";
        formInfos += "    userId [" + std::to_string(info.userId) + "]\n";

        if (info.hapSourceDirs.size() > 0) {
            formInfos += "    hapSourceDirs [";
            for (auto &hapDir : info.hapSourceDirs) {
                formInfos += " hapSourceDir[" + hapDir + "]";
            }
            formInfos += "]\n";
        }

        if (info.formUserUids.size() > 0) {
            formInfos += "    formUserUids [";
            for (auto &uId : info.formUserUids) {
                formInfos += " Uid[" + std::to_string(uId) + "] ";
            }
            formInfos += "]\n";
        }

        // formCacheData
        std::string strCacheData;
        std::map<std::string, std::pair<sptr<FormAshmem>, int32_t>> imageMap;
        if (FormCacheMgr::GetInstance().GetData(info.formId, strCacheData, imageMap)) {
            formInfos += "    formCacheData [";
            formInfos += strCacheData;
            formInfos += "]\n" + LINE_SEPARATOR;
        }
    }

    HILOG_INFO("%{public}s success. Form infos:%{public}s", __func__, formInfos.c_str());
}
/**
 * @brief Dump form infos.
 * @param formRecordInfo Form Host record info.
 * @param formInfo Form dump info.
 */
void FormDumpMgr::DumpFormHostInfo(const FormHostRecord &formHostRecord, std::string &formInfo) const
{
    HILOG_INFO("%{public}s called.", __func__);
    formInfo += "  ================FormHostRecord=================\n";
    formInfo += "  callerUid [" + std::to_string(formHostRecord.GetCallerUid()) + "]\n";
    formInfo += "  hostBundleName [" + formHostRecord.GetHostBundleName() + "]\n";
    HILOG_INFO("%{public}s success. Host Form infos:%{public}s", __func__, formInfo.c_str());
}

/**
 * @brief Dump form infos.
 * @param formRecordInfo Form record info.
 * @param formInfo Form dump info.
 */
void FormDumpMgr::DumpFormInfo(const FormRecord &formRecordInfo, std::string &formInfo) const
{
    HILOG_INFO("%{public}s called.", __func__);
    formInfo += "  ================FormRecord=================\n";
    formInfo += "  FormId [" + std::to_string(formRecordInfo.formId) + "]\n";
    formInfo += "    formName [" + formRecordInfo.formName + "]\n";
    formInfo += "    bundleName [" + formRecordInfo.bundleName + "]\n";
    formInfo += "    moduleName [" + formRecordInfo.moduleName + "]\n";
    formInfo += "    abilityName [" + formRecordInfo.abilityName + "]\n";
    formInfo += "    isInited [" + std::to_string(formRecordInfo.isInited) + "]\n";
    formInfo += "    needRefresh [" + std::to_string(formRecordInfo.needRefresh) + "]\n";
    formInfo += "    isEnableUpdate [" + std::to_string(formRecordInfo.isEnableUpdate) + "]\n";
    formInfo += "    isCountTimerRefresh [" + std::to_string(formRecordInfo.isCountTimerRefresh) + "]\n";
    formInfo += "    specification [" + std::to_string(formRecordInfo.specification) + "]\n";
    formInfo += "    updateDuration [" + std::to_string(formRecordInfo.updateDuration) + "]\n";
    formInfo += "    updateAtHour [" + std::to_string(formRecordInfo.updateAtHour) + "]\n";
    formInfo += "    updateAtMin [" + std::to_string(formRecordInfo.updateAtMin) + "]\n";
    formInfo += "    formTempFlg [" + std::to_string(formRecordInfo.formTempFlg) + "]\n";
    formInfo += "    formVisibleNotify [" + std::to_string(formRecordInfo.formVisibleNotify) + "]\n";
    formInfo += "    formVisibleNotifyState [" + std::to_string(formRecordInfo.formVisibleNotifyState) + "]\n";
    formInfo += "    formSrc [" + formRecordInfo.formSrc + "]\n";
    formInfo += "    designWidth [" + std::to_string(formRecordInfo.formWindow.designWidth) + "]\n";
    formInfo += "    autoDesignWidth [" + std::to_string(formRecordInfo.formWindow.autoDesignWidth) + "]\n";
    formInfo += "    versionCode [" + std::to_string(formRecordInfo.versionCode) + "]\n";
    formInfo += "    versionName [" + formRecordInfo.versionName + "]\n";
    formInfo += "    compatibleVersion [" + std::to_string(formRecordInfo.compatibleVersion) + "]\n";
    formInfo += "    icon [" + formRecordInfo.icon + "]\n";
    formInfo += "    userId [" + std::to_string(formRecordInfo.userId) + "]\n";

    if (formRecordInfo.hapSourceDirs.size() > 0) {
        formInfo += "    hapSourceDirs [";
        for (auto &hapDir : formRecordInfo.hapSourceDirs) {
            formInfo += " hapSourceDir[" + hapDir + "] ";
        }
        formInfo += "]\n";
    }

    if (formRecordInfo.formUserUids.size() > 0) {
        formInfo += "    formUserUids [";
        for (auto &uId : formRecordInfo.formUserUids) {
            formInfo += " Uid[" + std::to_string(uId) + "] ";
        }
        formInfo += "]\n";
    }

    // formCacheData
    std::string strCacheData;
    std::map<std::string, std::pair<sptr<FormAshmem>, int32_t>> imageMap;
    if (FormCacheMgr::GetInstance().GetData(formRecordInfo.formId, strCacheData, imageMap)) {
        formInfo += "    formCacheData[";
        formInfo += strCacheData;
        formInfo += "]\n" + LINE_SEPARATOR;
    }

    HILOG_INFO("%{public}s success. Form infos:%{public}s", __func__, formInfo.c_str());
}
}  // namespace AppExecFwk
}  // namespace OHOS