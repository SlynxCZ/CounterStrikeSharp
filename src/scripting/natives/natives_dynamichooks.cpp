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

template <typename T> static T LoadValue(const void* ptr)
{
    T out{};
    std::memcpy(&out, ptr, sizeof(T));
    return out;
}

template <typename T> static void StoreParam(KHookWrapper* h, int idx, T value)
{
    static_assert(sizeof(T) <= sizeof(uint64_t));

    if (h->argStorage.size() <= idx) h->argStorage.resize(idx + 1);

    std::memcpy(&h->argStorage[idx], &value, sizeof(T));
    h->args[idx] = &h->argStorage[idx];
}

void DHookGetReturn(ScriptContext& ctx)
{
    auto type = ctx.GetArgument<DataType_t>(1);

    void* val = KHook::GetCurrentValuePtr(true);
    if (!val) ctx.ThrowNativeError("No return value");

    switch (type)
    {
        case DATA_TYPE_BOOL:
            ctx.SetResult(*(bool*)val);
            break;
        case DATA_TYPE_CHAR:
            ctx.SetResult(*(char*)val);
            break;
        case DATA_TYPE_UCHAR:
            ctx.SetResult(*(unsigned char*)val);
            break;
        case DATA_TYPE_SHORT:
            ctx.SetResult(*(short*)val);
            break;
        case DATA_TYPE_USHORT:
            ctx.SetResult(*(unsigned short*)val);
            break;
        case DATA_TYPE_INT:
            ctx.SetResult(*(int*)val);
            break;
        case DATA_TYPE_UINT:
            ctx.SetResult(*(unsigned int*)val);
            break;
        case DATA_TYPE_LONG:
            ctx.SetResult(*(long*)val);
            break;
        case DATA_TYPE_ULONG:
            ctx.SetResult(*(unsigned long*)val);
            break;
        case DATA_TYPE_LONG_LONG:
            ctx.SetResult(*(long long*)val);
            break;
        case DATA_TYPE_ULONG_LONG:
            ctx.SetResult(*(unsigned long long*)val);
            break;
        case DATA_TYPE_FLOAT:
            ctx.SetResult(*(float*)val);
            break;
        case DATA_TYPE_DOUBLE:
            ctx.SetResult(*(double*)val);
            break;

        case DATA_TYPE_POINTER:
        case DATA_TYPE_STRING:
            ctx.SetResult(*(void**)val);
            break;

        default:
            ctx.ThrowNativeError("Unknown return type");
    }
}

void DHookSetReturn(ScriptContext& ctx)
{
    auto type = ctx.GetArgument<DataType_t>(1);
    int idx = 2;

#define SAVE(T)                                                                                                          \
    {                                                                                                                    \
        T v = ctx.GetArgument<T>(idx);                                                                                   \
        KHook::SaveReturnValue(KHook::Action::Override, &v, sizeof(v), reinterpret_cast<void*>(KHook::init_operator<T>), \
                               reinterpret_cast<void*>(KHook::deinit_operator<T>), false);                               \
    }

    switch (type)
    {
        case DATA_TYPE_BOOL:
            SAVE(bool);
            break;
        case DATA_TYPE_CHAR:
            SAVE(char);
            break;
        case DATA_TYPE_UCHAR:
            SAVE(unsigned char);
            break;
        case DATA_TYPE_SHORT:
            SAVE(short);
            break;
        case DATA_TYPE_USHORT:
            SAVE(unsigned short);
            break;
        case DATA_TYPE_INT:
            SAVE(int);
            break;
        case DATA_TYPE_UINT:
            SAVE(unsigned int);
            break;
        case DATA_TYPE_LONG:
            SAVE(long);
            break;
        case DATA_TYPE_ULONG:
            SAVE(unsigned long);
            break;
        case DATA_TYPE_LONG_LONG:
            SAVE(long long);
            break;
        case DATA_TYPE_ULONG_LONG:
            SAVE(unsigned long long);
            break;
        case DATA_TYPE_FLOAT:
            SAVE(float);
            break;
        case DATA_TYPE_DOUBLE:
            SAVE(double);
            break;

        case DATA_TYPE_POINTER:
        {
            void* v = ctx.GetArgument<void*>(idx);
            KHook::SaveReturnValue(KHook::Action::Override, &v, sizeof(void*), (void*)KHook::init_operator<void*>,
                                   (void*)KHook::deinit_operator<void*>, false);
            break;
        }

        case DATA_TYPE_STRING:
        {
            const char* v = ctx.GetArgument<const char*>(idx);
            KHook::SaveReturnValue(KHook::Action::Override, &v, sizeof(const char*), (void*)KHook::init_operator<const char*>,
                                   (void*)KHook::deinit_operator<const char*>, false);
            break;
        }

        default:
            ctx.ThrowNativeError("Unknown return type");
    }
#undef SAVE
}

void DHookGetParam(ScriptContext& ctx)
{
    auto hook = ctx.GetArgument<KHookWrapper*>(0);
    auto type = ctx.GetArgument<DataType_t>(1);
    int index = ctx.GetArgument<int>(2);

    if (!hook) ctx.ThrowNativeError("Invalid hook");

    if (index < 0 || index >= hook->args.size()) ctx.ThrowNativeError("Param index out of range");

    void* val = hook->args[index];

    switch (type)
    {
        case DATA_TYPE_BOOL:
            ctx.SetResult(LoadValue<bool>(val));
            break;
        case DATA_TYPE_CHAR:
            ctx.SetResult(LoadValue<char>(val));
            break;
        case DATA_TYPE_UCHAR:
            ctx.SetResult(LoadValue<unsigned char>(val));
            break;
        case DATA_TYPE_SHORT:
            ctx.SetResult(LoadValue<short>(val));
            break;
        case DATA_TYPE_USHORT:
            ctx.SetResult(LoadValue<unsigned short>(val));
            break;
        case DATA_TYPE_INT:
            ctx.SetResult(LoadValue<int>(val));
            break;
        case DATA_TYPE_UINT:
            ctx.SetResult(LoadValue<unsigned int>(val));
            break;
        case DATA_TYPE_LONG:
            ctx.SetResult(LoadValue<long>(val));
            break;
        case DATA_TYPE_ULONG:
            ctx.SetResult(LoadValue<unsigned long>(val));
            break;
        case DATA_TYPE_LONG_LONG:
            ctx.SetResult(LoadValue<long long>(val));
            break;
        case DATA_TYPE_ULONG_LONG:
            ctx.SetResult(LoadValue<unsigned long long>(val));
            break;
        case DATA_TYPE_FLOAT:
            ctx.SetResult(LoadValue<float>(val));
            break;
        case DATA_TYPE_DOUBLE:
            ctx.SetResult(LoadValue<double>(val));
            break;

        case DATA_TYPE_STRING:
            ctx.SetResult((const char*)val);
            break;

        case DATA_TYPE_POINTER:
            ctx.SetResult(val);
            break;

        default:
            ctx.ThrowNativeError("Unknown param type");
    }
}

void DHookSetParam(ScriptContext& ctx)
{
    auto hook = ctx.GetArgument<KHookWrapper*>(0);
    auto type = ctx.GetArgument<DataType_t>(1);
    int index = ctx.GetArgument<int>(2);

    if (!hook) ctx.ThrowNativeError("Invalid hook");

    if (index < 0 || index >= hook->args.size()) ctx.ThrowNativeError("Param index out of range");

    int idx = 3;

    switch (type)
    {
        case DATA_TYPE_BOOL:
            StoreParam(hook, index, ctx.GetArgument<bool>(idx));
            break;
        case DATA_TYPE_CHAR:
            StoreParam(hook, index, ctx.GetArgument<char>(idx));
            break;
        case DATA_TYPE_UCHAR:
            StoreParam(hook, index, ctx.GetArgument<unsigned char>(idx));
            break;
        case DATA_TYPE_SHORT:
            StoreParam(hook, index, ctx.GetArgument<short>(idx));
            break;
        case DATA_TYPE_USHORT:
            StoreParam(hook, index, ctx.GetArgument<unsigned short>(idx));
            break;
        case DATA_TYPE_INT:
            StoreParam(hook, index, ctx.GetArgument<int>(idx));
            break;
        case DATA_TYPE_UINT:
            StoreParam(hook, index, ctx.GetArgument<unsigned int>(idx));
            break;
        case DATA_TYPE_LONG:
            StoreParam(hook, index, ctx.GetArgument<long>(idx));
            break;
        case DATA_TYPE_ULONG:
            StoreParam(hook, index, ctx.GetArgument<unsigned long>(idx));
            break;
        case DATA_TYPE_LONG_LONG:
            StoreParam(hook, index, ctx.GetArgument<long long>(idx));
            break;
        case DATA_TYPE_ULONG_LONG:
            StoreParam(hook, index, ctx.GetArgument<unsigned long long>(idx));
            break;
        case DATA_TYPE_FLOAT:
            StoreParam(hook, index, ctx.GetArgument<float>(idx));
            break;
        case DATA_TYPE_DOUBLE:
            StoreParam(hook, index, ctx.GetArgument<double>(idx));
            break;
        case DATA_TYPE_STRING:
            StoreParam(hook, index, ctx.GetArgument<const char*>(idx));
            break;

        case DATA_TYPE_POINTER:
            hook->args[index] = ctx.GetArgument<void*>(idx);
            break;

        default:
            ctx.ThrowNativeError("Unknown param type");
    }
}

REGISTER_NATIVES(dynamichooks, {
    ScriptEngine::RegisterNativeHandler("DYNAMIC_HOOK_GET_RETURN", DHookGetReturn);
    ScriptEngine::RegisterNativeHandler("DYNAMIC_HOOK_SET_RETURN", DHookSetReturn);
    ScriptEngine::RegisterNativeHandler("DYNAMIC_HOOK_GET_PARAM", DHookGetParam);
    ScriptEngine::RegisterNativeHandler("DYNAMIC_HOOK_SET_PARAM", DHookSetParam);
})
} // namespace counterstrikesharp
