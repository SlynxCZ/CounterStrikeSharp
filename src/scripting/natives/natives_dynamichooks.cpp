/*
 *  This file is part of CounterStrikeSharp.
 *  CounterStrikeSharp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CounterStrikeSharp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CounterStrikeSharp.  If not, see <https://www.gnu.org/licenses/>. *
 */

#include "mm_plugin.h"
#include "core/timer_system.h"
#include "scripting/autonative.h"
#include "scripting/script_engine.h"
#include "core/function.h"
#include <cstring>

namespace counterstrikesharp {

template<typename T>
static void StoreValue(KHookWrapper* h, T value)
{
    static_assert(sizeof(T) <= sizeof(uint64_t));
    std::memcpy(&h->returnStorage, &value, sizeof(T));
    h->returnValue = &h->returnStorage;
}

template<typename T>
static T LoadValue(const void* ptr)
{
    T out{};
    std::memcpy(&out, ptr, sizeof(T));
    return out;
}

template<typename T>
static void StoreParam(KHookWrapper* h, int idx, T value)
{
    static_assert(sizeof(T) <= sizeof(uint64_t));
    if (h->argStorage.size() <= idx)
        h->argStorage.resize(idx + 1);
    std::memcpy(&h->argStorage[idx], &value, sizeof(T));
    h->args[idx] = &h->argStorage[idx];
}

void DHookGetReturn(ScriptContext& script_context)
{
    auto hook = script_context.GetArgument<KHookWrapper*>(0);
    auto type = script_context.GetArgument<DataType_t>(1);

    if (!hook)
        script_context.ThrowNativeError("Invalid hook");

    switch (type)
    {
        case DATA_TYPE_BOOL:   script_context.SetResult(LoadValue<bool>(hook->returnValue)); break;
        case DATA_TYPE_CHAR:   script_context.SetResult(LoadValue<char>(hook->returnValue)); break;
        case DATA_TYPE_UCHAR:  script_context.SetResult(LoadValue<unsigned char>(hook->returnValue)); break;
        case DATA_TYPE_SHORT:  script_context.SetResult(LoadValue<short>(hook->returnValue)); break;
        case DATA_TYPE_USHORT: script_context.SetResult(LoadValue<unsigned short>(hook->returnValue)); break;
        case DATA_TYPE_INT:    script_context.SetResult(LoadValue<int>(hook->returnValue)); break;
        case DATA_TYPE_UINT:   script_context.SetResult(LoadValue<unsigned int>(hook->returnValue)); break;
        case DATA_TYPE_LONG:   script_context.SetResult(LoadValue<long>(hook->returnValue)); break;
        case DATA_TYPE_ULONG:  script_context.SetResult(LoadValue<unsigned long>(hook->returnValue)); break;
        case DATA_TYPE_LONG_LONG:  script_context.SetResult(LoadValue<long long>(hook->returnValue)); break;
        case DATA_TYPE_ULONG_LONG: script_context.SetResult(LoadValue<unsigned long long>(hook->returnValue)); break;
        case DATA_TYPE_FLOAT:  script_context.SetResult(LoadValue<float>(hook->returnValue)); break;
        case DATA_TYPE_DOUBLE: script_context.SetResult(LoadValue<double>(hook->returnValue)); break;

        case DATA_TYPE_POINTER:
        case DATA_TYPE_STRING:
            script_context.SetResult(hook->returnValue);
            break;

        default:
            assert(!"Unknown return type");
    }
}

void DHookSetReturn(ScriptContext& script_context)
{
    auto hook = script_context.GetArgument<KHookWrapper*>(0);
    auto type = script_context.GetArgument<DataType_t>(1);

    if (!hook)
        script_context.ThrowNativeError("Invalid hook");

    int idx = 2;

    switch (type)
    {
        case DATA_TYPE_BOOL:   StoreValue(hook, script_context.GetArgument<bool>(idx)); break;
        case DATA_TYPE_CHAR:   StoreValue(hook, script_context.GetArgument<char>(idx)); break;
        case DATA_TYPE_UCHAR:  StoreValue(hook, script_context.GetArgument<unsigned char>(idx)); break;
        case DATA_TYPE_SHORT:  StoreValue(hook, script_context.GetArgument<short>(idx)); break;
        case DATA_TYPE_USHORT: StoreValue(hook, script_context.GetArgument<unsigned short>(idx)); break;
        case DATA_TYPE_INT:    StoreValue(hook, script_context.GetArgument<int>(idx)); break;
        case DATA_TYPE_UINT:   StoreValue(hook, script_context.GetArgument<unsigned int>(idx)); break;
        case DATA_TYPE_LONG:   StoreValue(hook, script_context.GetArgument<long>(idx)); break;
        case DATA_TYPE_ULONG:  StoreValue(hook, script_context.GetArgument<unsigned long>(idx)); break;
        case DATA_TYPE_LONG_LONG:  StoreValue(hook, script_context.GetArgument<long long>(idx)); break;
        case DATA_TYPE_ULONG_LONG: StoreValue(hook, script_context.GetArgument<unsigned long long>(idx)); break;
        case DATA_TYPE_FLOAT:  StoreValue(hook, script_context.GetArgument<float>(idx)); break;
        case DATA_TYPE_DOUBLE: StoreValue(hook, script_context.GetArgument<double>(idx)); break;
        case DATA_TYPE_STRING: hook->returnValue = (void*)script_context.GetArgument<const char*>(idx); break;

        case DATA_TYPE_POINTER:
            hook->returnValue = script_context.GetArgument<void*>(idx);
            break;

        default:
            assert(!"Unknown return type");
    }
}

void DHookGetParam(ScriptContext& script_context)
{
    auto hook = script_context.GetArgument<KHookWrapper*>(0);
    auto type = script_context.GetArgument<DataType_t>(1);
    int index = script_context.GetArgument<int>(2);

    if (!hook)
        script_context.ThrowNativeError("Invalid hook");

    if (index < 0 || index >= hook->args.size())
    {
        script_context.ThrowNativeError("Param index out of range");
    }

    void* val = hook->args[index];

    switch (type)
    {
        case DATA_TYPE_BOOL:   script_context.SetResult(LoadValue<bool>(val)); break;
        case DATA_TYPE_CHAR:   script_context.SetResult(LoadValue<char>(val)); break;
        case DATA_TYPE_UCHAR:  script_context.SetResult(LoadValue<unsigned char>(val)); break;
        case DATA_TYPE_SHORT:  script_context.SetResult(LoadValue<short>(val)); break;
        case DATA_TYPE_USHORT: script_context.SetResult(LoadValue<unsigned short>(val)); break;
        case DATA_TYPE_INT:    script_context.SetResult(LoadValue<int>(val)); break;
        case DATA_TYPE_UINT:   script_context.SetResult(LoadValue<unsigned int>(val)); break;
        case DATA_TYPE_LONG:   script_context.SetResult(LoadValue<long>(val)); break;
        case DATA_TYPE_ULONG:  script_context.SetResult(LoadValue<unsigned long>(val)); break;
        case DATA_TYPE_LONG_LONG:  script_context.SetResult(LoadValue<long long>(val)); break;
        case DATA_TYPE_ULONG_LONG: script_context.SetResult(LoadValue<unsigned long long>(val)); break;
        case DATA_TYPE_FLOAT:  script_context.SetResult(LoadValue<float>(val)); break;
        case DATA_TYPE_DOUBLE: script_context.SetResult(LoadValue<double>(val)); break;
        case DATA_TYPE_STRING: script_context.SetResult((const char*)val); break;

        case DATA_TYPE_POINTER:
            script_context.SetResult(val);
            break;

        default:
            assert(!"Unknown param type");
    }
}

void DHookSetParam(ScriptContext& script_context)
{
    auto hook = script_context.GetArgument<KHookWrapper*>(0);
    auto type = script_context.GetArgument<DataType_t>(1);
    int index = script_context.GetArgument<int>(2);

    if (!hook)
        script_context.ThrowNativeError("Invalid hook");

    if (index < 0 || index >= hook->args.size())
    {
        script_context.ThrowNativeError("Param index out of range");
    }

    int idx = 3;

    switch (type)
    {
        case DATA_TYPE_BOOL:   StoreParam(hook, index, script_context.GetArgument<bool>(idx)); break;
        case DATA_TYPE_CHAR:   StoreParam(hook, index, script_context.GetArgument<char>(idx)); break;
        case DATA_TYPE_UCHAR:  StoreParam(hook, index, script_context.GetArgument<unsigned char>(idx)); break;
        case DATA_TYPE_SHORT:  StoreParam(hook, index, script_context.GetArgument<short>(idx)); break;
        case DATA_TYPE_USHORT: StoreParam(hook, index, script_context.GetArgument<unsigned short>(idx)); break;
        case DATA_TYPE_INT:    StoreParam(hook, index, script_context.GetArgument<int>(idx)); break;
        case DATA_TYPE_UINT:   StoreParam(hook, index, script_context.GetArgument<unsigned int>(idx)); break;
        case DATA_TYPE_LONG:   StoreParam(hook, index, script_context.GetArgument<long>(idx)); break;
        case DATA_TYPE_ULONG:  StoreParam(hook, index, script_context.GetArgument<unsigned long>(idx)); break;
        case DATA_TYPE_LONG_LONG:  StoreParam(hook, index, script_context.GetArgument<long long>(idx)); break;
        case DATA_TYPE_ULONG_LONG: StoreParam(hook, index, script_context.GetArgument<unsigned long long>(idx)); break;
        case DATA_TYPE_FLOAT:  StoreParam(hook, index, script_context.GetArgument<float>(idx)); break;
        case DATA_TYPE_DOUBLE: StoreParam(hook, index, script_context.GetArgument<double>(idx)); break;
        case DATA_TYPE_STRING: StoreParam(hook, index, script_context.GetArgument<const char *>(idx)); break;
        case DATA_TYPE_POINTER:
            hook->args[index] = script_context.GetArgument<void*>(idx);
            break;

        default:
            assert(!"Unknown param type");
    }
}

REGISTER_NATIVES(dynamichooks, {
    ScriptEngine::RegisterNativeHandler("DYNAMIC_HOOK_GET_RETURN", DHookGetReturn);
    ScriptEngine::RegisterNativeHandler("DYNAMIC_HOOK_SET_RETURN", DHookSetReturn);
    ScriptEngine::RegisterNativeHandler("DYNAMIC_HOOK_GET_PARAM", DHookGetParam);
    ScriptEngine::RegisterNativeHandler("DYNAMIC_HOOK_SET_PARAM", DHookSetParam);
})
} // namespace counterstrikesharp
