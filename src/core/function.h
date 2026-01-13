/**
 * =============================================================================
 * Source Python
 * Copyright (C) 2012-2015 Source Python Development Team.  All rights reserved.
 * =============================================================================
 *
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
 * This file has been modified from its original form, under the GNU General
 * Public License, version 3.0.
 */

#pragma once

#include "khook.hpp"
#include "scripting/callback_manager.h"
#include "scripting/script_engine.h"
#include <map>
#include <optional>

namespace counterstrikesharp {

enum DataType_t
{
    DATA_TYPE_VOID,
    DATA_TYPE_BOOL,
    DATA_TYPE_CHAR,
    DATA_TYPE_UCHAR,
    DATA_TYPE_SHORT,
    DATA_TYPE_USHORT,
    DATA_TYPE_INT,
    DATA_TYPE_UINT,
    DATA_TYPE_LONG,
    DATA_TYPE_ULONG,
    DATA_TYPE_LONG_LONG,
    DATA_TYPE_ULONG_LONG,
    DATA_TYPE_FLOAT,
    DATA_TYPE_DOUBLE,
    DATA_TYPE_POINTER,
    DATA_TYPE_STRING,
    DATA_TYPE_VARIANT
};

enum Protection_t
{
    PROTECTION_NONE,
    PROTECTION_READ,
    PROTECTION_READ_WRITE,
    PROTECTION_EXECUTE,
    PROTECTION_EXECUTE_READ,
    PROTECTION_EXECUTE_READ_WRITE
};

enum Convention_t
{
    CONV_CUSTOM,
    CONV_CDECL,
    CONV_THISCALL,
    CONV_STDCALL,
    CONV_FASTCALL
};

struct KHookWrapper
{
    void* originalFunc = nullptr;

    std::vector<uint64_t> argStorage;
    std::vector<void*> args;

    uint64_t returnStorage{};
    void* returnValue = nullptr;
};

struct HookHolder
{
    std::unique_ptr<KHook::__Hook> obj;
    void (*configure)(KHook::__Hook*, void*) = nullptr;

    explicit operator bool() const { return obj != nullptr; }
};

class ValveFunction
{
  public:
    ValveFunction(void* addr, Convention_t conv, std::vector<DataType_t> args, DataType_t ret);

    ~ValveFunction();

    bool IsCallable();

    void SetOffset(int offset) { m_offset = offset; }
    void SetSignature(const char* sig) { m_signature = sig; }

    void Call(ScriptContext& ctx, int offset = 0, bool bypass = false);

    void AddHook(const std::function<HookResult(HookMode, KHookWrapper&)>& cb);
    void AddHook(CallbackT callable, bool post);
    void RemoveHook(CallbackT callable, bool post);

    template <typename R, typename... A> KHook::Return<R> OnPre(A... args);
    template <typename R, typename... A> KHook::Return<R> OnPost(A... args);

  public:
    void* m_ulAddr{};
    void* m_trampoline{};

    std::vector<DataType_t> m_Args;
    DataType_t m_eReturnType{};

    Convention_t m_eCallingConvention{};
    int m_iCallingConvention{};
    int m_offset{};
    const char* m_signature{};

    ScriptCallback* m_precallback = nullptr;
    ScriptCallback* m_postcallback = nullptr;

    std::optional<std::function<HookResult(HookMode, KHookWrapper&)>> m_callback;
    std::vector<HookResult> m_lastPreHookResult;

    KHookWrapper m_runtimeHook;
    HookHolder m_khook;
};

} // namespace counterstrikesharp
