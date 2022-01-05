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

#include "js_runtime.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>

#include "native_engine/impl/ark/ark_native_engine.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t MAX_BUF_SIZE = 128;
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
#if defined(_ARM64_)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/libark_debugger.z.so";
#endif

class ArkJsRuntime : public JsRuntime {
public:
    ArkJsRuntime()
    {
        isArkEngine_ = true;
    }

    ~ArkJsRuntime() override
    {
        Deinitialize();
        if (vm_ != nullptr) {
            panda::JSNApi::DestroyJSVM(vm_);
            vm_ = nullptr;
        }
    }

    void StartDebugMode() override
    {
        panda::JSNApi::StartDebugger(ARK_DEBUGGER_LIB_PATH, vm_, isDebugMode_);
    }

private:
    static int32_t PrintVmLog(int32_t id, int32_t level, const char* tag, const char* fmt, const char* message)
    {
        HILOG_INFO("ArkLog: %{public}s", message);
        return 0;
    }

    bool Initialize(const Runtime::Options& options) override
    {
        panda::RuntimeOption pandaOption;
        pandaOption.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
        pandaOption.SetGcPoolSize(DEFAULT_GC_POOL_SIZE);
        pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
        pandaOption.SetLogBufPrint(PrintVmLog);
        vm_ = panda::JSNApi::CreateJSVM(pandaOption);
        if (vm_ == nullptr) {
            return false;
        }

        nativeEngine_ = std::make_unique<ArkNativeEngine>(vm_, static_cast<JsRuntime*>(this));
        return JsRuntime::Initialize(options);
    }

    panda::ecmascript::EcmaVM* vm_ = nullptr;
    bool isDebugMode_ = true;
};

NativeValue* ConsoleLog(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("engine or callback info is nullptr");
        return nullptr;
    }

    if (info->argc != 1) {
        HILOG_WARN("console.log() MUST have only one parameter");
        return engine->CreateUndefined();
    }

    NativeString* str = ConvertNativeValueTo<NativeString>(info->argv[0]);
    if (str == nullptr) {
        HILOG_ERROR("Faild to convert parameter to string");
        return engine->CreateUndefined();
    }

    char buf[MAX_BUF_SIZE];
    size_t len = 0;
    str->GetCString(buf, sizeof(buf) - 1, &len);
    buf[len] = '\0';

    HILOG_INFO("console.log: %{public}s\n", buf);
    return engine->CreateUndefined();
}

bool MakeFilePath(const std::string& codePath, const std::string& modulePath, std::string& fileName)
{
    std::string path(codePath);
    path.append("/").append(modulePath);

    char resolvedPath[PATH_MAX];
    if (realpath(path.c_str(), resolvedPath) == nullptr) {
        HILOG_ERROR("Failed to call realpath, errno = %d", errno);
        return false;
    }

    fileName = resolvedPath;
    return true;
}
} // namespace

std::unique_ptr<Runtime> JsRuntime::Create(const Runtime::Options& options)
{
    std::unique_ptr<JsRuntime> instance = std::make_unique<ArkJsRuntime>();
    if (!instance->Initialize(options)) {
        return std::unique_ptr<Runtime>();
    }
    return instance;
}

bool JsRuntime::Initialize(const Options& options)
{
    HandleScope handleScope(*this);

    NativeValue* consoleValue = nativeEngine_->CreateObject();
    NativeObject* consoleObj = ConvertNativeValueTo<NativeObject>(consoleValue);
    if (consoleObj == nullptr) {
        HILOG_ERROR("Failed to create console object");
        return false;
    }

    BindNativeFunction(*nativeEngine_, *consoleObj, "log", ConsoleLog);

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine_->GetGlobal());
    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return false;
    }
    globalObj->SetProperty("console", consoleValue);

    methodRequireNapiRef_.reset(nativeEngine_->CreateReference(globalObj->GetProperty("requireNapi"), 1));
    if (!methodRequireNapiRef_) {
        HILOG_ERROR("Failed to create reference for global.requireNapi");
        return false;
    }

    codePath_ = options.codePath;
    return true;
}

void JsRuntime::Deinitialize()
{
    methodRequireNapiRef_.reset();
    nativeEngine_.reset();
    modules_.clear();
}

std::unique_ptr<NativeReference> JsRuntime::LoadModule(const std::string& moduleName, const std::string& modulePath)
{
    HILOG_INFO("JsRuntime::LoadModule(%{public}s, %{public}s)", moduleName.c_str(), modulePath.c_str());

    if (!LoadModuleFile(moduleName, modulePath)) {
        HILOG_ERROR("Failed to load module file: %{public}s", modulePath.c_str());
        return std::unique_ptr<NativeReference>();
    }

    HandleScope handleScope(*this);

    NativeValue* argv[] = {
        nativeEngine_->CreateString(moduleName.c_str(), moduleName.length()),
        nativeEngine_->CreateBoolean(true),
    };
    NativeValue* classValue =
        nativeEngine_->CallFunction(nativeEngine_->GetGlobal(), methodRequireNapiRef_->Get(), argv, ArraySize(argv));
    NativeValue* instanceValue = nativeEngine_->CreateInstance(classValue, nullptr, 0);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(nativeEngine_->CreateReference(instanceValue, 1));
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModule(
    const std::string& moduleName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsRuntime::LoadSystemModule(%{public}s)", moduleName.c_str());

    HandleScope handleScope(*this);

    NativeValue* className = nativeEngine_->CreateString(moduleName.c_str(), moduleName.length());
    NativeValue* classValue =
        nativeEngine_->CallFunction(nativeEngine_->GetGlobal(), methodRequireNapiRef_->Get(), &className, 1);
    NativeValue* instanceValue = nativeEngine_->CreateInstance(classValue, argv, argc);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(nativeEngine_->CreateReference(instanceValue, 1));
}

bool JsRuntime::LoadModuleFile(const std::string& moduleName, const std::string& modulePath)
{
    NativeModuleManager* moduleMgr = nativeEngine_->GetModuleManager();
    if (moduleMgr == nullptr) {
        HILOG_ERROR("NativeModuleManager is NULL");
        return false;
    }
    if (moduleMgr->LoadNativeModule(moduleName.c_str(), nullptr, true, false, isArkEngine_) != nullptr) {
        return true;
    }

    std::string fileName;
    if (!MakeFilePath(codePath_, modulePath, fileName)) {
        HILOG_ERROR("Failed to make module file path: %{private}s", modulePath.c_str());
        return false;
    }

    HILOG_DEBUG("Try to open module file: %{public}s", fileName.c_str());

    std::ifstream stream(fileName, std::ios_base::binary | std::ios_base::in | std::ios_base::ate);
    if (!stream.is_open()) {
        HILOG_ERROR("Failed to open module file: %{private}s", fileName.c_str());
        return false;
    }

    size_t fileLength = stream.tellg();
    std::unique_ptr<char[]> buf(new(std::nothrow) char[fileLength]);
    if (!buf) {
        HILOG_ERROR("Not enough memory");
        return false;
    }

    stream.seekg(0, std::ios_base::beg);
    stream.read(buf.get(), fileLength);

    modules_.emplace_back();
    auto& moduleFileInfo = modules_.back();
    moduleFileInfo.moduleName = moduleName;
    moduleFileInfo.fileName = fileName;
    moduleFileInfo.fileLength = fileLength;
    moduleFileInfo.fileData = std::move(buf);

    NativeModule newModuleInfo = {
        .name = moduleFileInfo.moduleName.c_str(),
        .fileName = moduleFileInfo.fileName.c_str(),
    };

    moduleMgr->Register(&newModuleInfo);

    // Register will not copy field 'jsCode' and 'jsCodeLen'
    NativeModule* moduleInfo = moduleMgr->LoadNativeModule(moduleName.c_str(), nullptr, true, false, isArkEngine_);
    if (moduleInfo == nullptr) {
        HILOG_ERROR("moduleInfo is nullptr");
        return false;
    }
    moduleInfo->jsCode = moduleFileInfo.fileData.get();
    moduleInfo->jsCodeLen = fileLength;

    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
