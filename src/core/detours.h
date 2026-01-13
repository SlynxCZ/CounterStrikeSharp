#pragma once

#include "core/log.h"
#include "core/function.h"
#include "core/globals.h"
#include "core/managers/entity_manager.h"

#include "scripting/script_engine.h"

namespace counterstrikesharp {

inline HookResult OnTakeDamageProxy(HookMode mode, KHookWrapper& hook)
{
    auto* pThis = reinterpret_cast<CBaseEntity*>(hook.args[0]);
    auto* pInfo = reinterpret_cast<CTakeDamageInfo*>(hook.args[1]);
    auto* pResult = reinterpret_cast<CTakeDamageResult*>(hook.args[2]);

    if (mode == HookMode::Pre)
    {
        if (!globals::entityManager.Hook_OnTakeDamage_Alive_Pre(pThis, pInfo, pResult))
        {
            int v = 1;
            KHook::SaveReturnValue(KHook::Action::Supersede, &v, sizeof(v), reinterpret_cast<void*>(KHook::init_operator<int>),
                                   reinterpret_cast<void*>(KHook::deinit_operator<int>), false);
            return HookResult::Handled;
        }
    }
    else
    {
        globals::entityManager.Hook_OnTakeDamage_Alive_Post(pThis, pInfo, pResult);
    }

    return HookResult::Continue;
}

} // namespace counterstrikesharp
