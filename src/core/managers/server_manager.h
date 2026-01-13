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

#pragma once

#include "khook.hpp"
#include "core/globals.h"
#include "core/global_listener.h"
#include "scripting/script_engine.h"

#include "core/game_system.h"

namespace counterstrikesharp {
class ScriptCallback;

class ServerManager : public GlobalClass
{
  public:
    ServerManager();
    ~ServerManager();
    void OnAllInitialized() override;
    void OnShutdown() override;
    void* GetEconItemSystem();
    bool IsPaused();
    void OnPrecacheResources(IEntityResourceManifest* pResourceManifest);

    ScriptCallback* on_server_pre_entity_think;
    ScriptCallback* on_server_post_entity_think;

    KHook::Return<void> Hook_ServerHibernationUpdate(ISource2Server* pThis, bool);
    KHook::Return<void> Hook_GameServerSteamAPIActivated(ISource2Server* pThis);
    KHook::Return<void> Hook_GameServerSteamAPIDeactivated(ISource2Server* pThis);
    KHook::Return<void> Hook_OnHostNameChanged(ISource2Server* pThis, const char*);
    KHook::Return<void> Hook_PreFatalShutdown(const ISource2Server* pThis);
    KHook::Return<void> Hook_UpdateWhenNotInGame(ISource2Server* pThis, float);
    KHook::Return<void> Hook_PreWorldUpdate(ISource2Server* pThis, bool);

  private:
    ScriptCallback* on_server_hibernation_update_callback;
    ScriptCallback* on_server_steam_api_activated_callback;
    ScriptCallback* on_server_steam_api_deactivated_callback;
    ScriptCallback* on_server_hostname_changed_callback;
    ScriptCallback* on_server_pre_fatal_shutdown;
    ScriptCallback* on_server_update_when_not_in_game;
    ScriptCallback* on_server_pre_world_update;

    ScriptCallback* on_server_precache_resources;
};

} // namespace counterstrikesharp
