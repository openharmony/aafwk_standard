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

#include "js_runtime.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>

#include "native_engine/impl/ark/ark_native_engine.h"
#ifdef SUPPORT_GRAPHICS
#include "core/common/container_scope.h"
#include "declarative_module_preloader.h"
#endif
#include "event_handler.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

#ifdef ENABLE_HITRACE
#include "hitrace/trace.h"
#endif
#include "systemcapability.h"
#include "parameters.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr uint8_t SYSCAP_MAX_SIZE = 64;
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
constexpr int64_t ASSET_FILE_MAX_SIZE = 20 * 1024 * 1024;
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

    void StartDebugMode(bool needBreakPoint) override
    {
        if (!debugMode_) {
            HILOG_INFO("Ark VM is starting debug mode [%{public}s]", needBreakPoint ? "break" : "normal");
            panda::JSNApi::StartDebugger(ARK_DEBUGGER_LIB_PATH, vm_, needBreakPoint);
            debugMode_ = true;
        }
    }

    bool RunScript(const std::string& path) override
    {
        static const char PANDA_MAIN_FUNCTION[] = "_GLOBAL::func_main_0";
        return vm_ != nullptr ? panda::JSNApi::Execute(vm_, path.c_str(), PANDA_MAIN_FUNCTION) : false;
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
        int arkProperties = OHOS::system::GetIntParameter<int>("persist.ark.properties", -1);
        pandaOption.SetArkProperties(arkProperties);
        HILOG_INFO("ArkJSRuntime::Initialize ark properties = %{public}d", arkProperties);
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
};

std::string GetLogContent(NativeCallbackInfo& info)
{
    std::string content;

    for (size_t i = 0; i < info.argc; i++) {
        NativeValue* value = info.argv[i];
        if (value->TypeOf() != NATIVE_STRING) {
            value = value->ToString();
        }

        NativeString* str = ConvertNativeValueTo<NativeString>(value);
        if (str == nullptr) {
            continue;
        }

        size_t bufferLen = str->GetLength();
        auto buffer = std::make_unique<char[]>(bufferLen + 1);
        if (buffer == nullptr) {
            break;
        }

        size_t strLen = 0;
        str->GetCString(buffer.get(), bufferLen + 1, &strLen);
        if (!content.empty()) {
            content.append(" ");
        }
        content.append(buffer.get());
    }

    return content;
}

template<LogLevel LEVEL>
NativeValue* ConsoleLog(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("engine or callback info is nullptr");
        return nullptr;
    }

    std::string content = GetLogContent(*info);
    HiLogPrint(LOG_APP, LEVEL, AMS_LOG_DOMAIN, "JsApp", "%{public}s", content.c_str());

    return engine->CreateUndefined();
}

void InitConsoleLogModule(NativeEngine& engine, NativeObject& globalObject)
{
    NativeValue* consoleValue = engine.CreateObject();
    NativeObject* consoleObj = ConvertNativeValueTo<NativeObject>(consoleValue);
    if (consoleObj == nullptr) {
        HILOG_ERROR("Failed to create console object");
        return;
    }

    BindNativeFunction(engine, *consoleObj, "log", ConsoleLog<LOG_INFO>);
    BindNativeFunction(engine, *consoleObj, "debug", ConsoleLog<LOG_DEBUG>);
    BindNativeFunction(engine, *consoleObj, "info", ConsoleLog<LOG_INFO>);
    BindNativeFunction(engine, *consoleObj, "warn", ConsoleLog<LOG_WARN>);
    BindNativeFunction(engine, *consoleObj, "error", ConsoleLog<LOG_ERROR>);
    BindNativeFunction(engine, *consoleObj, "fatal", ConsoleLog<LOG_FATAL>);

    globalObject.SetProperty("console", consoleValue);
}

NativeValue* SetTimeout(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("Set timeout failed with engine or callback info is nullptr.");
        return nullptr;
    }

    JsRuntime& jsRuntime = *reinterpret_cast<JsRuntime*>(engine->GetJsEngine());
    return jsRuntime.SetCallbackTimer(*engine, *info, false);
}

NativeValue* CanIUse(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("get syscap failed since engine or callback info is nullptr.");
        return nullptr;
    }

    if (info->argc != 1 || info->argv[0]->TypeOf() != NATIVE_STRING) {
        HILOG_ERROR("Get syscap failed with invalid parameter.");
        return engine->CreateUndefined();
    }

    char syscap[SYSCAP_MAX_SIZE] = { 0 };

    NativeString* str = ConvertNativeValueTo<NativeString>(info->argv[0]);
    if (str == nullptr) {
        HILOG_ERROR("Convert to NativeString failed.");
        return engine->CreateUndefined();
    }
    size_t bufferLen = str->GetLength();
    size_t strLen = 0;
    str->GetCString(syscap, bufferLen + 1, &strLen);

    bool ret = HasSystemCapability(syscap);
    return engine->CreateBoolean(ret);
}

NativeValue* SetInterval(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("Set interval failed with engine or callback info is nullptr.");
        return nullptr;
    }

    JsRuntime& jsRuntime = *reinterpret_cast<JsRuntime*>(engine->GetJsEngine());
    return jsRuntime.SetCallbackTimer(*engine, *info, true);
}

NativeValue* ClearTimeoutOrInterval(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("Clear timer failed with engine or callback info is nullptr.");
        return nullptr;
    }

    JsRuntime& jsRuntime = *reinterpret_cast<JsRuntime*>(engine->GetJsEngine());
    return jsRuntime.ClearCallbackTimer(*engine, *info);
}

void InitTimerModule(NativeEngine& engine, NativeObject& globalObject)
{
    BindNativeFunction(engine, globalObject, "setTimeout", SetTimeout);
    BindNativeFunction(engine, globalObject, "setInterval", SetInterval);
    BindNativeFunction(engine, globalObject, "clearTimeout", ClearTimeoutOrInterval);
    BindNativeFunction(engine, globalObject, "clearInterval", ClearTimeoutOrInterval);
}

void InitSyscapModule(NativeEngine& engine, NativeObject& globalObject)
{
    BindNativeFunction(engine, globalObject, "canIUse", CanIUse);
}

bool MakeFilePath(const std::string& codePath, const std::string& modulePath, std::string& fileName)
{
    std::string path(codePath);
    path.append("/").append(modulePath);
    if (path.length() > PATH_MAX) {
        HILOG_ERROR("Path length(%{public}d) longer than MAX(%{public}d)", (int32_t)path.length(), PATH_MAX);
        return false;
    }
    char resolvedPath[PATH_MAX + 1] = { 0 };
    if (realpath(path.c_str(), resolvedPath) != nullptr) {
        fileName = resolvedPath;
        return true;
    }

    auto start = path.find_last_of('/');
    auto end = path.find_last_of('.');
    if (end == std::string::npos || end == 0) {
        HILOG_ERROR("No secondary file path");
        return false;
    }

    auto pos = path.find_last_of('.', end - 1);
    if (pos == std::string::npos) {
        HILOG_ERROR("No secondary file path");
        return false;
    }

    path.erase(start + 1, pos - start);
    HILOG_INFO("Try using secondary file path: %{public}s", path.c_str());

    if (realpath(path.c_str(), resolvedPath) == nullptr) {
        HILOG_ERROR("Failed to call realpath, errno = %{public}d", errno);
        return false;
    }

    fileName = resolvedPath;
    return true;
}

void RegisterInitWorkerFunc(NativeEngine& engine)
{
    auto&& initWorkerFunc = [](NativeEngine* nativeEngine) {
        HILOG_INFO("RegisterInitWorkerFunc called");
        if (nativeEngine == nullptr) {
            HILOG_ERROR("Input nativeEngine is nullptr");
            return;
        }

        NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine->GetGlobal());
        if (globalObj == nullptr) {
            HILOG_ERROR("Failed to get global object");
            return;
        }

        InitConsoleLogModule(*nativeEngine, *globalObj);
    };
    engine.SetInitWorkerFunc(initWorkerFunc);
}

bool GetResourceData(const std::string& filePath, std::vector<uint8_t>& content)
{
    std::ifstream stream(filePath);
    if (!stream.is_open()) {
        HILOG_ERROR("GetResourceData failed with file can't open, check uri.");
        return false;
    }

    stream.seekg(0, std::ios::end);
    auto fileLen = stream.tellg();
    if (fileLen > ASSET_FILE_MAX_SIZE) {
        HILOG_ERROR("GetResourceData failed with file too large.");
        stream.close();
        return false;
    }

    content.resize(fileLen);
    stream.seekg(0, std::ios::beg);
    stream.read(reinterpret_cast<char*>(content.data()), content.size());
    stream.close();

    return true;
}

void RegisterAssetFunc(NativeEngine& engine, const std::string& codePath)
{
    auto&& assetFunc = [codePath](const std::string& uri, std::vector<uint8_t>& content, std::string &ami) {
        if (uri.empty()) {
            HILOG_ERROR("Uri is empty.");
            return;
        }

        HILOG_INFO("RegisterAssetFunc called, uri: %{private}s", uri.c_str());
        size_t index = uri.find_last_of(".");
        if (index == std::string::npos) {
            HILOG_ERROR("Invalid uri");
            return;
        }

        std::string fileUri = uri.substr(0, index) + ".abc";
        std::string targetFile = codePath;
        targetFile += (codePath.back() == '/') ? fileUri : "/" + fileUri;
        ami = targetFile;
        HILOG_INFO("Get asset, ami: %{private}s", ami.c_str());
        if (!GetResourceData(ami, content)) {
            HILOG_ERROR("Get asset content failed.");
            return;
        }
    };
    engine.SetGetAssetFunc(assetFunc);
}

void RegisterWorker(NativeEngine& engine, const std::string& codePath)
{
    RegisterInitWorkerFunc(engine);
    RegisterAssetFunc(engine, codePath);
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
    // Create event handler for runtime
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(options.eventRunner);
    nativeEngine_->SetPostTask([this](bool needSync) {
        eventHandler_->PostTask(
            [this, needSync]() {
                nativeEngine_->Loop(LOOP_NOWAIT, needSync);
            },
            "idleTask");
    });
    nativeEngine_->CheckUVLoop();

    HandleScope handleScope(*this);

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine_->GetGlobal());
    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return false;
    }

    InitConsoleLogModule(*nativeEngine_, *globalObj);
    InitTimerModule(*nativeEngine_, *globalObj);
    InitSyscapModule(*nativeEngine_, *globalObj);

    // Simple hook function 'isSystemplugin'
    BindNativeFunction(*nativeEngine_, *globalObj, "isSystemplugin",
        [](NativeEngine* engine, NativeCallbackInfo* info) -> NativeValue* {
            return engine->CreateUndefined();
        });

    methodRequireNapiRef_.reset(nativeEngine_->CreateReference(globalObj->GetProperty("requireNapi"), 1));
    if (!methodRequireNapiRef_) {
        HILOG_ERROR("Failed to create reference for global.requireNapi");
        return false;
    }
#ifdef SUPPORT_GRAPHICS
    if (options.loadAce) {
        OHOS::Ace::DeclarativeModulePreloader::Preload(*nativeEngine_);
    }
#endif
    codePath_ = options.codePath;

    auto moduleManager = NativeModuleManager::GetInstance();
    std::string packagePath = options.packagePath;
    if (moduleManager && !packagePath.empty()) {
        moduleManager->SetAppLibPath(packagePath.c_str());
    }

    RegisterWorker(*nativeEngine_, options.codePath);

    return true;
}

void JsRuntime::Deinitialize()
{
    for (auto it = modules_.begin(); it != modules_.end(); it = modules_.erase(it)) {
        delete it->second;
        it->second = nullptr;
    }

    methodRequireNapiRef_.reset();
    nativeEngine_->CancelCheckUVLoop();
    RemoveTask("idleTask");
    nativeEngine_.reset();
}

std::unique_ptr<NativeReference> JsRuntime::LoadModule(const std::string& moduleName, const std::string& modulePath)
{
    HILOG_INFO("JsRuntime::LoadModule(%{public}s, %{public}s)", moduleName.c_str(), modulePath.c_str());

    HandleScope handleScope(*this);

    NativeValue* classValue = nullptr;

    auto it = modules_.find(modulePath);
    if (it != modules_.end()) {
        classValue = it->second->Get();
    } else {
        std::string fileName;
        if (!MakeFilePath(codePath_, modulePath, fileName)) {
            HILOG_ERROR("Failed to make module file path: %{private}s", modulePath.c_str());
            return std::unique_ptr<NativeReference>();
        }

        NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine_->GetGlobal());
        NativeValue* exports = nativeEngine_->CreateObject();
        globalObj->SetProperty("exports", exports);

        if (!RunScript(fileName)) {
            HILOG_ERROR("Failed to run script: %{public}s", fileName.c_str());
            return std::unique_ptr<NativeReference>();
        }

        NativeObject* exportsObj = ConvertNativeValueTo<NativeObject>(globalObj->GetProperty("exports"));
        if (exportsObj == nullptr) {
            HILOG_ERROR("Failed to get exports objcect: %{public}s", modulePath.c_str());
            return std::unique_ptr<NativeReference>();
        }

        classValue = exportsObj->GetProperty("default");
        if (classValue == nullptr) {
            HILOG_ERROR("Failed to get default objcect: %{public}s", modulePath.c_str());
            return std::unique_ptr<NativeReference>();
        }

        modules_.emplace(modulePath, nativeEngine_->CreateReference(classValue, 1));
    }

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

bool JsRuntime::RunScript(const std::string& path)
{
    return nativeEngine_->RunScript(path.c_str()) != nullptr;
}

bool JsRuntime::RunSendboxScript(const std::string& path)
{
    std::string fileName;
    if (!MakeFilePath(codePath_, path, fileName)) {
        HILOG_ERROR("Failed to make module file path: %{private}s", path.c_str());
        return false;
    }

    if (!RunScript(fileName)) {
        HILOG_ERROR("Failed to run script: %{public}s", fileName.c_str());
        return false;
    }
    return true;
}

#ifdef SUPPORT_GRAPHICS
using OHOS::Ace::ContainerScope;
#endif
class TimerTask final {
public:
    TimerTask(
        JsRuntime& jsRuntime, std::shared_ptr<NativeReference> jsFunction, const std::string &name, int64_t interval)
        : jsRuntime_(jsRuntime), jsFunction_(jsFunction), name_(name), interval_(interval)
    {
#ifdef SUPPORT_GRAPHICS
        containerScopeId_ = ContainerScope::CurrentId();
#endif
#ifdef ENABLE_HITRACE
        traceId_ = new OHOS::HiviewDFX::HiTraceId(OHOS::HiviewDFX::HiTrace::GetId());
#endif
    }

    ~TimerTask()
    {
#ifdef ENABLE_HITRACE
        if (traceId_) {
            delete traceId_;
            traceId_ = nullptr;
        }
#endif
    }

    void operator()()
    {
        if (interval_ > 0) {
            jsRuntime_.PostTask(*this, name_, interval_);
        }
#ifdef SUPPORT_GRAPHICS
        // call js function
        ContainerScope containerScope(containerScopeId_);
#endif
        HandleScope handleScope(jsRuntime_);

        std::vector<NativeValue*> args_;
        args_.reserve(jsArgs_.size());
        for (auto arg : jsArgs_) {
            args_.emplace_back(arg->Get());
        }

        NativeEngine& engine = jsRuntime_.GetNativeEngine();
#ifdef ENABLE_HITRACE
        if (traceId_ && traceId_->IsValid()) {
            OHOS::HiviewDFX::HiTrace::SetId(*traceId_);
            engine.CallFunction(engine.CreateUndefined(), jsFunction_->Get(), args_.data(), args_.size());
            OHOS::HiviewDFX::HiTrace::ClearId();
            delete traceId_;
            traceId_ = nullptr;
            return;
        }
#endif
        engine.CallFunction(engine.CreateUndefined(), jsFunction_->Get(), args_.data(), args_.size());
    }

    void PushArgs(std::shared_ptr<NativeReference> ref)
    {
        jsArgs_.emplace_back(ref);
    }

private:
    JsRuntime& jsRuntime_;
    std::shared_ptr<NativeReference> jsFunction_;
    std::vector<std::shared_ptr<NativeReference>> jsArgs_;
    std::string name_;
    int64_t interval_ = 0;
#ifdef SUPPORT_GRAPHICS
    int32_t containerScopeId_ = 0;
#endif
#ifdef ENABLE_HITRACE
    OHOS::HiviewDFX::HiTraceId* traceId_ = nullptr;
#endif
};

void JsRuntime::PostTask(const TimerTask& task, const std::string& name, int64_t delayTime)
{
    eventHandler_->PostTask(task, name, delayTime);
}

void JsRuntime::RemoveTask(const std::string& name)
{
    eventHandler_->RemoveTask(name);
}

NativeValue* JsRuntime::SetCallbackTimer(NativeEngine& engine, NativeCallbackInfo& info, bool isInterval)
{
    // parameter check, must have at least 2 params
    if (info.argc < 2 || info.argv[0]->TypeOf() != NATIVE_FUNCTION || info.argv[1]->TypeOf() != NATIVE_NUMBER) {
        HILOG_ERROR("Set callback timer failed with invalid parameter.");
        return engine.CreateUndefined();
    }

    // parse parameter
    std::shared_ptr<NativeReference> jsFunction(engine.CreateReference(info.argv[0], 1));
    int64_t delayTime = *ConvertNativeValueTo<NativeNumber>(info.argv[1]);
    uint32_t callbackId = callbackId_++;
    std::string name = "JsRuntimeTimer_";
    name.append(std::to_string(callbackId));

    // create timer task
    TimerTask task(*this, jsFunction, name, isInterval ? delayTime : 0);
    for (size_t index = 2; index < info.argc; ++index) {
        task.PushArgs(std::shared_ptr<NativeReference>(engine.CreateReference(info.argv[index], 1)));
    }

    JsRuntime::PostTask(task, name, delayTime);
    return engine.CreateNumber(callbackId);
}

NativeValue* JsRuntime::ClearCallbackTimer(NativeEngine& engine, NativeCallbackInfo& info)
{
    // parameter check, must have at least 1 param
    if (info.argc < 1 || info.argv[0]->TypeOf() != NATIVE_NUMBER) {
        HILOG_ERROR("Clear callback timer failed with invalid parameter.");
        return engine.CreateUndefined();
    }

    uint32_t callbackId = *ConvertNativeValueTo<NativeNumber>(info.argv[0]);
    std::string name = "JsRuntimeTimer_";
    name.append(std::to_string(callbackId));

    // event should be cancelable before executed
    JsRuntime::RemoveTask(name);
    return engine.CreateUndefined();
}

std::string JsRuntime::BuildNativeAndJsBackStackTrace()
{
    std::string straceStr = "";
    [[maybe_unused]]bool temp = nativeEngine_->BuildNativeAndJsBackStackTrace(straceStr);
    return straceStr;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
