#include <sourcemod>
#include <sdktools>

#pragma semicolon 1
#pragma newdecls required

/* ===========================================================================
 * ent_test - do thuc the cho du an entlimit_mm
 * ===========================================================================
 *
 * VIET LAI 08/08/2026 cho hop voi plugin Metamod HIEN TAI.
 *
 * === SAI LAM GOC DA SUA ===
 * Ban cu in "Total Edicts Used: N / 2048" bang cach dem IsValidEntity().
 * Do la SO ENTITY DANG SONG. Con plugin Metamod thi log `num_edicts` - MOC
 * NUOC CAO NHAT cua bo cap phat, KHONG BAO GIO GIAM. Hai dai luong khac nhau.
 * Ca du an da di sai huong hai ngay vi lan lon dung hai so nay (muc 0-SO cua
 * session_handoff.md).
 *
 *   song      = dem IsValidEntity()          <- ban cu chi in cai nay
 *   num_edicts= GetEntityCount()             <- moc nuoc cao, BAN CU THIEU
 *   trong     = num_edicts - song            <- slot da giai phong, dung lai duoc
 *   tran      = GetMaxEntities()             <- 2048
 *
 * `ED_Alloc: no free edicts` CHI no khi trong == 0 VA num_edicts == tran.
 * Nhin mot minh `song` khong bao gio doan duoc luc nao sap chet.
 *
 * === BAN VA "GHOST WEAPON" DA TAT MAC DINH ===
 * Ban cu moc round_start roi bat/tat EF_NODRAW cho moi weapon_* sau MOI lan
 * restart - ke ca sau wipe. Do la ban va cho thoi 4096 (entity o chi so >=2048
 * mat model). Nay khong con entity nao tren 2047 nen no vo dung, VA no CHE MAT
 * dung trieu chung dang can quan sat: "sau wipe vu khi con model khong".
 * => Mac dinh TAT. Bat lai bang:  sm_ent_ghostfix 1
 *
 * === LENH ===
 *   sm_ent_report              bao cao 4 dai luong
 *   sm_ent_classes [n]         xep hang n lop dong nhat (mac dinh 30)
 *   sm_ent_snap <nhan>         chup mot moc, in ra console kem nhan
 *   sm_ent_diff                DO TICH TU: lan 1 ghi moc, lan 2 in do chenh
 *   sm_ent_diff reset          xoa moc, ghi lai tu dau
 *   sm_ent_hud                 bat/tat HUD
 *   sm_ent_add [n]             sinh n info_target de ep tai (mac dinh 500)
 *   sm_ent_clear               xoa het info_target da sinh
 *   sm_ent_ghostfix [0|1]      ban va ghost weapon - MAC DINH 0
 * =========================================================================== */

#define MAX_CLASSES 256

// --- moc chuan cho sm_ent_diff: do TICH TU trong luc choi ---
// sm_ent_classes chi chup mot thoi diem. Muon biet lop nao TANG DAN thi
// phai so hai lan do. Day la bo nho cho lan do dau.
char g_BaseName[MAX_CLASSES][64];
int  g_BaseCount[MAX_CLASSES];
int  g_BaseUsed = 0;
int  g_BaseLive = 0;
bool g_HasBase  = false;
#define NET_LIMIT   2048        // tran giao thuc 11 bit

bool g_bShowHud[MAXPLAYERS + 1] = {false, ...};
bool g_bGhostFix = false;       // MAC DINH TAT - xem khoi chu thich tren

public Plugin myinfo = {
    name = "Entity Counter (entlimit_mm)",
    author = "Antigravity",
    description = "Dem edict dung cach: tach song / num_edicts / trong",
    version = "4.0",
    url = ""
};

public void OnPluginStart() {
    RegAdminCmd("sm_ent_report",   Cmd_Report,   ADMFLAG_ROOT, "Bao cao edict");
    RegAdminCmd("sm_ent_classes",  Cmd_Classes,  ADMFLAG_ROOT, "Xep hang lop dong nhat");
    RegAdminCmd("sm_ent_snap",     Cmd_Snap,     ADMFLAG_ROOT, "Chup mot moc kem nhan");
    RegAdminCmd("sm_ent_diff",     Cmd_Diff,     ADMFLAG_ROOT, "So voi moc truoc - tim lop TICH TU");
    RegAdminCmd("sm_ent_hud",      Cmd_Hud,      ADMFLAG_ROOT, "Bat/tat HUD");
    RegAdminCmd("sm_ent_add",      Cmd_Add,      ADMFLAG_ROOT, "Sinh info_target de ep tai");
    RegAdminCmd("sm_ent_clear",    Cmd_Clear,    ADMFLAG_ROOT, "Xoa info_target da sinh");
    RegAdminCmd("sm_ent_ghostfix", Cmd_GhostFix, ADMFLAG_ROOT, "Ban va ghost weapon (mac dinh TAT)");

    HookEvent("round_start",  Event_RoundStart, EventHookMode_PostNoCopy);
    HookEvent("mission_lost", Event_MissionLost, EventHookMode_PostNoCopy);
    // Bat dung khoanh khac survivor CUOI CUNG nga xuong - som hon mission_lost.
    //
    // !!  CHI HOOK SU KIEN CHAC CHAN CO TRONG L4D2.
    // Loi da mac 08/08: them "player_falldeath" (su kien cua TF2/HL2DM, L4D2
    // KHONG co) -> HookEvent nem loi -> CA PLUGIN khong nap duoc, moi lenh
    // sm_ent_* bao "Unknown command". Mot ten sai giet ca plugin.
    // => Muon them su kien moi thi kiem trong resource/modevents.res truoc.
    HookEvent("player_death",         Event_SurvivorDown);
    HookEvent("player_incapacitated", Event_SurvivorDown);

    CreateTimer(0.5, Timer_UpdateHud, _, TIMER_REPEAT);
}

/* --- Dem: tra ve so entity SONG, va so lop --- */
int CountLive() {
    int live = 0;
    int cap = GetMaxEntities();
    for (int i = 0; i < cap; i++) {
        if (IsValidEntity(i)) live++;
    }
    return live;
}

void Report(int client, const char[] tag) {
    int live  = CountLive();
    int num   = GetEntityCount();     // = sv.num_edicts, moc nuoc cao
    int cap   = GetMaxEntities();     // = gpGlobals->maxEntities
    int free  = num - live;
    if (free < 0) free = 0;

    ReplyToCommand(client, "=== EDICT [%s] ===", tag);
    ReplyToCommand(client, "  song        = %d      (IsValidEntity)", live);
    ReplyToCommand(client, "  num_edicts  = %d      (moc nuoc cao, khong giam)", num);
    ReplyToCommand(client, "  trong       = %d      (num_edicts - song)", free);
    ReplyToCommand(client, "  tran        = %d", cap);
    ReplyToCommand(client, "  bien do     = %d      (tran - song)", cap - live);

    // Canh bao dung dieu kien that: het cho khi TRONG=0 VA num_edicts cham tran
    if (free == 0 && num >= cap) {
        ReplyToCommand(client, "  *** NGUY: 0 slot trong VA num_edicts cham tran -> ED_Alloc sap bao loi ***");
    } else if (cap - live < 100) {
        ReplyToCommand(client, "  ! bien do con duoi 100");
    } else {
        ReplyToCommand(client, "  OK");
    }
    ReplyToCommand(client, "==========================");
}

public Action Cmd_Report(int client, int args) {
    Report(client, "hien tai");
    return Plugin_Handled;
}

public Action Cmd_Snap(int client, int args) {
    char tag[64];
    if (args > 0) GetCmdArgString(tag, sizeof(tag));
    else strcopy(tag, sizeof(tag), "khong nhan");
    Report(client, tag);
    // in ca ra console server de vao console.log
    int live = CountLive();
    PrintToServer("[ent_test] SNAP '%s': song=%d num_edicts=%d trong=%d tran=%d",
                  tag, live, GetEntityCount(), GetEntityCount() - live, GetMaxEntities());
    return Plugin_Handled;
}

/* --- Xep hang lop dong nhat: de doi chieu voi bang kiem ke cua plugin Metamod --- */
public Action Cmd_Classes(int client, int args) {
    int top = 30;
    if (args > 0) {
        char a[8];
        GetCmdArg(1, a, sizeof(a));
        top = StringToInt(a);
        if (top < 1)  top = 1;
        if (top > 100) top = 100;
    }

    char names[MAX_CLASSES][64];
    int  counts[MAX_CLASSES];
    int  used = 0, live = 0, unread = 0;

    int cap = GetMaxEntities();
    for (int i = 0; i < cap; i++) {
        if (!IsValidEntity(i)) continue;
        live++;

        char cls[64];
        if (!GetEntityClassname(i, cls, sizeof(cls)) || cls[0] == '\0') { unread++; continue; }

        int j = 0;
        for (; j < used; j++) {
            if (StrEqual(names[j], cls)) { counts[j]++; break; }
        }
        if (j == used && used < MAX_CLASSES) {
            strcopy(names[used], 64, cls);
            counts[used] = 1;
            used++;
        }
    }

    ReplyToCommand(client, "=== %d thuc the song, %d lop, %d khong doc duoc ===", live, used, unread);

    // sap xep chon truc tiep, giong bang kiem ke cua plugin Metamod
    for (int a = 0; a < used && a < top; a++) {
        int best = a;
        for (int b = a + 1; b < used; b++) {
            if (counts[b] > counts[best]) best = b;
        }
        if (best != a) {
            int c = counts[a]; counts[a] = counts[best]; counts[best] = c;
            char t[64];
            strcopy(t, sizeof(t), names[a]);
            strcopy(names[a], 64, names[best]);
            strcopy(names[best], 64, t);
        }
        ReplyToCommand(client, "  %5d  %s", counts[a], names[a]);
        PrintToServer("[ent_test] %5d  %s", counts[a], names[a]);
    }
    return Plugin_Handled;
}

/* --- MOC "SURVIVOR CUOI CUNG NGA XUONG" -----------------------------------
 * Gia thuyet nguoi dung: wipeclear chua bat dung khoanh khac nay.
 * mission_lost ban SAU do, va RestartRound con sau them 7 giay nua.
 * Doan giua do sinh xac / ragdoll / hieu ung => can biet no ton bao nhieu edict.
 * So sanh ba moc: DOI NGA -> mission_lost -> round_start. */
int SurvivorsStanding() {
    int n = 0;
    for (int i = 1; i <= MaxClients; i++) {
        if (!IsClientInGame(i)) continue;
        if (GetClientTeam(i) != 2) continue;          // 2 = survivor
        if (!IsPlayerAlive(i)) continue;
        if (HasEntProp(i, Prop_Send, "m_isIncapacitated") &&
            GetEntProp(i, Prop_Send, "m_isIncapacitated") != 0) continue;
        n++;
    }
    return n;
}

public void Event_SurvivorDown(Event event, const char[] name, bool dontBroadcast) {
    int cli = GetClientOfUserId(event.GetInt("userid"));
    if (cli <= 0 || !IsClientInGame(cli) || GetClientTeam(cli) != 2) return;

    int standing = SurvivorsStanding();
    int live = CountLive();
    PrintToServer("[ent_test] '%s': con %d survivor dung | song=%d num_edicts=%d trong=%d",
                  name, standing, live, GetEntityCount(), GetEntityCount() - live);

    if (standing == 0) {
        PrintToServer("[ent_test] *** DOI DA NGA HET *** song=%d num_edicts=%d trong=%d",
                      live, GetEntityCount(), GetEntityCount() - live);
    }
}

/* --- Tu chup moc quanh wipe: dung de doi chieu voi log WIPECLEAR --- */
public void Event_MissionLost(Event event, const char[] name, bool dontBroadcast) {
    int live = CountLive();
    PrintToServer("[ent_test] MOC mission_lost: song=%d num_edicts=%d trong=%d",
                  live, GetEntityCount(), GetEntityCount() - live);
}

public void Event_RoundStart(Event event, const char[] name, bool dontBroadcast) {
    int live = CountLive();
    PrintToServer("[ent_test] MOC round_start: song=%d num_edicts=%d trong=%d",
                  live, GetEntityCount(), GetEntityCount() - live);

    if (g_bGhostFix) CreateTimer(0.5, Timer_FixInvisibleWeapons);
}

/* --- Ban va ghost weapon: giu lai nhung MAC DINH TAT ---
 * Chi co nghia khi co entity o chi so >= 2048 (thoi 4096). Nay khong con.
 * Va no CHE MAT trieu chung "mat model sau wipe" dang can quan sat. */
public Action Timer_FixInvisibleWeapons(Handle timer) {
    int touched = 0;
    for (int i = 1; i < NET_LIMIT; i++) {
        if (!IsValidEntity(i)) continue;

        char cls[64];
        if (!GetEntityClassname(i, cls, sizeof(cls))) continue;

        if (strncmp(cls, "weapon_", 7) == 0 ||
            strncmp(cls, "item_", 5) == 0 ||
            StrEqual(cls, "upgrade_spawn") ||
            StrEqual(cls, "upgrade_item"))
        {
            if (HasEntProp(i, Prop_Send, "m_fEffects")) {
                int fx = GetEntProp(i, Prop_Send, "m_fEffects");
                SetEntProp(i, Prop_Send, "m_fEffects", fx | 32);    // EF_NODRAW
                SetEntProp(i, Prop_Send, "m_fEffects", fx & ~32);
                touched++;
            }
        }
    }
    PrintToServer("[ent_test] ghostfix: da cham %d thuc the", touched);
    return Plugin_Handled;
}

public Action Cmd_GhostFix(int client, int args) {
    if (args > 0) {
        char a[8];
        GetCmdArg(1, a, sizeof(a));
        g_bGhostFix = (StringToInt(a) != 0);
    }
    ReplyToCommand(client, "[ent_test] ghostfix = %s%s", g_bGhostFix ? "BAT" : "TAT",
        g_bGhostFix ? "  (CANH BAO: se che mat loi mat model khi test wipe)" : "");
    return Plugin_Handled;
}

public Action Cmd_Add(int client, int args) {
    int n = 500;
    if (args > 0) {
        char a[8];
        GetCmdArg(1, a, sizeof(a));
        n = StringToInt(a);
        if (n < 1) n = 1;
        if (n > 4000) n = 4000;
    }
    int made = 0;
    for (int i = 0; i < n; i++) {
        int ent = CreateEntityByName("info_target");
        if (ent == -1) break;              // het cho -> dung, dung ep them
        DispatchSpawn(ent);
        made++;
    }
    ReplyToCommand(client, "[ent_test] da sinh %d / %d info_target", made, n);
    if (made < n) ReplyToCommand(client, "  ! dung som o %d - engine tu choi cap them", made);
    Report(client, "sau khi ep tai");
    return Plugin_Handled;
}

public Action Cmd_Clear(int client, int args) {
    int n = 0, ent = -1;
    while ((ent = FindEntityByClassname(ent, "info_target")) != -1) {
        AcceptEntityInput(ent, "Kill");
        n++;
    }
    ReplyToCommand(client, "[ent_test] da xoa %d info_target", n);
    ReplyToCommand(client, "  (luu y: num_edicts KHONG giam - do la moc nuoc cao)");
    Report(client, "sau khi xoa");
    return Plugin_Handled;
}

// Do TICH TU: lan dau ghi moc, cac lan sau in DO CHENH so voi moc.
//   sm_ent_diff         - ghi moc (lan dau) / in chenh lech (lan sau)
//   sm_ent_diff reset   - xoa moc, ghi lai tu dau
public Action Cmd_Diff(int client, int args) {
    if (args > 0) {
        char a[16];
        GetCmdArg(1, a, sizeof(a));
        if (StrEqual(a, "reset", false)) {
            g_HasBase = false; g_BaseUsed = 0;
            ReplyToCommand(client, "[ent_test] da xoa moc. Go sm_ent_diff de ghi moc moi.");
            return Plugin_Handled;
        }
    }

    char names[MAX_CLASSES][64];
    int  counts[MAX_CLASSES];
    int  used = 0, live = 0;

    int cap = GetMaxEntities();
    for (int i = 0; i < cap; i++) {
        if (!IsValidEntity(i)) continue;
        live++;
        char cls[64];
        if (!GetEntityClassname(i, cls, sizeof(cls)) || cls[0] == '\0') continue;
        int j = 0;
        for (; j < used; j++) if (StrEqual(names[j], cls)) { counts[j]++; break; }
        if (j == used && used < MAX_CLASSES) {
            strcopy(names[used], 64, cls); counts[used] = 1; used++;
        }
    }

    if (!g_HasBase) {
        for (int i = 0; i < used; i++) { strcopy(g_BaseName[i], 64, names[i]); g_BaseCount[i] = counts[i]; }
        g_BaseUsed = used; g_BaseLive = live; g_HasBase = true;
        ReplyToCommand(client, "[ent_test] DA GHI MOC: %d thuc the song, %d lop.", live, used);
        ReplyToCommand(client, "[ent_test] Choi tiep vai phut roi go sm_ent_diff lan nua.");
        PrintToServer("[ent_test] DIFF moc: song=%d lop=%d", live, used);
        return Plugin_Handled;
    }

    // chenh lech theo tung lop
    char dName[MAX_CLASSES][64];
    int  dVal[MAX_CLASSES];
    int  dUsed = 0;
    for (int i = 0; i < used; i++) {
        int old = 0;
        for (int j = 0; j < g_BaseUsed; j++) if (StrEqual(g_BaseName[j], names[i])) { old = g_BaseCount[j]; break; }
        int d = counts[i] - old;
        if (d != 0 && dUsed < MAX_CLASSES) { strcopy(dName[dUsed], 64, names[i]); dVal[dUsed] = d; dUsed++; }
    }
    // lop bien mat hoan toan
    for (int j = 0; j < g_BaseUsed && dUsed < MAX_CLASSES; j++) {
        bool still = false;
        for (int i = 0; i < used; i++) if (StrEqual(names[i], g_BaseName[j])) { still = true; break; }
        if (!still) { strcopy(dName[dUsed], 64, g_BaseName[j]); dVal[dUsed] = -g_BaseCount[j]; dUsed++; }
    }

    ReplyToCommand(client, "=== TICH TU: song %d -> %d (chenh %+d) ===", g_BaseLive, live, live - g_BaseLive);
    PrintToServer("[ent_test] === TICH TU: song %d -> %d (chenh %+d) ===", g_BaseLive, live, live - g_BaseLive);

    for (int a = 0; a < dUsed && a < 30; a++) {
        int best = a;
        for (int b = a + 1; b < dUsed; b++) if (dVal[b] > dVal[best]) best = b;
        if (best != a) {
            int c = dVal[a]; dVal[a] = dVal[best]; dVal[best] = c;
            char t[64]; strcopy(t, sizeof(t), dName[a]);
            strcopy(dName[a], 64, dName[best]); strcopy(dName[best], 64, t);
        }
        if (dVal[a] == 0) break;
        ReplyToCommand(client, "  %+5d  %s", dVal[a], dName[a]);
        PrintToServer("[ent_test]   %+5d  %s", dVal[a], dName[a]);
    }
    ReplyToCommand(client, "(sm_ent_diff reset de ghi moc moi)");
    return Plugin_Handled;
}

public Action Cmd_Hud(int client, int args) {
    if (client == 0) {
        ReplyToCommand(client, "[ent_test] HUD chi dung trong game. Console dung sm_ent_report.");
        return Plugin_Handled;
    }
    g_bShowHud[client] = !g_bShowHud[client];
    ReplyToCommand(client, "[ent_test] HUD = %s", g_bShowHud[client] ? "BAT" : "TAT");
    return Plugin_Handled;
}

public void OnClientDisconnect(int client) {
    if (client > 0 && client <= MaxClients) g_bShowHud[client] = false;
}

public Action Timer_UpdateHud(Handle timer) {
    bool need = false;
    for (int i = 1; i <= MaxClients; i++) {
        if (IsClientInGame(i) && g_bShowHud[i]) { need = true; break; }
    }
    if (!need) return Plugin_Continue;

    int live = CountLive();
    int num  = GetEntityCount();
    int cap  = GetMaxEntities();
    int free = num - live;
    if (free < 0) free = 0;

    for (int i = 1; i <= MaxClients; i++) {
        if (IsClientInGame(i) && g_bShowHud[i]) {
            PrintHintText(i, "song %d | num_edicts %d | trong %d\nbien do %d / %d",
                          live, num, free, cap - live, cap);
        }
    }
    return Plugin_Continue;
}
