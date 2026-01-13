/**
 * =============================================================================
 * Source Python
 * Copyright (C) 2012-2015 Source Python Development Team.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, the Source Python Team gives you permission
 * to link the code of this program (as well as its derivative works) to
 * "Half-Life 2," the "Source Engine," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, the Source.Python
 * Development Team grants this exception to all derivative works.
 *
 * This file has been modified from its original form, under the terms of GNU
 * General Public License, version 3.0.
 */

#include "core/function.h"
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <algorithm>
#include "core/log.h"
#include "dyncall/dyncall/dyncall.h"

namespace counterstrikesharp {

static DCCallVM* g_pCallVM = dcNewCallVM(4096);

int GetDynCallConvention(Convention_t c)
{
    switch (c)
    {
        case CONV_CDECL:
            return DC_CALL_C_DEFAULT;
        case CONV_THISCALL:
#ifdef _WIN32
            return DC_CALL_C_X86_WIN32_THIS_MS;
#else
            return DC_CALL_C_X86_WIN32_THIS_GNU;
#endif
#ifdef _WIN32
        case CONV_STDCALL:
            return DC_CALL_C_X86_WIN32_STD;
        case CONV_FASTCALL:
            return DC_CALL_C_X86_WIN32_FAST_MS;
#endif
        default:
            return -1;
    }
}

template <DataType_t T> struct ToCpp;

template <> struct ToCpp<DATA_TYPE_VOID>
{
    using type = void;
};
template <> struct ToCpp<DATA_TYPE_BOOL>
{
    using type = bool;
};
template <> struct ToCpp<DATA_TYPE_CHAR>
{
    using type = char;
};
template <> struct ToCpp<DATA_TYPE_UCHAR>
{
    using type = unsigned char;
};
template <> struct ToCpp<DATA_TYPE_SHORT>
{
    using type = short;
};
template <> struct ToCpp<DATA_TYPE_USHORT>
{
    using type = unsigned short;
};
template <> struct ToCpp<DATA_TYPE_INT>
{
    using type = int;
};
template <> struct ToCpp<DATA_TYPE_UINT>
{
    using type = unsigned int;
};
template <> struct ToCpp<DATA_TYPE_LONG>
{
    using type = long;
};
template <> struct ToCpp<DATA_TYPE_ULONG>
{
    using type = unsigned long;
};
template <> struct ToCpp<DATA_TYPE_LONG_LONG>
{
    using type = long long;
};
template <> struct ToCpp<DATA_TYPE_ULONG_LONG>
{
    using type = unsigned long long;
};
template <> struct ToCpp<DATA_TYPE_FLOAT>
{
    using type = float;
};
template <> struct ToCpp<DATA_TYPE_DOUBLE>
{
    using type = double;
};
template <> struct ToCpp<DATA_TYPE_POINTER>
{
    using type = void*;
};
template <> struct ToCpp<DATA_TYPE_STRING>
{
    using type = const char*;
};
template <> struct ToCpp<DATA_TYPE_VARIANT>
{
    using type = void*;
};

template <typename T> HookHolder MakeHook(std::unique_ptr<T> ptr)
{
    HookHolder h;
    h.configure = [](KHook::__Hook* base, void* addr) {
        static_cast<T*>(base)->Configure(addr);
    };
    h.obj = std::move(ptr);
    return h;
}

template <DataType_t RET, size_t IDX, DataType_t... BUILT> struct HookBuilder
{
    static HookHolder Build(ValveFunction* self, const std::vector<DataType_t>& args)
    {
        if constexpr (IDX == sizeof...(BUILT))
        {
            using R = typename ToCpp<RET>::type;

            using HookT = KHook::Function<R, typename ToCpp<BUILT>::type...>;

            return MakeHook(std::make_unique<HookT>(self, &ValveFunction::OnPre<R, typename ToCpp<BUILT>::type...>,
                                                    &ValveFunction::OnPost<R, typename ToCpp<BUILT>::type...>));
        }
        else
        {
            if (IDX >= args.size()) return {};

            switch (args[IDX])
            {
                case DATA_TYPE_INT:
                    return HookBuilder<RET, IDX + 1, BUILT..., DATA_TYPE_INT>::Build(self, args);

                case DATA_TYPE_FLOAT:
                    return HookBuilder<RET, IDX + 1, BUILT..., DATA_TYPE_FLOAT>::Build(self, args);

                case DATA_TYPE_DOUBLE:
                    return HookBuilder<RET, IDX + 1, BUILT..., DATA_TYPE_DOUBLE>::Build(self, args);

                case DATA_TYPE_POINTER:
                    return HookBuilder<RET, IDX + 1, BUILT..., DATA_TYPE_POINTER>::Build(self, args);

                case DATA_TYPE_STRING:
                    return HookBuilder<RET, IDX + 1, BUILT..., DATA_TYPE_STRING>::Build(self, args);

                default:
                    return {};
            }
        }
    }
};

template <typename... A> static void FillArgs(KHookWrapper& h, A... args)
{
    h.args.clear();
    h.argStorage.clear();

    constexpr size_t count = sizeof...(A);
    h.argStorage.resize(count);

    size_t i = 0;
    ((std::memcpy(&h.argStorage[i], &args, sizeof(args)), h.args.push_back(&h.argStorage[i]), ++i), ...);
}

ValveFunction::ValveFunction(void* addr, Convention_t conv, std::vector<DataType_t> args, DataType_t ret)
{
    m_ulAddr = addr;
    m_Args = std::move(args);
    m_eReturnType = ret;

    m_eCallingConvention = conv;
    m_iCallingConvention = GetDynCallConvention(conv);
}

ValveFunction::~ValveFunction()
{
    if (m_precallback) globals::callbackManager.ReleaseCallback(m_precallback);
    if (m_postcallback) globals::callbackManager.ReleaseCallback(m_postcallback);
}

bool ValveFunction::IsCallable() { return m_iCallingConvention != -1; }

template <class R, class FN> static R CallHelper(FN fn, DCCallVM* vm, void* addr) { return (R)fn(vm, addr); }

void ValveFunction::Call(ScriptContext& ctx, int offset, bool bypass)
{
    if (!IsCallable()) return;

    dcReset(g_pCallVM);
    dcMode(g_pCallVM, m_iCallingConvention);

    for (size_t i = 0; i < m_Args.size(); i++)
    {
        int idx = i + offset;

        switch (m_Args[i])
        {
            case DATA_TYPE_INT:
                dcArgInt(g_pCallVM, ctx.GetArgument<int>(idx));
                break;
            case DATA_TYPE_FLOAT:
                dcArgFloat(g_pCallVM, ctx.GetArgument<float>(idx));
                break;
            case DATA_TYPE_DOUBLE:
                dcArgDouble(g_pCallVM, ctx.GetArgument<double>(idx));
                break;
            case DATA_TYPE_POINTER:
                dcArgPointer(g_pCallVM, ctx.GetArgument<void*>(idx));
                break;
            case DATA_TYPE_STRING:
                dcArgPointer(g_pCallVM, (void*)ctx.GetArgument<const char*>(idx));
                break;
            default:
                break;
        }
    }

    void* target = bypass && m_trampoline ? m_trampoline : m_ulAddr;

    switch (m_eReturnType)
    {
        case DATA_TYPE_VOID:
            dcCallVoid(g_pCallVM, target);
            break;
        case DATA_TYPE_INT:
            ctx.SetResult(CallHelper<int>(dcCallInt, g_pCallVM, target));
            break;
        case DATA_TYPE_FLOAT:
            ctx.SetResult(CallHelper<float>(dcCallFloat, g_pCallVM, target));
            break;
        case DATA_TYPE_DOUBLE:
            ctx.SetResult(CallHelper<double>(dcCallDouble, g_pCallVM, target));
            break;
        case DATA_TYPE_POINTER:
            ctx.SetResult(CallHelper<void*>(dcCallPointer, g_pCallVM, target));
            break;
        default:
            break;
    }
}

void ValveFunction::AddHook(const std::function<HookResult(HookMode, KHookWrapper&)>& cb)
{
    m_callback = cb;

    if (!m_khook)
    {
        switch (m_eReturnType)
        {
            case DATA_TYPE_VOID:
                m_khook = HookBuilder<DATA_TYPE_VOID, 0>::Build(this, m_Args);
                break;

            case DATA_TYPE_INT:
                m_khook = HookBuilder<DATA_TYPE_INT, 0>::Build(this, m_Args);
                break;

            case DATA_TYPE_FLOAT:
                m_khook = HookBuilder<DATA_TYPE_FLOAT, 0>::Build(this, m_Args);
                break;

            case DATA_TYPE_DOUBLE:
                m_khook = HookBuilder<DATA_TYPE_DOUBLE, 0>::Build(this, m_Args);
                break;

            case DATA_TYPE_POINTER:
                m_khook = HookBuilder<DATA_TYPE_POINTER, 0>::Build(this, m_Args);
                break;

            default:
                CSSHARP_CORE_ERROR("Unsupported return type");
                return;
        }
    }

    if (m_khook && m_khook.configure)
    {
        m_khook.configure(m_khook.obj.get(), m_ulAddr);
    }
}

void ValveFunction::AddHook(CallbackT fn, bool post)
{
    if (!m_khook)
    {
        switch (m_eReturnType)
        {
            case DATA_TYPE_VOID:
                m_khook = HookBuilder<DATA_TYPE_VOID, 0>::Build(this, m_Args);
                break;

            case DATA_TYPE_INT:
                m_khook = HookBuilder<DATA_TYPE_INT, 0>::Build(this, m_Args);
                break;

            case DATA_TYPE_FLOAT:
                m_khook = HookBuilder<DATA_TYPE_FLOAT, 0>::Build(this, m_Args);
                break;

            case DATA_TYPE_DOUBLE:
                m_khook = HookBuilder<DATA_TYPE_DOUBLE, 0>::Build(this, m_Args);
                break;

            case DATA_TYPE_POINTER:
                m_khook = HookBuilder<DATA_TYPE_POINTER, 0>::Build(this, m_Args);
                break;

            default:
                CSSHARP_CORE_ERROR("Unsupported return type");
                return;
        }
    }

    if (!m_khook) return;

    if (m_khook && m_khook.configure)
    {
        m_khook.configure(m_khook.obj.get(), m_ulAddr);
    }

    if (post)
    {
        if (!m_postcallback) m_postcallback = globals::callbackManager.CreateCallback("");

        m_postcallback->AddListener(fn);
    }
    else
    {
        if (!m_precallback) m_precallback = globals::callbackManager.CreateCallback("");

        m_precallback->AddListener(fn);
    }
}

void ValveFunction::RemoveHook(CallbackT callable, bool post)
{
    ScriptCallback* cb = post ? m_postcallback : m_precallback;

    if (!cb)
        return;

    if (m_khook && m_khook.obj)
    {
        m_khook.obj.reset();
    }

    cb->RemoveListener(callable);

    if (cb->GetFunctionCount() == 0)
    {
        globals::callbackManager.ReleaseCallback(cb);

        if (post)
            m_postcallback = nullptr;
        else
            m_precallback = nullptr;
    }
}

template <typename R, typename... A> KHook::Return<R> ValveFunction::OnPre(A... args)
{
    FillArgs(m_runtimeHook, args...);

    m_runtimeHook.originalFunc = m_ulAddr;

    HookResult max = HookResult::Continue;

    if (m_callback) max = m_callback.value()(HookMode::Pre, m_runtimeHook);

    if (m_precallback)
    {
        m_precallback->Reset();
        m_precallback->ScriptContext().Push(&m_runtimeHook);

        for (auto fn : m_precallback->GetFunctions())
        {
            fn(&m_precallback->ScriptContextStruct());
            max = std::max(max, m_precallback->ScriptContext().GetResult<HookResult>());
        }
    }

    m_lastPreHookResult.push_back(max);

    if (max >= HookResult::Handled) return { KHook::Action::Supersede };

    return { KHook::Action::Ignore };
}

template <typename R, typename... A> KHook::Return<R> ValveFunction::OnPost(A... args)
{
    HookResult pre = HookResult::Continue;

    if (!m_lastPreHookResult.empty())
    {
        pre = m_lastPreHookResult.back();
        m_lastPreHookResult.pop_back();
    }

    if (pre >= HookResult::Handled) return { KHook::Action::Ignore };

    if (m_callback)
    {
        if (m_callback.value()(HookMode::Post, m_runtimeHook) >= HookResult::Handled) return { KHook::Action::Supersede };
    }

    if (!m_postcallback) return { KHook::Action::Ignore };

    m_postcallback->Reset();
    m_postcallback->ScriptContext().Push(&m_runtimeHook);

    HookResult max = HookResult::Continue;

    for (auto fn : m_postcallback->GetFunctions())
    {
        fn(&m_postcallback->ScriptContextStruct());
        max = std::max(max, m_postcallback->ScriptContext().GetResult<HookResult>());
    }

    if (max >= HookResult::Handled) return { KHook::Action::Supersede };

    return { KHook::Action::Ignore };
}

} // namespace counterstrikesharp
