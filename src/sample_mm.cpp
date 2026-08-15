// ===========================================================================
// edictbudget - giu server L4D2 khong chet vi "ED_Alloc: no free edicts"
// ===========================================================================
//
// ===========================================================================
//  TAC GIA - DOC TRUOC
//
//  Toan bo ma nguon nay do AI viet: Claude (Anthropic), chay trong Claude Code.
//  Khong phai mot phan, khong phai "co AI ho tro" - la TOAN BO: thiet ke, doc
//  nguoc server.dll/engine.dll, viet ma, do dac, va cac ghi chu ban dang doc.
//
//  Nguoi dung la nguoi VAN HANH may chu that: ho dat bai toan, chay thu, chup
//  log, phat hien loi, va bac bo nhieu ket luan sai cua AI. Nhieu doan trong file
//  nay ghi ro "SAI, da sua" chinh la vi the.
//
//  Noi ro dieu nay vi hai ly do:
//    1. Ai doc ma nen biet no den tu dau de tu quyet dinh muc do tin tuong.
//    2. Nhieu ket luan o day rut ra tu doc nguoc nhi phan, khong phai tu tai lieu
//       chinh thuc. Chung deu kem dia chi ham va doan lenh de kiem lai duoc.
//       Cai gi khong xac minh duoc thi ghi thang la KHONG XAC DINH.
//
//  Giay phep: GPLv3. Xem file LICENSE.
// ===========================================================================
//
// NHIEM VU: giu so entity DANG SONG duoi tran 2048.
// KHONG nang tran - chi so entity trong giao thuc Source rong 11 bit (toi da
// 2047), nen entity CO MANG nam o chi so >=2048 se bi client giai ma sai.
//
// !!  GIOI HAN - PHAI DOC:
//   Ban va nay KHONG NGAN DUOC HOAN TOAN "ED_Alloc: no free edicts".
//   No chi lam hai viec: thu hoi edict DUNG LUC, va go edict khoi nhung lop
//   THUC SU KHONG DUNG MANG. Neu ban than map can nhieu hon 2048 entity CO
//   MANG cung luc thi khong co cach nao cuu - do la tran cua giao thuc, khong
//   phai cua ban va.
//   Vi du do duoc: mot map cong dung 312 point_spotlight + 312 spotlight_end
//   + 312 beam = 936 edict (45,7%) cho rieng hieu ung anh sang. Ca ba lop deu
//   phai co mang. Ban va khong dong toi duoc.
//
// ---------------------------------------------------------------------------
// BA CO CHE
//
// 1. wipeclear - don khi doi survivor thua
//    Luc wipe, game CO don entity nhung don MUON. Trinh tu that:
//      CDirector::Restart -> RestartRound(slot 178)
//                              |-- hoi sinh player      <== an het edict O DAY
//                              |-- CleanUpMap(slot 179) <== moi don, DA MUON
//    Moc vao DAU RestartRound, lam nua "don" cua CleanUpMap truoc doan ngon kia:
//      UTIL_Remove(ngoai preserve list) -> CleanupDeleteList()
//      -> AllowImmediateEdictReuse()
//    roi de game chay tiep; CleanUpMap tu dung lai map tu entity lump.
//    Chi don khi co mission_lost dang cho (co mot-lan). Khong co cong thi no
//    don ngay luc t=1.00 khi map vua nap va PHA MAP.
//    DO DUOC: 5 wipe lien tiep (map thuong), 3 wipe lien tiep (c6m1_riverbank).
//
// 2. freegate - cho phep tai dung slot vua giai phong
//    ED_Alloc TU CHOI tai dung mot edict trong 1 GIAY sau khi no duoc giai
//    phong. Wipe xoa roi tao lai hang tram entity trong CUNG mot khoanh khac,
//    nen khong cai nao qua noi cong do => chet trong khi con ~999 slot trong.
//    Doi mot byte trong engine.dll: jae -> jmp. Dinh vi bang QUET CHU KY.
//    An toan nho sv_useexplicitdelete (mac dinh bat) - Valve thiet ke no THAY
//    CHO thoi gian cho nay.
//    DO DUOC (doi chung): cung tinh huong num_edicts=2048 + ~999 slot trong,
//    freegate=0 -> CHET, freegate=1 -> chay tiep binh thuong.
//
// 3. noedict - khien lop khong dung mang KHONG LAY edict
//    CBaseEntity::PostConstructor xet bit 9 cua m_iEFlags (EFL_SERVER_ONLY):
//      = 0 -> AddNetworkableEntity    -> dai 0-2047, TON edict
//      = 1 -> AddNonNetworkableEntity -> dai 2049-4095, KHONG ton edict
//    Dai 2049-4095 (2047 o) la THIET KE GOC cua engine. Tran no chi in canh
//    bao roi tra handle khong hop le - KHONG giet server.
//    Thay vtable slot 29 (+0x74) cua rieng CLight/CDecal, bat bit roi goi ham
//    goc. Khong va byte, khong dung engine.dll.
//    DO DUOC: mot map tu CHET o 2048 edict -> nap duoc voi num_edicts=1178.
//
// ---------------------------------------------------------------------------
// KET QUA DO DUOC TREN BA CHIEN DICH CANG NHAT
//
// Tran engine: max_edicts = 2048. Cot "EDICT du kien" doc tu lump 0 cua BSP bang
// tools\bsp_cost.py; cot "do that" la num_edicts luc chay tren may chu.
// Cong thuc:  EDICT = (entity trong lump) - (lop trong noedict.txt)
//                     + 2 x point_spotlight co spawnflags&1
//
// 1. chernobyl  (5 map) - chua ch04_pripyat03, map khoi nguon cua ca du an
//      map              lump   noedict go   EDICT du kien   tong lump
//      ch01_jupiter     1532       316         1216             1532
//      ch02_pripyat01   2204      1138         1067             2205
//      ch03_pripyat02   1686       869          816             1685
//      ch04_pripyat03   2246      1039         1212             2251
//      ch05_pripyat04    940       301          648              949
//
//    PHEP KIEM NGUOC:
//      ch04_pripyat03 truoc khi co noedict: CHET o 2048 luc nap.
//      Cong thuc du doan (co noedict): 1212. Do that tren may chu: 1178.
//      Sai lech +34, tuc 2,9%. Cong thuc viet ra SAU, khop voi su co xay ra TRUOC.
//
//    !! GIOI HAN PHAI NHO - DUNG BIEN CON SO NAY THANH PHAN QUYET:
//      Cot "tong lump" KHONG PHAI so entity cung song mot luc. No chi la so dong
//      trong lump. Thuc te entity duoc KICH HOAT DAN:
//        - weapon_*_spawn tu UTIL_Remove chinh no ngay sau khi sinh vu khi
//        - StartDisabled chua kich hoat
//        - point_template sinh muon
//        - Director sinh dan theo tien trinh choi
//      ch02_pripyat01 co "tong lump" 2205 nhung KHONG HE CHET, ke ca truoc khi co
//      noedict. ch04_pripyat03 o 2251 thi chet. Hai con so chi cach nhau 46 =>
//      khong co nguong sach nao o day.
//      Sai so do duoc, LAN NAO CUNG THUA:
//        the_hive m3  du doan 1688 -> do that 1592  (-96)
//        the_hive m4  du doan 2067 -> do that 1955  (-112)
//        pripyat03    du doan 1212 -> do that 1178  (-34)
//      => Dung cong thuc lam CAN TREN va BANG XEP HANG. Muon biet map co chet
//         khong thi phai DO: loadprobe (8 frame dau) va heartbeat (moi 5 phut).
//
// 2. the_hive  (5 map)
//      map   EDICT du kien   ghi chu
//      m1         966
//      m2        1834        639 env_sprite - KHONG go duoc, xem duoi
//      m3        1688        80 point_spotlight (he so 3)
//      m4        2067        VUOT TRAN. 312 point_spotlight = 936 edict
//      m5        1343
//      Do that tren may chu, m4: dinh num_edicts=1955, trong=0, cho tho 93 slot.
//      Sau khi bat swap: song 1954 -> 1330. Cho tho 93 -> 718 slot.
//      m3 sau khi bat swap: song 1591 -> 1431.
//
// 3. anemoia / backroom  (6 map)
//      map           lump   noedict go   EDICT du kien
//      arcade        1246       433          812
//      kitty         2954      1444         1509   <- noedict CUU map nay
//      party         2198       399         1798      964 prop_dynamic
//      poolrooms      921       306          640
//      poolrooms2     914       313          626
//      reality       1351       488          862
//      kitty la bang chung manh nhat cho noedict: khong co no thi map ~2953 edict,
//      vuot tran 900 slot, chet chac luc nap.
//
// TONG KET GIAM DUOC BAO NHIEU (so entity duoc dat EFL_SERVER_ONLY / doi lop):
//      noedict   anemoia kitty   1444 entity  |  chernobyl ch02  1138
//                chernobyl ch04  1039         |  chernobyl ch03   869
//                anemoia reality  488         |  the_hive m4      465
//                the_hive m3      443
//      Chi MOT truong hop da CHUNG MINH duoc la "khong co no thi chet":
//        ch04_pripyat03 - chet that o 2048 truoc khi co noedict, sau do nap duoc
//        voi num_edicts=1178. Cac map khac chi la con so lump lon, CHUA CHUNG MINH.
//      swap      the_hive m4   624 edict (312 x 2)
//                the_hive m3   160 edict (80 x 2)
//                the_hive m5     8 edict (chi 4/12 cai co spawnflags&1)
//                anemoia      ~26 edict/map (chi poolrooms co 13 cai) - khong dang
//
// RUI RO CUA `swap` DA DUOC DINH LUONG (15/08) - NHO HON NHIEU SO VOI LO NGAI BAN DAU:
//   `beam_spotlight` giu FCAP_ACROSS_TRANSITION con `point_spotlight` thi bo, nen
//   ban dau tuong so entity mang sang tang manh. Do lai bang PVS that:
//     m4 -> m5 :  +0   (312 beam_spotlight cua m4 KHONG cai nao trong PVS landmark)
//     m3 -> m4 : +48   (48 point_spotlight nam trong PVS cua landmark_m4 tren m3)
//   Danh sach chuyen man that: m3->m4 = 22 entity, m4->m5 = 32. Tran engine 512.
//   Doi 48 edict lay 784 => KHONG SUA.
//
//   Vi sao truoc do uoc nham 739/1051: server.dll co 54 ham ObjectCaps khac nhau,
//   31 trong so do cung `and eax,0xFFFFFFFD` (bo co) nhung khuon byte khac
//   CPointEntity nen bi bo sot. Rieng the_hive: CSprite@1009A5D0 (env_sprite 236),
//   CBeam@10081580 (beam 312), CSpotlightEnd@101DEEB0 (spotlight_end 312) deu bo co.
//
//   CChangeLevel::BuildChangeList @101FF060 KHONG duyet gEntList. No duyet
//   UTIL_EntitiesInPVS(landmark) @10209BC0 - chi entity trong PVS cua info_landmark
//   (1-4% ban do) - VA co dong `cmp dword [esi+0x28],0 ; je` bo qua entity KHONG CO
//   EDICT. => noedict MIEN NHIEM HOAN TOAN voi chuyen man.
//   Vuot 512 goi tier0!Warning (KHONG phai Error), giu 512 muc dau, bo phan du.
//
// CHO PLUGIN CHUA XU LY DUOC:
//      the_hive m2  = 1834, thu pham 639 env_sprite.
//      anemoia party = 1798, thu pham 964 prop_dynamic.
//      Ca hai lop deu CO SendTable rieng (CSprite, CDynamicProp) nen KHONG go mang
//      duoc, va deu he so 1 nen swap vo dung. Can co che khac - dang nghien cuu
//      huong sua entity lump ngay trong Hook_LevelInit (xem RewriteLump).
//      KHI NAO CO CONG THUC CHUNG CHO anemoia THI GHI THEM VAO DAY.
//
// ---------------------------------------------------------------------------
// FILE CAU HINH  (left4dead2\addons\edictbudget\)
//   stage.txt      0 = nam im hoan toan  |  1 = hoat dong
//   patches.txt    cong tac tung phan, doi xong chi khoi dong lai server
//                  wipeclear = 0 tat / 1 chi quan sat / 2 don that
//   noedict.txt    lop bat EFL_SERVER_ONLY. Truoc khi them lop moi phai qua
//                  du 6 dieu kien - ghi trong chinh file do.
//   wipekeep.txt   lop GIU THEM khi wipeclear don. DE TRONG moi dung: o wipe,
//                  entity bi xoa se DUOC DUNG LAI tu entity lump, nen giu them
//                  chi lam hep bien do.
//   mapkeep.txt    lop KHONG DUOC DON khi chuyen man (chi dung khi mapclear>=2).
//                  Nguoc voi wipekeep: o chuyen man, xoa nham la MAT VINH VIEN.
//
// BUILD
//   SOURCE_ENGINE PHAI = 15 (LEFT4DEAD2) theo cach danh so cua Metamod.
//   Build nham 11 (TF2) lam lech moi chi so vtable, SH_CALL goi nham ham engine.
//
// ---------------------------------------------------------------------------
// HUONG 4096: NANG GIOI HAN LA LAM DUOC. GIOI HAN 11 BIT MOI LA KHONG THE.
//
// Phai noi ro hai chuyen khac nhau, dung gop lam mot:
//
//   (a) NANG SO EDICT len 4096 hoac cao hon  ->  LAM DUOC, ma o duoi day.
//   (b) Dat entity CO MANG o chi so >= 2048  ->  KHONG THE, va khong bao gio
//       lam duoc bang cach va server.dll/engine.dll.
//
// Ly do (b) khong the: chi so entity duoc ma hoa trong goi tin bang truong
// 11 bit (toi da 2047). Do la dinh dang GOI TIN, nam o ca hai dau day - client
// va server. Va server khong lam client hieu duoc chi so 2048; client se giai
// ma ra mot chi so khac han. Muon sua thi phai sua ca client.dll cua tung
// nguoi choi, tuc khong kha thi.
//
// => Cho trong dai 2048-4095 CHI dung duoc cho entity KHONG CO MANG.
//    Va engine DA CO san co che cho viec do: EFL_SERVER_ONLY + nua tren cua
//    m_EntPtrArray (xem noedict o duoi). Khong can va byte nao.
//
// ---------------------------------------------------------------------------
// CAC BYTE NANG GIOI HAN - GHI LAI DE DOI CHIEU, MAC DINH TAT HET
//
//   bigarray   SV_AllocateEdicts cap 4096 edict thay vi 2048.
//              Chu ky trong engine.dll:
//                  B8 00 08 00 00   mov eax, 0x800      <- 2048
//                  89 86 18 02 00 00
//                  A3 ?? ?? ?? ??
//              Ghi de 4 byte tai m+1 bang so o muon cap (EXT_LIMIT = 4096).
//              Dat 8192 cung chay - mang cap phat theo so nay.
//
//   snapshot   Doi 7 cho truy cap hai bang m_pPackedData / m_pSerialNumber
//              sang bo dem 4096 o. BAT BUOC di kem bigarray: mang 4096 edict
//              voi bang snapshot 2048 o thi TE HON la khong lam gi - phan
//              edict thua se ghi de len bo nho ben canh.
//              7 chu ky dang  8B 84 B1 9C ...  ->  8B 04 B5 <dia chi moi>
//
//   pinmax     LevelInit ghim sv.max_edicts ve 2048.
//   pinglobals LevelInit ghim gpGlobals->maxEntities ve 2048.
//              Hai cai nay giu TRAN CUA ENGINE o 2048 de bo cap phat cua
//              engine khong tu dat entity len dai cao. Thieu chung thi
//              num_edicts leo qua 2047 va entity CO MANG tran len dai cao
//              - dung dieu (b) noi tren.
//
//   markfree   LevelInit dong dau FL_EDICT_FREE len cac o 2048-4095.
//
// ---------------------------------------------------------------------------
// VI SAO VAN TAT HET
//
//   1. Nhom nay LAM HONG VONG HOI SINH LUC WIPE - tuc pha luon wipeclear,
//      thu duy nhat dang giai quyet duoc dot bung lon nhat.
//   2. No khong giai duoc bai toan goc. Cho trong o dai cao chi chua duoc
//      entity khong co mang, ma viec do noedict lam duoc bang duong CHINH
//      THUC cua engine, khong can va byte.
//   3. Do thuc te: bat bigarray+snapshot ma thieu pinmax/pinglobals thi
//      num_edicts = 2060, entity NGAU NHIEN tran len tren 2047 - mat on dinh.
//
//   Ma van con de doi chieu va de ai muon do lai thi co san. Khong duoc bat
//   trong ban chay.
// ===========================================================================

#include "sample_mm.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdint.h>
#include <windows.h>
#include <psapi.h>
#include <igameevents.h>

// windows.h dinh nghia GetClassName -> GetClassNameA (ham cua Win32 GUI), lam
// hong loi goi IServerNetworkable::GetClassName(). Go macro di.
#ifdef GetClassName
#undef GetClassName
#endif

SamplePlugin g_SamplePlugin;
ISmmPlugin *OOSM_api = &g_SamplePlugin;
IServerGameDLL *server = NULL;
IServerGameClients *gameclients = NULL;
IVEngineServer *engine = NULL;
IGameEventManager2 *gameevents = NULL;
CGlobalVars *gpGlobals = NULL;

SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, char const *, char const *, char const *, char const *, bool, bool);
SH_DECL_HOOK3_void(IServerGameDLL, ServerActivate, SH_NOATTRIB, 0, edict_t *, int, int);
SH_DECL_HOOK1(IVEngineServer, CreateEdict, SH_NOATTRIB, 0, edict_t *, int);
SH_DECL_HOOK1_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool);

PLUGIN_EXPOSE(SamplePlugin, g_SamplePlugin);

// --------------------------------------------------------------------------
// Constants
// --------------------------------------------------------------------------
#define NET_LIMIT   2048    // 11-bit protocol ceiling; slots 0..2047 only
#define EXT_LIMIT   4096    // total array size we make the engine allocate
#define EDICT_SIZE  16      // sizeof(edict_t) on this 32-bit build
#define FL_FREE     2       // FL_EDICT_FREE = (1<<1), public/edict.h

// --------------------------------------------------------------------------
// Engine globals, resolved by signature at load time.
// sv layout: num_edicts(+0x214) max_edicts(+0x218) edicts(+0x21C) states(+0x220)
// --------------------------------------------------------------------------
static uint32_t* g_num_edicts = NULL;
static uint32_t* g_max_edicts = NULL;
static uint32_t* g_edicts     = NULL;
static uint32_t* g_edict_states = NULL;

// HAI CAU HOI KHAC NHAU, va gop chung lam mot da tung la loi that:
//   g_BigArrayOn      - SV_AllocateEdicts da duoc noi rong len 4096 chua?
//   g_EngineArray4096 - co AN TOAN de DAT thu gi o tren 2047 khong?
//
// Cau thu hai con doi bang snapshot phai duoc doi cho nua. Cau thu nhat, tu no,
// bat buoc ta phai ghim sv.max_edicts ve 2048 moi lan nap map: mang 4096 ma de
// max_edicts o 4096 thi ED_Alloc se noi them entity CO MANG binh thuong vao chi
// so 2048+, ma khong client nao dia chi hoa duoc (giao thuc chi mang 11 bit),
// nen chung ton tai va phan hoi nhung khong bao gio co model.
// Dieu kien ghim ma dat theo co GOP thi mang noi rong xuat xuong ma thieu day an toan.
static bool g_BigArrayOn      = false;
static bool g_EngineArray4096 = false;

// Chot mot lan cho bo canh moi frame (xem Hook_GameFrame).
static bool g_WarnedNum = false;
static bool g_WarnedMax = false;
static bool g_WarnedGlob = false;
static bool g_ExtReady        = false;  // dai mo rong da dung duoc chua?
static int  g_Cursor          = EXT_LIMIT - 1;  // cap phat DI XUONG, xem Hook_CreateEdict
static int  g_Stage           = 1;

uint8_t* FindPattern(const char* module, const char* pattern, const char* mask);

// Dinh nghia o duoi, canh bo canh danh sach SourceMod ma chung thuoc ve.
static uint32_t* g_SMListHead;
static void ResolveSMListHead();
edict_t* Hook_CreateEdict_Post(int forceIndex);   // dinh nghia o cuoi file

// ==========================================================================
// Ham phu nho
// ==========================================================================
static void WriteProtected(void* dst, const void* src, size_t len) {
    DWORD old;
    VirtualProtect(dst, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy(dst, src, len);
    VirtualProtect(dst, len, old, &old);
}

// Thu ca hai thu muc lam viec: srcds.exe chay tu THU MUC GOC cua may chu, con
// listen server hoac trinh khoi chay khac co the dang o trong left4dead2\.
static FILE* OpenPluginFile(const char* name, const char* mode) {
    char path[MAX_PATH];
    _snprintf(path, sizeof(path), "left4dead2\\addons\\edictbudget\\%s", name);
    path[sizeof(path)-1] = 0;
    FILE* f = fopen(path, mode);
    if (f) return f;
    _snprintf(path, sizeof(path), "addons\\edictbudget\\%s", name);
    path[sizeof(path)-1] = 0;
    return fopen(path, mode);
}

// ==========================================================================
// GHI LOG RA FILE RIENG
// ==========================================================================
//
// META_LOG chi day ra console cua server. Neu may chu khong bat ghi console.log
// thi moi so lieu do duoc deu mat. Nen toan bo log cua plugin ghi thang vao
// file rieng:
//
//     left4dead2ddons\edictbudget\edictbudget.log
//
// Moi dong co dau thoi gian. File mo o che do noi tiep, khong ghi de.
// fflush sau moi dong de neu server chet dot ngot van con du log den phut cuoi
// - dung luc can nhat.
//
// CONG TAC logconsole = 1 thi in ra CA console (mac dinh 0).
// --------------------------------------------------------------------------
static FILE* g_LogFile   = NULL;
static bool  g_LogOpened = false;
// logconsole: 1 = in CA ra console server. Mac dinh 0 - chi ghi file.
static bool  g_LogConsole = false;

static void EL_LOG(const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);
    buf[sizeof(buf) - 1] = 0;

    if (!g_LogOpened) {
        g_LogOpened = true;
        g_LogFile = OpenPluginFile("edictbudget.log", "a");
        if (g_LogFile) {
            time_t t = time(NULL);
            struct tm* lt = localtime(&t);
            fprintf(g_LogFile,
                    "\n===== phien moi: %04d-%02d-%02d %02d:%02d:%02d =====\n",
                    lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
                    lt->tm_hour, lt->tm_min, lt->tm_sec);
        }
    }
    if (g_LogFile) {
        time_t t = time(NULL);
        struct tm* lt = localtime(&t);
        fprintf(g_LogFile, "%02d:%02d:%02d  %s\n",
                lt->tm_hour, lt->tm_min, lt->tm_sec, buf);
        fflush(g_LogFile);
    }
    if (g_LogConsole && g_PLAPI) META_LOG(g_PLAPI, "%s", buf);
}

static void EL_LOG_CLOSE() {
    if (g_LogFile) { fclose(g_LogFile); g_LogFile = NULL; }
    g_LogOpened = false;
}

// ==========================================================================
// Cong tac cho tung ban va, doc tu patches.txt luc nap
// ==========================================================================
//
// Nam ban va byte doc lap di vao engine, va loi do BAT KY cai nao trong so do
// gay ra deu trong GIONG HET NHAU tu ben ngoai. Build lai de chia doi tim thu
// pham thi moi lan mat tron mot vong tat/chep/khoi dong lai, nen thay vao do
// moi ban va co mot cong tac rieng: ghi "ten=0" vao patches.txt roi khoi dong lai.
//
// Thieu file hoac thieu khoa deu coi la BAT, nen truong hop binh thuong khong
// can file nao ca.

static bool g_PatchFreetime    = true;
static bool g_PatchIndexBounds = true;
static bool g_PatchForcedIndex = true;
static bool g_PatchBigArray    = true;
static bool g_PatchSnapshot    = true;

// JMP noi tuyen 7 byte de nhay qua CreateEntityByName cua server.dll, cong mot
// trampoline tu viet. No CHI ton tai de biet classname phuc vu danh sach cho
// phep va ban kiem ke - khi tat phan phan tach thi no la chi phi thuan tuy.
// No KHONG co cong tac, nen da am tham hoat dong trong MOI lan chia doi tim loi
// va chua bao gio bi loai tru mot lan nao. Plugin goc 233 dong khong he detour
// kieu nay ma khoi dong rat vung - chinh dieu do dua no vao dien tinh nghi.
static bool g_PatchDetour      = true;

// Yeu cau engine BO thoi gian cho 1 giay truoc khi mot edict vua giai phong
// duoc cap phat lai.
//
// ED_Alloc tu choi tai su dung mot edict cho toi 1 giay sau khi no duoc giai
// phong. Ma mot lan wipe giai phong hang tram entity roi tao lai chung trong
// CUNG MOT FRAME, nen khong cai nao du dieu kien, va engine buoc phai noi them
// edict moi - do chinh la thu lam can kiet mot map dang o muc 2012/2047.
//
// IVEngineServer::AllowImmediateEdictReuse() la cau tra loi cua chinh Valve cho
// viec nay ("Tells the engine we can immdiately re-use all edict indices even
// though we may not have waited enough time", eiface.h:345). Convar di kem
// sv_useexplicitdelete - mac dinh BAT - lam engine bao cho client biet entity cu
// da bien mat TRUOC khi chi so cua no duoc tai dung, va do dung la thu ma thoi
// gian cho kia dang bao ve.
//
// Huong nay danh dung vao co che hong that su. Phan tach chi bao gio cung chi
// them bien do; con cai nay xoa bo NHU CAU phai co bien do.
static bool g_ImmediateReuse   = true;

// Cho phep tai su dung edict ngay, va thang vao ED_Alloc.
//
// AllowImmediateEdictReuse() phai duoc goi TU BEN NGOAI, nen no chi cham toi
// nhung lan cap phat di qua IVEngineServer::CreateEdict. Da chung minh (mucA
// 0-AAC) rang lan cap phat that bai KHONG di qua duong do - hook chua bao gio
// duoc goi cho no. Va byte tai chinh diem quyet dinh thi khong co lo hong day.
static bool g_PatchFreeGate    = true;
static int  g_MinFreeSeen      = 999999;
static int  g_EdgeLines        = 0;

// Dem so lan cap phat edict trong MOT frame.
//
// Mau cuoi cung truoc khi chet luon la num_edicts=2012 voi ~904 slot trong -
// trang thai ma ED_Alloc KHONG THE bao loi (2012 < 2048 nen no cap moi).
// De toi duoc nhanh loi thi trong khoang giua hai lan lay mau (<0.25s) phai
// co ~940 lan cap phat: 36 lan day num_edicts len 2048, cong 904 lan chiem
// het slot trong. Neu dung, wipe la mot dot bung no ~940 entity CUNG LUC va
// map that su vuot 2048 o dinh - luc do moi huong "tai su dung slot" deu vo
// nghia vi khong con gi de tai su dung.
static int  g_AllocThisFrame   = 0;
static int  g_MaxBurst         = 0;

// Thay doi goi la "mang lon" thuc ra la BA viec rieng biet tinh co di chung
// voi nhau. Khi hoa ra chinh nhom nay lam hong vong hoi sinh entity luc wipe,
// chung buoc phai tach ra de thu duoc rieng tung cai:
//   bigarray  - SV_AllocateEdicts cap 4096 edict thay vi 2048
//   pinlimits - LevelInit dat max_edicts / maxEntities ve lai 2048
//   markfree  - LevelInit dong co FL_EDICT_FREE len cac o 2048-4095
static bool g_PinMax     = true;   // sv.max_edicts        -> 2048
static bool g_PinGlobals = true;   // gpGlobals->maxEntities -> 2048
static bool g_MarkFree   = true;

// WIPECLEAR: don thuc the o dau CTerrorGameRules::RestartRound (vtable slot 178),
// truoc vong hoi sinh player. Xem khoi giai thich day du gan InstallWipeClear().
//
// BA TRANG THAI, khong phai hai - de moi buoc thu chi doi MOT thu:
//   0 = TAT HOAN TOAN. Khong moc vtable, khong nghe su kien. No-op that su,
//       dung lam moc doi chieu.
//   1 = CHI QUAN SAT. Moc vtable + nghe su kien, log day du moc thoi gian va
//       so slot, nhung KHONG xoa mot entity nao. Rui ro gan bang khong, va no
//       tra loi cau con treo: tin hieu thua ban TRUOC hay SAU RestartRound,
//       va num_edicts cham 2048 o doan nao.
//   2 = DON THAT. Lam nua "don" cua CleanUpMap ngay dau RestartRound.
//
// Mac dinh 0. Doi trong patches.txt, khong can build lai.
static int g_WipeClear = 0;

// Bay ED_Alloc: ghi 0xE9 (JMP) de len 8 byte nhanh loi trong engine.dll roi dung
// lai hai nhanh goc trong stub. Truoc 07/08 no KHONG co cong tac - tu chay moi
// khi stage!=0. Nay tach ra vi:
//   - muc 8c ghi "khong crash khi stage=0", ma nay stage=1 chi con bay + wipeclear
//     => bay la nghi can duy nhat con lai cho con crash sourcemod+0x13b63
//   - va vi no LA mot ban va byte vao engine.dll, dung de no chay ngam khong cong tac
// Khi wipeclear da chay dung thi bay chi con de chan doan - tat duoc.
static bool g_PatchTrap = true;

// ---------------------------------------------------------------------------
// HAI CONG TAC XU LY THUC THE KHONG DUNG MANG
//
// Van de: entity khong can gui cho client VAN chiem edict trong dai 0-2047 -
// dai ma giao thuc 11 bit danh cho thu phai gui. Do la lang phi thuan.
// Khong go duoc edict cua entity dang song (DetachEdict() la private, chi
// destructor goi duoc). Nen chi con hai duong:
//
//   nonetkill = 1   XOA HAN chung sau khi map nap xong.
//                   Duyet gEntList, UTIL_Remove lop khop serveronly.txt,
//                   roi CleanupDeleteList() de tra edict ngay.
//                   Duoc: tra slot ve dai 0-2047 vinh vien.
//                   Mat: mat luon chuc nang cua entity do.
//
//   nonethigh = 1   DAY len dai 2048-4095 thay vi xoa.
//                   Dung lai duong Hook_CreateEdict san co (cap phat xuong tu
//                   4095) - xem khoi chu thich tai ham do.
//                   !!  CAN bigarray=1 VA snapshot=1, neu khong g_ExtReady=false
//                   va no khong lam gi ca.
//                   !!  Muc 0-AAA: huong nay TUNG gay crash trong phep A/B sach
//                   nhat cua du an. Bat lai la co y chap nhan rui ro do de do lai.
//
// Ca hai deu doc danh sach lop tu serveronly.txt (quy tac khop: dong ket thuc
// '_' = khop tien to, con lai = khop chinh xac).
//
// !!  KHONG bat ca hai cung luc - chung mau thuan. Neu bat ca hai, nonetkill
// thang va nonethigh bi bo qua (co canh bao trong log).
// ---------------------------------------------------------------------------
static bool g_NoNetKill = false;
static bool g_NoNetHigh = false;

// noedict: bat EFL_SERVER_ONLY cho cac lop trong noedict.txt => chung KHONG
// duoc cap edict, nam o dai 2049-4095 (thiet ke GOC cua engine).
// XXX KHONG lien quan 4096. Xem khoi giai thich day du gan InstallNoEdict().
static bool g_NoEdict   = false;

// mapclear: don entity luc CHUYEN MAN, giu lai do nguoi choi mang sang duoc.
// 0 tat | 1 chi quan sat | 2 don that. Xem khoi giai thich gan InstallMapClear().
static int  g_MapClear = 0;
// Tran so entity mapclear duoc xoa moi lan chuyen man. 0 = khong gioi han.
// Dung de TIM NGUONG: hai lan xoa >1300 deu chet cam.
static int  g_MapClearMax = 0;
// 1 = CHI xoa entity co FCAP_ACROSS_TRANSITION. 0 = xoa tat ca (mac dinh, hanh vi cu).
//
// !!! DE MAC DINH 0. CHE DO 1 DA DUOC THU 09:44 14/08 VA GIET SERVER NGAY.
//
//   MAPCLEAR #1 (che do 2, chi-mang-sang=1): tong 1551 | mang sang 295
//   | go 200 (mang sang 200), giu 156, bo qua vi khong mang sang 1153, cham tran 42
//   -> "Server is hibernating" + khoi dong lai. Khong ED_Alloc, khong assert.
//
// Luat dung, giai thich duoc CA BA lan chet (thay cho luat "nguong 1300" da sai):
//     cap 100  -> trong do    9 cai mang sang -> SONG
//     >1300    -> trong do ~270 cai mang sang -> CHET
//     carry=1  -> trong do  200 cai mang sang -> CHET
//   KHONG PHAI SO LUONG GIET. LA XOA CAI MANG SANG GIET.
//   "Nguong 1300" chi la trung hop: xoa cang nhieu thi cang vo phai nhieu cai mang sang.
//
// Co che: hook chay POST nen PrepareLevelChange goc DA lap xong danh sach chuyen man.
// Entity co FCAP_ACROSS_TRANSITION nam san trong danh sach do. Xoa sau khi danh sach
// da lap => danh sach tro vao vung da giai phong => sap khi engine xu ly chuyen man.
// Hook PRE cung khong cuu duoc: engine van phai doc chinh nhung entity ay de lap danh sach.
//
// He qua: chi cai mang sang moi ton edict o map sau, ma cai mang sang thi khong duoc
// dung vao => MAPCLEAR VE NGUYEN TAC KHONG GIAI QUYET DUOC "m3 -> m4".
// Giu cong tac nay lai chi de ghi lai thi nghiem, KHONG phai de bat.
static int  g_MapClearCarryOnly = 0;
// Con tro ServerClass ma GetServerClass() tra ve khi lop KHONG co SendTable rieng
// (tuc la dung chung DT_BaseEntity). Xem cong an toan 3 trong InstallNoEdict().
//
// !!  DAY LA DIA CHI TRONG ANH TINH (ImageBase 0x10000000). Luc chay server.dll nap
//    o base khac, con tro doc tu vtable DA DUOC DOI THEO BASE.
//    Phai so voi  base + (0x107D78A8 - 0x10000000)  chu KHONG so thang.
//
//    Ban 16:01 ngay 14/08 so thang -> ca 4 lop dang chay (infodecal/light/light_spot/
//    path_track) deu bi TU CHOI, noedict tat hoan toan, "da sua 0 vtable / 4 lop".
//    Log cho thay ca bon deu tra 0x540378A8, base that la 0x53860000:
//        0x53860000 + 0x7D78A8 = 0x540378A8   <- dung, chi la chua doi hang so.
//    Cung loai loi voi vu chu ky mapclue bi tu choi vi prologue chua mat na.
#define DT_BASEENTITY_RVA 0x7D78A8u
// Ghi so edict trong N frame dau sau khi nap map, de bat DINH TAM THOI. 0 = tat.
//
// Vi sao can: `MOC CO SO` ghi tai ServerActivate, luc do nhieu entity CHUA spawn xong.
// Vi du point_spotlight tao spotlight_end + beam trong Activate()/Think(), tuc la SAU
// ServerActivate. Do la ly do m4 ghi num_edicts=1463 luc do trong khi dem tu lump ra
// 2067 - phan chenh xuat hien o may frame ke tiep.
// Ngoai ra ~35 lop weapon_*_spawn tao entity that roi UTIL_Remove chinh no; UTIL_Remove
// hoan den cuoi frame nen moi cai chiem 2 edict cung luc trong frame nap.
// Ca hai gia thuyet deu chi kiem duoc bang cach lay mau TUNG FRAME.
// swap: doi lop entity thanh lop re hon luc tao. Xem khoi giai thich o InstallSwap().
// 0 = tat | 1 = CHI QUAN SAT (dem, khong doi) | 2 = doi that
static int  g_Swap = 0;
// Tran so lan doi. 0 = khong gioi han. Dung de thu vai chuc cai truoc khi doi het.
static int  g_SwapMax = 0;
static int  g_LoadProbe = 8;
static int  g_LoadProbeLeft = 0;
static int  g_LoadProbeFrame = 0;
static int  g_LoadProbePeak = 0;
// heartbeat: so GIAY giua hai lan ghi so lieu thuc the vao log. 0 = tat.
// Chi ghi log, khong dong vao entity nao. Xem HeartbeatSample().
static int  g_Heartbeat = 0;

// ---------------------------------------------------------------------------
// nonetkill: DOI TEN classname TAI CHO trong entity lump, o LevelInit.
//
// CO CHE (da xac minh tren binary):
//   classname la  -> CEntityFactoryDictionary::Create (0x10206A40)
//                    -> DevWarning("Attempted to create unknown entity type %s!")
//                    -> tra NULL
//                 -> MapEntity_ParseEntity (0x101198F0): DevWarning("Can't init %s"),
//                    KHONG deref NULL
//                 -> MapEntity_ParseAllEntities (0x1011A600): bo qua NULL
//   => entity im lang khong duoc spawn, KHONG ton edict, KHONG ton han muc nao.
//   Khop tai lieu Valve: "Entities... not recognized by the server do not create
//   edicts... they are simply not spawned."
//
// XXX HAI DUONG GIET SERVER - PHAI TRANH:
//   0x1011A6C0  khoi khong mo bang '{'      -> tier0!Error  (import 0x105C1224)
//   0x10119943  khoi thieu key "classname"  -> tier0!Error
//   => TUYET DOI khong xoa khoi, khong doi do dai chuoi. CHI ghi de gia tri.
//
// CACH DOI: thay DUNG MOT ky tu dau thanh '~'.
//   infodecal -> ~nfodecal
//   Bao dam cung do dai, va khong classname nao cua L4D2 bat dau bang '~'
//   (557 classname da liet ke, khong cai nao).
//
// ###########################################################################
// XXX DANH SACH MAC DINH: RONG. DUNG THEM 'light*' HAY 'infodecal' VAO DAY.
// ###########################################################################
// Mac dinh cung tay { infodecal, light, light_spot } la SAI:
//   -> ch04_pripyat03 HIEN THI SAI ANH SANG.
//
// NGUYEN NHAN GOC - nonetkill khac nonethigh o mot diem sinh tu:
//   nonethigh : entity VAN DUOC TAO, VAN chay Spawn()/Activate(), chi la khong
//               cap edict. Moi TAC DUNG PHU van xay ra. -> anh sang DUNG.
//   nonetkill : entity KHONG BAO GIO TON TAI. Spawn()/Activate() khong chay.
//               -> MAT SACH tac dung phu.
//
// => nonetkill SAI VE BAN CHAT voi moi entity ma GIA TRI CUA NO NAM O TAC DUNG
//    PHU LUC SPAWN. Da xac minh tren binary (output/binscan/step_light.py):
//
//   CLight::Spawn 0x1010FA10  (dung chung cho light / light_spot /
//                              light_directional; light_environment = jmp toi day)
//     [esi+0x140] m_iszName == 0  -> UTIL_Remove(this)      // den "tro", tu xoa
//     [esi+0x140] m_iszName != 0  -> neu m_iStyle >= 32:
//                                      engine->LightStyle(m_iStyle, pattern)
//                                      (0x107F7698 = g_pEngineServer, vt +0xA0)
//     Den CO TEN = den BAT/TAT DUOC. VRAD nuong no thanh mot lightstyle rieng
//     luc compile; entity luc chay la thu DUY NHAT dat trang thai dau cho lop
//     lightmap do. Cat entity -> LightStyle() khong chay -> lop do giu mac dinh
//     -> SANG SAI. Den KHONG ten thi da tu xoa san, cat cung KHONG duoc gi.
//
//   CDecal::Spawn 0x102362A0 / CDecal::Activate 0x10236D10
//     Spawn:    m_nTexture < 0 hoac (deathmatch && lowprio) -> UTIL_Remove
//               con lai -> SONG. Server dedicated khong phai deathmatch => SONG.
//     Activate: khong targetname -> jmp StaticDecal() (dan decal roi TU XOA)
//     => infodecal CHUA TUNG giu edict lau dai. Cat no tiet kiem GAN NHU BANG 0,
//        doi lay TOAN BO decal cua map. Lo von nang.
//     (infodecal do VScript tao thi sinh luc chay, khong qua lump -> khong dinh.)
//
// !!  Muon giam edict cho ho light/infodecal thi dung NONETHIGH, khong phai day.
//
// Doc tu nonetkill.txt neu co (moi dong mot classname). Truoc khi them BAT CU
// lop nao, phai tra loi duoc: "Spawn()/Activate() cua no co lam gi khong?"
// Neu co -> KHONG duoc cat.
// XXX KHONG them lop nhom "song lau dai" (logic_auto, func_nav_attribute_region,
//    info_gamemode, info_survivor_position...).
// ---------------------------------------------------------------------------
#define KILL_MAX 64
static char g_KillList[KILL_MAX][40];
static int  g_KillCount = 0;
static char* g_LumpCopy = NULL;

static void LoadKillList() {
    g_KillCount = 0;
    // RONG co y. Xem khoi cam o tren: mac dinh cu {infodecal,light,light_spot}
    // lam sai anh sang ch04_pripyat03. Khong co nonetkill.txt = khong cat gi.
    static const char* kDefault[] = { NULL };

    FILE* f = OpenPluginFile("nonetkill.txt", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f) && g_KillCount < KILL_MAX) {
            size_t L = strlen(line);
            if (L > 0 && line[L-1] != '\n') { int c; while ((c=fgetc(f)) != EOF && c != '\n') {} }
            char* p = line;
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '#' || *p == '\r' || *p == '\n' || !*p) continue;
            char* e = p + strlen(p) - 1;
            while (e >= p && (*e=='\r'||*e=='\n'||*e==' '||*e=='\t')) *e-- = 0;
            if (!*p) continue;
            bool ok = true;
            for (const char* q = p; *q; q++)
                if (!((*q>='a'&&*q<='z')||(*q>='A'&&*q<='Z')||(*q>='0'&&*q<='9')||*q=='_')) { ok=false; break; }
            if (!ok) continue;
            strncpy(g_KillList[g_KillCount], p, sizeof(g_KillList[0])-1);
            g_KillList[g_KillCount][sizeof(g_KillList[0])-1] = 0;
            g_KillCount++;
        }
        fclose(f);
    } else {
        for (int i = 0; kDefault[i] && g_KillCount < KILL_MAX; i++) {
            strncpy(g_KillList[g_KillCount], kDefault[i], sizeof(g_KillList[0])-1);
            g_KillList[g_KillCount][sizeof(g_KillList[0])-1] = 0;
            g_KillCount++;
        }
    }
    EL_LOG("[EdictBudget] NONETKILL: %d lop trong danh sach%s",
             g_KillCount, OpenPluginFile("nonetkill.txt","r") ? "" : " (mac dinh)");
    for (int i = 0; i < g_KillCount; i++)
        EL_LOG("[EdictBudget] NONETKILL:   [%d] '%s'", i, g_KillList[i]);
}

// Tra ve chuoi lump DA SUA (cap phat moi), hoac NULL neu khong sua gi.
// Chi ghi de gia tri classname, giu nguyen do dai va moi ky tu khac.
static const char* RewriteLump(const char* lump, int* outHits) {
    *outHits = 0;
    if (!lump || g_KillCount <= 0) return NULL;

    size_t len = strlen(lump);
    char* buf = (char*)malloc(len + 1);
    if (!buf) return NULL;
    memcpy(buf, lump, len + 1);

    // Tim moi cap:  "classname" "<gia tri>"
    const char* KEY = "\"classname\"";
    size_t klen = strlen(KEY);
    for (size_t i = 0; i + klen < len; i++) {
        if (memcmp(buf + i, KEY, klen) != 0) continue;
        size_t j = i + klen;
        while (j < len && (buf[j]==' '||buf[j]=='\t')) j++;
        if (j >= len || buf[j] != '"') continue;
        size_t vs = ++j;                       // dau gia tri
        while (j < len && buf[j] != '"') j++;
        if (j >= len) break;
        size_t vlen = j - vs;
        if (vlen == 0 || vlen >= 40) continue;

        char cls[40];
        memcpy(cls, buf + vs, vlen); cls[vlen] = 0;
        for (int k = 0; k < g_KillCount; k++) {
            if (strcmp(cls, g_KillList[k]) == 0) {
                buf[vs] = '~';                 // DOI DUNG MOT KY TU, giu do dai
                (*outHits)++;
                break;
            }
        }
    }

    if (*outHits == 0) { free(buf); return NULL; }
    return buf;
}

// ---------------------------------------------------------------------------
// CEF - DA GO KHOI KE HOACH (07/08). Ghi lai de khong ai them lai nham.
//
// Y dinh ban dau: dua CEF vao chinh plugin nay, vi CEF goc (`mmscef-code`)
// VON LA Metamod plugin chu khong phai SourceMod extension.
//
// NGUOI DUNG CHOT: KHONG chep CEF vao day. Ma nguon CEF goc chi de THAM KHAO,
// no KHONG ho tro day du L4D2 - can thiet ke lai neu muon co co che nay.
//
// Ly do ky thuat:
//   CEF goc dung `PEntityOfEntIndex` de tim slot trong. Tren L4D2, L4D da BO
//   ham do khoi IVEngineServer, nen `engine_wrappers.h` thay bang phep tinh
//   con tro thuan - LUON khac NULL => vong lap chay toi maxEntities roi bail.
//   Tuc CEF goc la mot NO-OP tren L4D2. No "on dinh" vi no khong lam gi ca.
//   => Chep nguyen xi sang day la chep mot thu khong chay.
//
// Neu ve sau can co che nay, phai THIET KE LAI cho L4D2:
//   - dung `edict_t::IsFree()` that, khong dung PEntityOfEntIndex
//   - va DO TRUOC: hien chua co so lieu nao cho thay co dinh nguy hiem luc
//     choi thuong. Do duoc 07/08: slot cao nhat tung dung = 682/2048, luon du
//     ~950 cho. Moi dot bung do duoc deu nam o nhanh wipe, va `wipeclear` da
//     xu ly.
//   - va nho rui ro muc 0-AA: tac gia CEF tu canh bao "PROBABLY UNSTABLE...
//     random crashing", va crash sourcemod+0x13b63 xuat hien dung khi ep chi so.
// ---------------------------------------------------------------------------

static void LoadPatchSwitches() {
    FILE* f = OpenPluginFile("patches.txt", "r");
    if (!f) return;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || !*p) continue;

        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = 0;

        char* end = eq - 1;                       // trim the key
        while (end >= p && (*end == ' ' || *end == '\t')) *end-- = 0;
        bool on = (atoi(eq + 1) != 0);

        if      (!_stricmp(p, "freetime"))    g_PatchFreetime    = on;
        else if (!_stricmp(p, "indexbounds")) g_PatchIndexBounds = on;
        else if (!_stricmp(p, "forcedindex")) g_PatchForcedIndex = on;
        else if (!_stricmp(p, "bigarray"))    g_PatchBigArray    = on;
        else if (!_stricmp(p, "snapshot"))    g_PatchSnapshot    = on;
        else if (!_stricmp(p, "detour"))      g_PatchDetour      = on;
        else if (!_stricmp(p, "reuse"))       g_ImmediateReuse   = on;
        else if (!_stricmp(p, "freegate"))    g_PatchFreeGate    = on;
        else if (!_stricmp(p, "pinmax"))      g_PinMax           = on;
        else if (!_stricmp(p, "pinglobals"))  g_PinGlobals       = on;
        else if (!_stricmp(p, "markfree"))    g_MarkFree         = on;
        else if (!_stricmp(p, "wipeclear"))   g_WipeClear        = atoi(eq + 1);  // 0/1/2
        else if (!_stricmp(p, "trap"))        g_PatchTrap        = on;
        else if (!_stricmp(p, "nonetkill"))   g_NoNetKill        = on;
        else if (!_stricmp(p, "nonethigh"))   g_NoNetHigh        = on;
        else if (!_stricmp(p, "noedict"))     g_NoEdict          = on;
        else if (!_stricmp(p, "mapclear"))    g_MapClear         = atoi(eq + 1);  // 0/1/2
        else if (!_stricmp(p, "mapclearmax")) g_MapClearMax      = atoi(eq + 1);  // 0 = khong gioi han
        else if (!_stricmp(p, "mapclearcarry")) g_MapClearCarryOnly = on;         // 1 = chi xoa cai mang sang
        else if (!_stricmp(p, "loadprobe"))   g_LoadProbe        = atoi(eq + 1);  // so frame lay mau sau khi nap
        else if (!_stricmp(p, "swap"))        g_Swap             = atoi(eq + 1);  // 0/1/2, xem InstallSwap()
        else if (!_stricmp(p, "swapmax"))     g_SwapMax          = atoi(eq + 1);  // 0 = khong gioi han
        else if (!_stricmp(p, "heartbeat"))   g_Heartbeat        = atoi(eq + 1);  // giay, 0 = tat
        else if (!_stricmp(p, "logconsole"))  g_LogConsole       = on;
    }
    fclose(f);

    EL_LOG("[EdictBudget] patches: freetime=%d indexbounds=%d forcedindex=%d "
             "bigarray=%d snapshot=%d pinmax=%d pinglobals=%d markfree=%d detour=%d reuse=%d freegate=%d wipeclear=%d trap=%d nonetkill=%d nonethigh=%d",
             g_PatchFreetime, g_PatchIndexBounds, g_PatchForcedIndex,
             g_PatchBigArray, g_PatchSnapshot, g_PinMax, g_PinGlobals, g_MarkFree,
             g_PatchDetour, g_ImmediateReuse, g_PatchFreeGate, g_WipeClear, g_PatchTrap,
             g_NoNetKill, g_NoNetHigh);

    if (g_NoNetHigh && !(g_PatchBigArray && g_PatchSnapshot))
        EL_LOG("[EdictBudget] !!  nonethigh=1 nhung bigarray=%d snapshot=%d "
                 "- can CA HAI =1, neu khong no KHONG lam gi ca",
                 g_PatchBigArray, g_PatchSnapshot);
    if (g_NoNetKill && g_NoNetHigh)
        EL_LOG("[EdictBudget] !!  bat CA HAI nonetkill va nonethigh - chung mau "
                 "thuan. nonetkill xoa entity truoc khi no kip duoc cap edict.");
}

// ==========================================================================
// Danh sach cho phep: classname nao duoc nam o tren 2047
// ==========================================================================
#define MAX_PREFIX 64
static char g_Prefix[MAX_PREFIX][32];
static int  g_PrefixCount = 0;

static void AddPrefix(const char* s) {
    if (g_PrefixCount >= MAX_PREFIX || !s || !*s) return;
    strncpy(g_Prefix[g_PrefixCount], s, sizeof(g_Prefix[0]) - 1);
    g_Prefix[g_PrefixCount][sizeof(g_Prefix[0]) - 1] = 0;
    g_PrefixCount++;
}

static void LoadAllowList() {
    g_PrefixCount = 0;

    FILE* f = OpenPluginFile("serveronly.txt", "r");
    if (f) {
        char line[128];
        while (fgets(line, sizeof(line), f)) {
            char* p = line;
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '#' || *p == '\r' || *p == '\n' || !*p) continue;
            char* e = p;
            while (*e && *e != '\r' && *e != '\n' && *e != ' ' && *e != '\t' && *e != '#') e++;
            *e = 0;
            AddPrefix(p);
        }
        fclose(f);
        EL_LOG("[EdictBudget] allow-list: %d prefixes from serveronly.txt", g_PrefixCount);
        if (g_PrefixCount) return;
    }

    // Mac dinh gan san, chon THAN TRONG. Cac ho nay hoat dong hoan toan qua I/O
    // theo TEN; khong co EHANDLE cua lop co mang nao tro toi chung, nen chi so
    // tren 2047 khong the lam hong mot handle. Muon mo rong thi ghi vao
    // serveronly.txt sau khi da xac minh.
    AddPrefix("logic_");
    AddPrefix("math_");
    AddPrefix("ai_");
    EL_LOG("[EdictBudget] allow-list: built-in default (%d prefixes)", g_PrefixCount);
}

// Muc ket thuc bang '_' la TIEN TO CUA CA HO ("logic_" bao ca logic_relay va
// moi thu khac trong ho do). Moi thu khac phai khop classname CHINH XAC.
//
// Phan biet nay quan trong: neu coi "light" la tien to thi no se nuot luon ca
// light_dynamic - lop nay khac voi den nuong san, no CO MANG, va phai nam duoi 2048.
static bool MayLiveHigh(const char* cls) {
    if (!cls) return false;
    for (int i = 0; i < g_PrefixCount; i++) {
        size_t len = strlen(g_Prefix[i]);
        if (len == 0) continue;
        if (g_Prefix[i][len - 1] == '_') {
            if (strncmp(cls, g_Prefix[i], len) == 0) return true;
        } else {
            if (strcmp(cls, g_Prefix[i]) == 0) return true;
        }
    }
    return false;
}

// ==========================================================================
// Audit: record exactly what we put above 2047, dump once per level
// ==========================================================================
#define AUDIT_MAX 64
static char g_AuditName[AUDIT_MAX][40];
static int  g_AuditCount[AUDIT_MAX];
static int  g_AuditUsed = 0, g_AuditOverflow = 0, g_AuditTotal = 0;

static void AuditReset() { g_AuditUsed = g_AuditOverflow = g_AuditTotal = 0; }

static void AuditAdd(const char* name) {
    if (!name) name = "(null)";
    g_AuditTotal++;
    for (int i = 0; i < g_AuditUsed; i++) {
        if (strcmp(g_AuditName[i], name) == 0) { g_AuditCount[i]++; return; }
    }
    if (g_AuditUsed >= AUDIT_MAX) { g_AuditOverflow++; return; }
    strncpy(g_AuditName[g_AuditUsed], name, sizeof(g_AuditName[0]) - 1);
    g_AuditName[g_AuditUsed][sizeof(g_AuditName[0]) - 1] = 0;
    g_AuditCount[g_AuditUsed] = 1;
    g_AuditUsed++;
}

static void AuditDump() {
    EL_LOG("[EdictBudget] AUDIT: %d entities in %d-%d across %d classes%s",
        g_AuditTotal, NET_LIMIT, EXT_LIMIT - 1, g_AuditUsed,
        g_AuditOverflow ? " (list truncated)" : "");
    for (int i = 0; i < g_AuditUsed; i++) {
        EL_LOG("[EdictBudget]    x%-4d %s", g_AuditCount[i], g_AuditName[i]);
    }
}

// --------------------------------------------------------------------------
// Kiem ke MOI lop ma map tao ra, du ta co doi cho no hay khong.
//
// Chon danh sach cho phep bang truc giac thi KHONG AN THUA: tap than trong
// logic_/math_/ai_ chi giai phong duoc 10 o tren c1m1_hotel, vi L4D2 dat phan
// lon logic cua map trong VScript chu khong phai trong entity. Muon chon co ich
// thi phai biet map THUC SU sinh ra nhung gi va bao nhieu cai, sap theo so
// luong, de nhung nhom phia may chu lon nhat lo ra ngay.
// --------------------------------------------------------------------------
#define CENSUS_MAX 192
#define CENSUS_TRIP 1900          // do ra truoc khi cham buc tuong 2047
static bool g_Tripped = false;
static char g_CensusName[CENSUS_MAX][40];
static int  g_CensusCount[CENSUS_MAX];
static int  g_CensusUsed = 0, g_CensusTotal = 0, g_CensusDropped = 0;

static void CensusReset() { g_CensusUsed = g_CensusTotal = g_CensusDropped = 0; }

static void CensusAdd(const char* name) {
    if (!name || !*name) return;
    g_CensusTotal++;
    for (int i = 0; i < g_CensusUsed; i++) {
        if (strcmp(g_CensusName[i], name) == 0) { g_CensusCount[i]++; return; }
    }
    if (g_CensusUsed >= CENSUS_MAX) { g_CensusDropped++; return; }
    strncpy(g_CensusName[g_CensusUsed], name, sizeof(g_CensusName[0]) - 1);
    g_CensusName[g_CensusUsed][sizeof(g_CensusName[0]) - 1] = 0;
    g_CensusCount[g_CensusUsed] = 1;
    g_CensusUsed++;
}

static void CensusDump() {
    EL_LOG("[EdictBudget] CENSUS: %d entities created, %d distinct classes%s (sorted by count)",
        g_CensusTotal, g_CensusUsed, g_CensusDropped ? " (some dropped)" : "");

    // Sap xep chon don gian tren chi so - bang nho va ham nay chi chay mot lan
    // moi map.
    for (int a = 0; a < g_CensusUsed && a < 60; a++) {
        int best = a;
        for (int b = a + 1; b < g_CensusUsed; b++) {
            if (g_CensusCount[b] > g_CensusCount[best]) best = b;
        }
        if (best != a) {
            int c = g_CensusCount[a]; g_CensusCount[a] = g_CensusCount[best]; g_CensusCount[best] = c;
            char t[40];
            strncpy(t, g_CensusName[a], sizeof(t));
            strncpy(g_CensusName[a], g_CensusName[best], sizeof(g_CensusName[0]));
            strncpy(g_CensusName[best], t, sizeof(g_CensusName[0]));
        }
        EL_LOG("[EdictBudget]   %5d  %s%s", g_CensusCount[a], g_CensusName[a],
                 MayLiveHigh(g_CensusName[a]) ? "   <-- relocated" : "");
    }
}

// ==========================================================================
// engine.dll patches
// ==========================================================================

// Resolve sv.num_edicts / max_edicts / edicts / edictchangeinfo.
static bool ResolveEngineGlobals() {
    const char* sig  = "\x8B\x0D\x00\x00\x00\x00\x6A\x01\xC1\xE1\x04\x68\x00\x00\x00\x00\x51\xE8\x00\x00\x00\x00\xA3\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x15\x00\x00\x00\x00";
    const char* mask = "xx????xxxxxx????xx????x????x????xx????";
    uint8_t* m = FindPattern("engine.dll", sig, mask);
    if (!m) {
        EL_LOG("[EdictBudget] FATAL: engine globals signature not found.");
        return false;
    }
    g_max_edicts    = *(uint32_t**)(m + 2);
    g_edicts        = *(uint32_t**)(m + 23);
    g_edict_states  = g_edicts + 1;
    g_num_edicts    = g_max_edicts - 1;
    EL_LOG("[EdictBudget] globals: num=%p max=%p edicts=%p states=%p",
        g_num_edicts, g_max_edicts, g_edicts, g_edict_states);
    return true;
}

// Mang freetime la bang 2048 so thuc co dinh, danh chi so theo chi so edict,
// duoc ED_Alloc dung de quyet dinh khi nao mot o vua giai phong duoc tai dung.
// Tro no sang mot vung dem 4096 muc, va noi rong lenh memset xoa no (kich thuoc
// dang bi ghi cung la 0x2000).
static void PatchFreetime() {
    HMODULE h = GetModuleHandle("engine.dll");
    if (!h) return;
    MODULEINFO mi;
    GetModuleInformation(GetCurrentProcess(), h, &mi, sizeof(mi));
    uint8_t* base = (uint8_t*)mi.lpBaseOfDll;

    uint32_t oldPtr = (uint32_t)base + 0x6b3a58;
    float* fresh = (float*)_aligned_malloc(EXT_LIMIT * sizeof(float), 16);
    memset(fresh, 0, EXT_LIMIT * sizeof(float));

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
    PIMAGE_NT_HEADERS nt  = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
    PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt);
    uint8_t* text = NULL; size_t textLen = 0;
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        if (strncmp((char*)sec[i].Name, ".text", 5) == 0) {
            text = base + sec[i].VirtualAddress;
            textLen = sec[i].Misc.VirtualSize;
            break;
        }
    }
    if (!text) return;

    // Noi rong "push 0x2000" trong ham xoa bang, phai lam TRUOC khi con tro
    // ben duoi no bi ghi de (chu ky dung lai truoc con tro do).
    const char* sizeSig  = "\x68\x00\x20\x00\x00\x6A\x00\x68";
    const char* sizeMask = "xxxxxxxx";
    uint8_t* sz = FindPattern("engine.dll", sizeSig, sizeMask);
    if (sz) {
        uint32_t v = EXT_LIMIT * sizeof(float);
        WriteProtected(sz + 1, &v, sizeof(v));
        EL_LOG("[EdictBudget] freetime clear size 0x2000 -> 0x%X", v);
    }

    int n = 0;
    for (size_t i = 0; i + 4 <= textLen; i++) {
        uint32_t* p = (uint32_t*)(text + i);
        if (*p == oldPtr) {
            uint32_t v = (uint32_t)fresh;
            WriteProtected(p, &v, sizeof(v));
            n++; i += 3;
        }
    }
    EL_LOG("[EdictBudget] freetime: repointed %d references to a %d-entry table", n, EXT_LIMIT);
}

// ==========================================================================
// Doi hai bang theo-entity cua CFrameSnapshotManager ra khoi doi tuong engine
// ==========================================================================
//
// Day la ban va lam cho chi so tren 2047 song duoc, ma khong co no thi khong.
//
// CFrameSnapshotManager giu hai bang co dinh 2048 muc noi tuyen ben trong no:
//
//     +0x009C  CPackedEntity* m_pPackedData  [2048]
//     +0x209C  int            m_pSerialNumber[2048]
//
// Bay lenh danh chi so vao chung, khong lenh nao kiem bien - engine khong co ly
// do phai kiem, vi chi so edict goc khong bao gio vuot 2047. Do do ghi tai chi
// so i >= 2048 se roi vao bat cu thu gi nam ke sau:
//
//     m_pPackedData[i]   voi i >= 2048  ->  m_pSerialNumber[i - 2048]
//                        (AM THAM lam hong serial cua mot entity THAP - day
//                         chinh la thu khien EHANDLE giai ma ra con tro rac va
//                         sap ngay lan giai tham chieu ke tiep)
//     m_pSerialNumber[i] voi i >= 2993  ->  sv, doi tuong CGameServer
//                        (giu trang thai map va con tro worldmodel; hong roi thi
//                         lan kiem tra lump BSP ke tiep doc phai rac va engine
//                         bao "Cannot load corrupted map")
//
// Vay khong co dai con nao an toan: cap phat len tren thi hong serial, cap phat
// xuong duoi thi hong sv. Ca hai bang deu PHAI doi sang cho co du 4096 muc.
//
// Moi lan truy cap deu la dang SIB 7 byte "[base + index*4 + disp32]" va viet
// lai thanh "[index*4 + abs32]" DUNG BANG DO DAI, nen day la sua tai cho, khong
// can trampoline, khong co rui ro lech bien lenh:
//
//     8B BC B1 9C 00 00 00   mov edi,[ecx+esi*4+0x9C]   -> 8B 3C B5 <packed>
//     8B 84 B1 9C 20 00 00   mov eax,[ecx+esi*4+0x209C] -> 8B 04 B5 <serial>
//     8B 84 91 9C 00 00 00   mov eax,[ecx+edx*4+0x9C]   -> 8B 04 95 <packed>
//     8B 8C 91 9C 20 00 00   mov ecx,[ecx+edx*4+0x209C] -> 8B 0C 95 <serial>
//     8B 94 B9 9C 00 00 00   mov edx,[ecx+edi*4+0x9C]   -> 8B 14 BD <packed>
//     89 94 B9 9C 00 00 00   mov [ecx+edi*4+0x9C],edx   -> 89 14 BD <packed>
//     89 94 B9 9C 20 00 00   mov [ecx+edi*4+0x209C],edx -> 89 14 BD <serial>
//
// Quet dich nguoc toan bo .text xac nhan day la BAY lan truy cap chi-so-co-ty-le
// duy nhat o ca hai do doi; cac tham chieu +0x9C khac thuoc ve doi tuong khong
// lien quan (ghi kich thuoc word, bien cuc bo tinh theo esp).
//
// Sot du mot cai thoi la engine se doc mot bang trong khi ghi vao bang kia, nen
// so luong nay da duoc kiem chung va ban va la DUOC-TAT-CA-HOAC-KHONG.

static uint32_t* g_PackedTable = NULL;   // 4096 muc
static uint32_t* g_SerialTable = NULL;   // 4096 muc

struct SnapAccess { const char* sig; const char* head; bool serial; };

static bool PatchSnapshotTables() {
    static const SnapAccess kSites[] = {
        { "\x8B\xBC\xB1\x9C\x00\x00\x00", "\x8B\x3C\xB5", false },
        { "\x8B\x84\xB1\x9C\x20\x00\x00", "\x8B\x04\xB5", true  },
        { "\x8B\x84\x91\x9C\x00\x00\x00", "\x8B\x04\x95", false },
        { "\x8B\x8C\x91\x9C\x20\x00\x00", "\x8B\x0C\x95", true  },
        { "\x8B\x94\xB9\x9C\x00\x00\x00", "\x8B\x14\xBD", false },
        { "\x89\x94\xB9\x9C\x00\x00\x00", "\x89\x14\xBD", false },
        { "\x89\x94\xB9\x9C\x20\x00\x00", "\x89\x14\xBD", true  },
    };
    const int kCount = sizeof(kSites) / sizeof(kSites[0]);

    // Tim du HET truoc khi dong vao bat cu thu gi.
    uint8_t* found[kCount];
    for (int i = 0; i < kCount; i++) {
        found[i] = FindPattern("engine.dll", kSites[i].sig, "xxxxxxx");
        if (!found[i]) {
            EL_LOG("[EdictBudget] snapshot table access %d/%d not found - "
                     "extended range stays DISABLED.", i + 1, kCount);
            return false;
        }
    }

    // Hai bang nay cung phai duoc xoa mot lan moi map, NHUNG KHONG duoc lam
    // bang cach sua ham xoa cua engine.
    //
    // Mot ban truoc da viet lai lenh memset ben trong
    // CFrameSnapshotManager::LevelChanged. Viec do lam hong mot extension khac:
    //
    //   [SM] Unable to load extension "cutlrbtreefix.ext":
    //        Failed to create_inline: CFrameSnapshotManager::LevelChanged
    //
    // cutlrbtreefix cai mot inline hook vao DUNG ham do, tim ham bang chu ky, va
    // may byte ta sua lam chu ky khong con khop - the la extension am tham ngung
    // nap, va may chu mat mot ban va engine that su ma no dang phu thuoc.
    //
    // Xoa bang CUA CHINH TA tu Hook_LevelInit cho ket qua y het, ma de nguyen ham
    // do khong doi mot byte nao. Engine van xoa mang noi tuyen gio da bo khong
    // cua no, ton 8KB memset moi map va khong hai gi.
    //
    // LUAT CHUNG rut ra tu cai gia nay: chi va nhung gi BUOC PHAI va, va uu tien
    // lam viec trong hook cua chinh minh hon la sua mot ham ma nguoi khac co the
    // dang moc vao.

    g_PackedTable = (uint32_t*)_aligned_malloc(EXT_LIMIT * sizeof(uint32_t), 16);
    g_SerialTable = (uint32_t*)_aligned_malloc(EXT_LIMIT * sizeof(uint32_t), 16);
    if (!g_PackedTable || !g_SerialTable) return false;
    memset(g_PackedTable, 0, EXT_LIMIT * sizeof(uint32_t));
    memset(g_SerialTable, 0, EXT_LIMIT * sizeof(uint32_t));

    // Bat dau tu ZERO la DUNG chu khong chi la tien: Metamod nap plugin luc may
    // chu khoi dong, TRUOC map dau tien, nen bang goc khong giu gi dang mang theo.
    // Chep lai co nghia la tin vao mot phong doan xem singleton nao so huu chung,
    // ma doan sai thi bang cua ta se duoc gieo bang con tro packed-entity treo.

    for (int i = 0; i < kCount; i++) {
        uint8_t buf[7];
        memcpy(buf, kSites[i].head, 3);
        *(uint32_t*)(buf + 3) = (uint32_t)(kSites[i].serial ? g_SerialTable : g_PackedTable);
        WriteProtected(found[i], buf, sizeof(buf));
    }

    EL_LOG("[EdictBudget] snapshot tables relocated: packed=%p serial=%p "
             "(%d entries each, %d accesses rewritten, cleared from our own hook)",
             g_PackedTable, g_SerialTable, EXT_LIMIT, kCount);
    return true;
}


// ==========================================================================
// Bo thoi gian cho 1 giay truoc khi tai su dung edict
// ==========================================================================
//
// ED_Alloc chi nhan mot edict da giai phong khi:
//     comiss  2.0f, freetime[i]      ; freetime < 2.0 (giai phong dau map)
//     ja      lay_no
//     fsub    freetime[i]            ; curtime - freetime
//     fcompi  1.0
//     jae     lay_no                 ; hoac da qua 1 GIAY   <-- va o day
//
// Do do wipe (xoa roi tao lai hang tram entity trong CUNG mot frame) khong co
// edict nao du dieu kien, engine buoc phai cap moi, num_edicts leo toi tran.
// Da do thuc te: num_edicts=2012 voi 906-918 edict DANG TRONG ma engine van
// bao "ED_Alloc: no free edicts". Day dung la loi engine Source 2009 ma tac
// gia CEF mo ta: "running out of edicts when you have 1000 free".
//
// Doi mot byte 73 -> EB (jae -> jmp) lam moi edict trong deu dung lai duoc
// ngay. Dich nhay giu nguyen, khong doi do dai lenh, khong trampoline.
//
// An toan: engine da co san sv_useexplicitdelete (mac dinh BAT) - khi mot chi
// so duoc tai dung som, no gui lenh xoa tuong minh xuong client truoc. Do
// chinh la co che Valve thiet ke thay cho thoi gian cho nay.
static void PatchFreetimeGate() {
    // fld1 ; fxch st(1) ; fcompi st(1) ; fstp st(0) ; jae
    const char* sig  = "\xD9\xE8\xD9\xC9\xDF\xF1\xDD\xD8\x73";
    const char* mask = "xxxxxxxxx";
    uint8_t* m = FindPattern("engine.dll", sig, mask);
    if (!m) {
        EL_LOG("[EdictBudget] freetime-gate: khong tim thay chu ky!");
        return;
    }
    uint8_t jmp = 0xEB;                 // 73 xx (jae) -> EB xx (jmp)
    WriteProtected(m + 8, &jmp, 1);
    EL_LOG("[EdictBudget] freetime-gate: bo thoi gian cho 1 giay tai %p "
             "- moi edict trong deu tai su dung duoc ngay", m + 8);
}


// ==========================================================================
// Bay tai CHINH nhanh loi cua ED_Alloc
// ==========================================================================
//
// Moi phep do dat tai IVEngineServer::CreateEdict deu MU: bo dem burst khong
// thay frame nao co >=32 lan cap phat, va hook chua bao gio duoc goi cho lan
// that bai. Nghia la ED_Alloc duoc goi tu duong noi bo cua engine.
//
// Cho duy nhat con nhin duoc la chinh nhanh loi:
//     1E0247  85 DB              test ebx, ebx
//     1E0249  0F 88 84 00 00 00  js   1E02D3      -> bao "no free edicts"
//     1E024F  ...                                 -> tai su dung ebx
//
// Tam 8 byte do bang mot JMP 5 byte toi stub cua ta + 3 NOP. Stub ghi log roi
// dung lai dung hai nhanh goc. Day la duong LANH - chi chay khi engine sap
// chet - nen rui ro thap hon han detour tren duong nong.
//
// ebx = chi so edict trong CUOI CUNG ma vong quet nhin thay (-1 = khong thay
// cai nao). Do chinh la con so can biet: engine co that su khong thay slot
// trong nao khong, trong khi ta dem duoc ~912.
static uint8_t* g_AllocFailStub = NULL;
static int g_AllocFailReports = 0;

static void __cdecl LogAllocFail(int ebxVal)
{
    if (g_AllocFailReports >= 4) return;
    g_AllocFailReports++;

    int freeCount = -1;
    if (gpGlobals && gpGlobals->pEdicts && g_num_edicts) {
        int n = (int)*g_num_edicts;
        if (n > EXT_LIMIT) n = EXT_LIMIT;
        uint8_t* arr = (uint8_t*)gpGlobals->pEdicts;
        freeCount = 0;
        for (int i = 0; i < n; i++)
            if (*(uint32_t*)(arr + i * EDICT_SIZE) & FL_FREE) freeCount++;
    }

    EL_LOG("[EdictBudget] *** ED_ALLOC SAP BAO LOI *** ebx(slot trong cuoi cung "
             "vong quet thay)=%d | num_edicts=%d max_edicts=%d | plugin dem duoc %d slot trong",
             ebxVal,
             g_num_edicts ? (int)*g_num_edicts : -1,
             g_max_edicts ? (int)*g_max_edicts : -1,
             freeCount);

    // Kiem ke TAI DUNG THOI DIEM NAY: cai gi dang chiem 2048 slot?
    //
    // Moi lan kiem ke truoc day deu dem luc BINH YEN va cho ra buc tranh khac
    // han - chinh no lam ca hai ngay di sai huong. Day la thoi diem duy nhat
    // co nghia: engine vua xac nhan khong con mot slot trong nao.
    //
    // Lan truoc bang nay KHONG in ra duoc: tieu de duoc log SAU vong lap, va
    // vong lap goi ham ao GetClassName() tren 2048 edict trong luc engine dang
    // hap hoi nen cham phai con tro hong va chet truoc khi kip in. Nay:
    //   - in tieu de TRUOC
    //   - boc SEH quanh moi lan doc mot edict
    //   - in tung dong ngay khi gom xong, khong doi toi cuoi
    if (gpGlobals && gpGlobals->pEdicts && g_num_edicts && g_AllocFailReports == 1) {
        int n = (int)*g_num_edicts;
        if (n > EXT_LIMIT) n = EXT_LIMIT;

        EL_LOG("[EdictBudget] === KIEM KE LUC HET EDICT: bat dau quet %d slot ===", n);

        static char name[128][40];
        static int  count[128];
        int used = 0, total = 0, unreadable = 0;

        for (int i = 0; i < n; i++) {
            const char* cn = NULL;
            __try {
                edict_t* e = gpGlobals->pEdicts + i;
                if (e->IsFree()) continue;
                total++;
                IServerNetworkable* net = e->GetNetworkable();
                if (net) cn = net->GetClassName();
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                unreadable++;
                continue;
            }
            if (!cn || !*cn) { unreadable++; continue; }

            int j = 0;
            for (; j < used; j++) if (strcmp(name[j], cn) == 0) { count[j]++; break; }
            if (j == used && used < 128) {
                strncpy(name[used], cn, sizeof(name[0]) - 1);
                name[used][sizeof(name[0]) - 1] = 0;
                count[used] = 1;
                used++;
            }
        }

        EL_LOG("[EdictBudget] === %d thuc the dang song, %d lop, %d khong doc duoc ===",
                 total, used, unreadable);

        for (int a = 0; a < used && a < 30; a++) {
            int best = a;
            for (int b = a + 1; b < used; b++) if (count[b] > count[best]) best = b;
            if (best != a) {
                int c = count[a]; count[a] = count[best]; count[best] = c;
                char t[40];
                strncpy(t, name[a], sizeof(t));
                strncpy(name[a], name[best], sizeof(name[0]));
                strncpy(name[best], t, sizeof(name[0]));
            }
            EL_LOG("[EdictBudget]   %5d  %s", count[a], name[a]);
        }
    }
}

static void PatchAllocFailTrap() {
    // "test ebx,ebx ; js rel32" mot minh xuat hien 16 lan trong engine.dll.
    // Phai neo them hai lenh phia truoc - "cmp ecx,eax ; jl rel32" - moi duy
    // nhat (da xac minh: dung 1 lan, tai RVA 0x1E023F).
    const char* sig  = "\x3B\xC8\x0F\x8C\x00\x00\x00\x00\x85\xDB\x0F\x88\x00\x00\x00\x00";
    const char* mask = "xxxx....xxxx....";
    uint8_t* anchor = FindPattern("engine.dll", sig, mask);
    uint8_t* m = anchor ? anchor + 8 : NULL;   // tro toi "test ebx,ebx"
    if (!m) {
        EL_LOG("[EdictBudget] alloc-fail trap: khong tim thay chu ky!");
        return;
    }

    uint8_t* errTarget  = m + 8 + *(int32_t*)(m + 4);   // dich cua js  -> nhanh bao loi
    uint8_t* contTarget = m + 8;                        // roi qua      -> tai su dung

    uint8_t* stub = (uint8_t*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE,
                                           PAGE_EXECUTE_READWRITE);
    if (!stub) return;
    g_AllocFailStub = stub;

    int k = 0;
    stub[k++] = 0x60;                                   // pushad
    stub[k++] = 0x9C;                                   // pushfd
    stub[k++] = 0x53;                                   // push ebx
    stub[k++] = 0xE8;                                   // call LogAllocFail
    *(int32_t*)(stub + k) = (int32_t)((uint8_t*)LogAllocFail - (stub + k + 4)); k += 4;
    stub[k++] = 0x83; stub[k++] = 0xC4; stub[k++] = 0x04;   // add esp,4
    stub[k++] = 0x9D;                                   // popfd
    stub[k++] = 0x61;                                   // popad
    stub[k++] = 0x85; stub[k++] = 0xDB;                 // test ebx,ebx
    stub[k++] = 0x0F; stub[k++] = 0x88;                 // js errTarget
    *(int32_t*)(stub + k) = (int32_t)(errTarget - (stub + k + 4)); k += 4;
    stub[k++] = 0xE9;                                   // jmp contTarget
    *(int32_t*)(stub + k) = (int32_t)(contTarget - (stub + k + 4)); k += 4;

    uint8_t patch[8];
    patch[0] = 0xE9;
    *(int32_t*)(patch + 1) = (int32_t)(stub - (m + 5));
    patch[5] = patch[6] = patch[7] = 0x90;              // nop
    WriteProtected(m, patch, sizeof(patch));

    EL_LOG("[EdictBudget] alloc-fail trap: cai tai %p (stub %p, loi->%p, tiep->%p)",
             m, stub, errTarget, contTarget);
}

// SV_AllocateEdicts mo dau bang "mov eax, 0x800" roi lay so do de dinh kich thuoc
// ca mang edict lan mang changeinfo. Nang mot gia tri tuc thoi do la CHINH ENGINE
// se cap phat mot mang 4096 muc that su, dung thoi diem, khong can ta lam tro gi
// voi con tro.
// Dword o cuoi la dia chi tuyet doi DA DUOC DOI THEO BASE, nen phai dat mat na.
static bool PatchEngineAllocSize() {
    const char* sig  = "\xB8\x00\x08\x00\x00\x89\x86\x18\x02\x00\x00\xA3\x00\x00\x00\x00";
    const char* mask = "xxxxxxxxxxxx....";
    uint8_t* m = FindPattern("engine.dll", sig, mask);
    if (!m) {
        EL_LOG("[EdictBudget] SV_AllocateEdicts signature not found - staying at 2048.");
        return false;
    }
    uint32_t v = EXT_LIMIT;
    WriteProtected(m + 1, &v, sizeof(v));
    EL_LOG("[EdictBudget] SV_AllocateEdicts now allocates %d edicts (patched at %p)", EXT_LIMIT, m);
    return true;
}

// engine.dll co hai ham tra chi so gan giong het nhau:
//   RVA 0x1E0030  IndexOfEdict      - kiem bien theo num_edicts
//   RVA 0x1E0060  IndexOfEdictInfo  - kiem bien theo max_edicts
// Bo cuc ca hai:
//   +20  cmp esi, [gioi han]
//   +26  jl  +41              ; trong tam -> tra ve chi so
//   +28  push "NUM_FOR_EDICT[INFO]: bad pointer"
//   +33  call Error           ; TU VONG - khong bao gio tra ve
//   +41  mov eax, esi ; ret
//
// Ta CO Y giu max_edicts o 2048 de bo cap phat cua chinh engine nam trong dai
// 11 bit co the noi mang. Nhung mang thuc su la 4096 muc, nen mot edict hoan
// toan hop le o chi so 2048+ se lam vap hai phep kiem nay va engine bo cuoc voi
// "NUM_FOR_EDICTINFO: bad pointer".
//
// Doi lenh nhay co dieu kien thanh vo dieu kien se bo qua loi goi tu vong ma van
// tra ve dung chi so - thu duy nhat mat di la phep kiem bien, ma no dang kiem
// theo mot gioi han chinh ta co tinh khai thap hon su that.
//
// Neo theo prologue cua ham la MOT SAI LAM: no tim duoc hai cho va am tham bo sot
// phan con lai. Co BON cho bo cuoc nhu vay nam trong BA ham co prologue khac nhau,
// va mot trong nhung cho bi bo sot lai duoc chinh ED_Alloc goi de xoa mot edict
// vua cap - nen moi entity duoc doi cho deu vap phai no ngay khi bat phan tach.
//
// Chuoi thong bao loi moi la cai neo dang tin. Moi cho bo cuoc deu duoc toi bang
// mot lenh nhay co dieu kien nam ngay truoc "push <chuoi>", nen tim cac tham chieu
// toi chuoi la tim ra HET, bat ke hinh dang ham ra sao:
//
//     3B 35 <gioi han>  cmp esi, [max_edicts]
//     7C xx             jl  <trong tam>       <- doi thanh EB, cung dich
//     68 <chuoi>        push "NUM_FOR_...: bad pointer"
//     E8 <rel>          call Error            <- khong bao gio tra ve
//
// Doi jl thanh jmp giu nguyen byte dich, nen luong dieu khien roi dung vao noi ma
// truong hop trong-tam le ra da roi vao. Khong mat gi ngoai mot phep kiem theo
// gioi han ma ta co tinh khai thap.
static int PatchAbortsFor(uint8_t* base, size_t size, const char* text) {
    size_t tlen = strlen(text);

    // Tim chuoi truoc, roi san lenh "push <dia chi luc chay cua no>". Doc dia chi
    // ra tu chinh anh module thi khong so bi doi theo base.
    uint8_t* str = NULL;
    for (size_t i = 0; i + tlen + 1 <= size; i++) {
        if (memcmp(base + i, text, tlen + 1) == 0) { str = base + i; break; }
    }
    if (!str) return 0;

    int done = 0;
    for (size_t i = 3; i + 5 <= size; i++) {
        if (base[i] != 0x68) continue;
        if (*(uint8_t**)(base + i + 1) != str) continue;

        // Co HAI bo cuc, va chi tim moi bo cuc dau la thu da lam mot ban truoc
        // bo sot han EDICT_NUM:
        //
        //   7C xx       jl trong-tam         <- lenh canh nam lui 2 byte
        //   68 <chuoi>  push "..."
        //
        //   7C xx       jl trong-tam         <- lenh canh nam lui 3 byte
        //   56          push esi             (mot tham so printf phu)
        //   68 <chuoi>  push "..."
        size_t at;
        if (base[i - 2] == 0x7C)                                   at = i - 2;
        else if (base[i - 3] == 0x7C && base[i - 1] >= 0x50
                                     && base[i - 1] <= 0x57)       at = i - 3;
        else continue;

        uint8_t jmp = 0xEB;
        WriteProtected(base + at, &jmp, 1);
        EL_LOG("[EdictBudget] bounds abort at %p disabled (%s)", base + at, text);
        done++;
    }
    return done;
}

static void PatchIndexOfEdictBounds() {
    HMODULE h = GetModuleHandle("engine.dll");
    if (!h) return;
    MODULEINFO mi;
    if (!GetModuleInformation(GetCurrentProcess(), h, &mi, sizeof(mi))) return;

    uint8_t* base = (uint8_t*)mi.lpBaseOfDll;
    size_t size = mi.SizeOfImage;

    // EDICT_NUM la phep tra chi so -> edict_t* nam sau PEntityOfEntIndex, ma
    // SourceMod, cac plugin va ban than game goi lien tuc. No so chi so voi
    // max_edicts, ma ta thi CO Y giu con so do o 2048 - nen moi lan tra cuu mot
    // entity ta da dat vao 2048..4095 deu giet may chu.
    int n = PatchAbortsFor(base, size, "NUM_FOR_EDICT: bad pointer")
          + PatchAbortsFor(base, size, "NUM_FOR_EDICTINFO: bad pointer")
          + PatchAbortsFor(base, size, "EDICT_NUM: bad number %i");

    if (n == 0) EL_LOG("[EdictBudget] no index-lookup bounds aborts found!");
    else        EL_LOG("[EdictBudget] index-lookup: %d bounds aborts disabled, "
                         "index preserved at every one (expect 6)", n);

    // Kiem xem gpGlobals->maxEntities THUC SU roi vao dau. Bo cuc CGlobalVars
    // trong SDK chi la mot GIA DINH, va hai truong nam hai ben maxEntities la
    // nTimestampNetworkingBase (100) va nTimestampRandomizeWindow (32) - chung
    // dieu khien cach ma hoa thuoc tinh theo so tick. Ghi 2048 de len mot trong
    // hai truong do la lam hong ma hoa tren toan may chu - nen phai KIEM chu
    // khong duoc GIA DINH.
    if (gpGlobals) {
        void* p = &gpGlobals->maxEntities;
        EL_LOG("[EdictBudget] gpGlobals=%p  &maxEntities=%p (engine+0x%X)  value=%d",
                 gpGlobals, p, (uint32_t)((uint8_t*)p - base), gpGlobals->maxEntities);
    }
}

// Nhanh "ep chi so" cua ED_Alloc kiem tinh hop le theo num_edicts, ma con so do
// chi dem NHUNG GI DA CAP RA cho toi luc nay - nen mot chi so ep buoc 2048+ luon
// bi tu choi. No phai duoc so voi mot con so lon hon.
//
// Nuoc di hien nhien - tro no sang max_edicts roi nang con so do len 4096 trong
// suot loi goi - la mot cai BAY. num_edicts chi tang chu khong bao gio giam, va
// tran cua no la max_edicts; nen chi mot frame co max_edicts == 4096 la du de
// num_edicts vuot 2048 VINH VIEN. Sau do TakeTickSnapshot duyet len toi num_edicts
// trong khi do day mot mang 2048 muc tren ngan xep, ghi tran qua stack cookie, va
// tien trinh chet khong minidump, khong mot dong nao tren console.
//
// So voi mot gia tri tuc thoi thuan tuy thi xoa bo han cua so nguy hiem do:
// khong mot bien toan cuc nao cua engine bi thay doi, nen khong gi quan sat duoc
// mot gioi han bi thoi phong.
//
//     3B 05 <abs32>   cmp eax, [num_edicts]   (6 byte)
//  -> 3D 00 10 00 00  cmp eax, 4096           (5 byte) + nop
//
// Nhanh noi-them o ben duoi van doc max_edicts, ma con so do van bi ghim o 2048 -
// nen cap phat thong thuong khong the leo qua dai co the noi mang. Chi mot chi so
// bi ep TUONG MINH moi cham toi nua tren, va nhanh do van kiem FL_EDICT_FREE
// truoc khi giao o ra.
static void PatchForcedIndexCheck() {
    const char* sig  = "\x55\x8B\xEC\x8B\x45\x08\x56\x85\xC0\x78\x00\x3B\x05\x00\x00\x00\x00\x7C";
    const char* mask = "xxxxxxxxxx.xx....x";
    uint8_t* m = FindPattern("engine.dll", sig, mask);
    if (!m) {
        EL_LOG("[EdictBudget] ED_Alloc forced-index signature not found.");
        return;
    }
    uint8_t buf[6] = { 0x3D, 0, 0, 0, 0, 0x90 };
    *(uint32_t*)(buf + 1) = EXT_LIMIT;
    WriteProtected(m + 11, buf, sizeof(buf));
    EL_LOG("[EdictBudget] ED_Alloc forced-index now compares against the "
             "constant %d - no engine global is touched at runtime (at %p)", EXT_LIMIT, m);
}

// ==========================================================================
// Detour CreateEntityByName - cach duy nhat de biet classname cua entity ma lan
// goi CreateEdict sap toi thuoc ve.
//
// Ban server.dll nay KHONG co chuoi "CreateEntityByName\0" doc lap, nen ta neo
// theo thong bao loi duy nhat nam ben trong chinh ham do. Prologue that su dai
// 7 byte (55 8B EC 56 8B 75 0C); cuop 5 byte nhu thong le se cat doi "8B 75 0C"
// va thuc thi rac.
// ==========================================================================
typedef void* (__cdecl *CreateEntityByName_t)(const char*, int);
static CreateEntityByName_t g_OrigCreateEntity = NULL;
static uint8_t* g_DetourAt = NULL;
static uint8_t  g_DetourSaved[7];
static uint8_t* g_Trampoline = NULL;
static const char* g_PendingClass = NULL;

static void* __cdecl Detour_CreateEntityByName(const char* cls, int forceIndex) {
    const char* prev = g_PendingClass;   // loi tao long nhau khong duoc ke thua
    g_PendingClass = cls;
    CensusAdd(cls);
    void* r = g_OrigCreateEntity(cls, forceIndex);
    g_PendingClass = prev;

    // Mot map het edict thi khong bao gio toi duoc ServerActivate, nen ban kiem ke
    // se khong bao gio duoc in ra cho DUNG cai map ma ta can no nhat.
    // Thay vao do, do ra MOT LAN ngay khi dai co mang bat dau day.
    if (!g_Tripped && g_num_edicts && *g_num_edicts >= CENSUS_TRIP) {
        g_Tripped = true;
        EL_LOG("[EdictBudget] --- num_edicts reached %d, early census ---",
                 (int)*g_num_edicts);
        CensusDump();
    }
    return r;
}

// ---------------------------------------------------------------------------
// WIPECLEAR - don thuc the o DAU chuoi restart, TRUOC vong hoi sinh player.
//
// CTerrorGameRules::CleanUpMap() (RVA 0x2DDB10) da tu lam dung viec nay:
//     UTIL_Remove(moi thu ngoai preserve list)
//       -> CleanupDeleteList() -> AllowImmediateEdictReuse()
//       -> MapEntity_ParseAllEntities()
// Van de la no chay MUON. Trinh tu that (da kiem bang capstone tren server.dll
// cua chinh server nay, 9.130.288 byte, ImageBase 0x10000000):
//
//   CDirector::Restart          0x2700D0
//     m_bRestarting = 1         0x27045F
//     RestartRound()            0x2704C4   <- vtable slot 178
//       VONG HOI SINH PLAYER    0x2E0794..0x2E08A3   <== tieu edict O DAY
//       FIRE round_start_pre_entity        0x2E08CE
//       CleanUpMap()            0x2E08DF   <== game moi don O DAY
//     m_bRestarting = 0         0x2705DF
//
// Moi thu truoc 0x2E08DF chay khi map VAN giu du 2012 entity / 35 slot trong.
// Khoi nay lam nua "don" cua CleanUpMap ngay dau RestartRound roi de game chay
// tiep binh thuong - CleanUpMap se thay gan nhu khong con gi de xoa, va
// MapEntity_ParseAllEntities van dung lai day du tu entity lump.
//
// QUAN TRONG - day vua la BAN VA vua la PHEP DO:
//   log "slot trong truoc -> sau" tra loi luon cau hoi con treo:
//     +~1100 slot va het crash  => lo nam TRUOC CleanUpMap, ban va dung
//     +~1100 slot ma van crash  => lo nam SAU khi dung lai xong; luc do bai
//                                  toan tro ve muc 0-KET-LUAN (map that su
//                                  can 2012/2047, khong co lang phi de thu hoi)
//
// Giu nguyen tap "preserve" CUA CHINH GAME (doc runtime tu RVA 0x7ACE40) nen
// ngu nghia y het CleanUpMap - chi khac THOI DIEM. Do la lua chon co y: doi
// mot bien duy nhat.
// ---------------------------------------------------------------------------

// --- Nghe su kien: CHI DE CHAN DOAN, khong con quyen chan ------------------
//
// Ban dau dinh dung 'mission_lost' lam cong chan. DA BO (phuong an A).
// Ly do, xac minh tren binary chu khong phai suy doan:
//   mission_lost ban DUY NHAT tai 0x10269096, trong ham 0x10268CA0. Ham do chi
//   push bon chuoi: 'trigger_finale', 'finale_trigger', 'FinaleLost',
//   'mission_lost' => day la duong THUA FINALE.
//   11 vi tri push mission_lost con lai deu la AddListener(+0x0C) hoac so chuoi.
//   c6m1_riverbank khong phai finale => cong se khong bao gio mo.
//
// Van giu listener vi no tra loi mot cau van con treo: thuc te mission_lost co
// ban khong, va ban truoc hay sau RestartRound. Log se noi.
// CO MOT-LAN, KHONG dung cua so thoi gian.
//
// Ban dau dung cua so 5.0s. SAI: do that te cho thay mission_lost ban luc
// t=63.47 con RestartRound chay luc t=70.50 - cach 7.03s, VUOT cua so 5s
// => cong se truot luon ca wipe that.
// Khoang cach nay do Director quyet dinh (man hinh thua, dem nguoc...), khong
// co gia tri nao an toan de doan. Dung co mot-lan thi khong phai doan:
//   mission_lost  -> bat co
//   RestartRound  -> co bat thi don, roi TAT co ngay
//   nap map moi   -> tat co (tranh co cu sot lai)
static bool  g_LossPending   = false;
static float g_LastLossTime  = -1.0f;
static char  g_LastLossName[32] = "";
static int   g_LossSignals   = 0;

// WIPE = doi survivor THUA = su kien 'mission_lost'.
// CHI 'mission_lost' duoc mo cong. 'round_end' cung ban khi THANG (qua chuong,
// finale) nen chi ghi log de doi chieu thu tu, KHONG mo cong.
class CLossListener : public IGameEventListener2 {
public:
    virtual void FireGameEvent(IGameEvent *ev) {
        if (!ev) return;
        const char* n = ev->GetName();
        if (!n) return;
        float t = gpGlobals ? gpGlobals->curtime : 0.0f;

        if (_stricmp(n, "mission_lost") == 0) {
            g_LossPending  = true;
            g_LastLossTime = t;
            strncpy(g_LastLossName, n, sizeof(g_LastLossName) - 1);
            g_LastLossName[sizeof(g_LastLossName) - 1] = 0;
            g_LossSignals++;
            EL_LOG("[EdictBudget] WIPECLEAR: *** WIPE (mission_lost) *** t=%.2f "
                     "(lan thu %d) - MO CONG", t, g_LossSignals);
        } else {
            // chi doi chieu thu tu, khong mo cong
            EL_LOG("[EdictBudget] WIPECLEAR: (doi chieu) su kien '%s' t=%.2f "
                     "- KHONG mo cong", n, t);
        }
    }
    virtual int GetEventDebugID() { return 42; }
};
static CLossListener g_LossListener;
static bool g_LossHooked = false;

typedef void  (__fastcall *fnCleanupDeleteList_t)(void*, void*);
typedef void* (__fastcall *fnNextEnt_t)(void*, void*, void*);
typedef void  (__cdecl    *fnUtilRemove_t)(void*);
typedef void  (__fastcall *fnRestartRound_t)(void*, void*);

static fnCleanupDeleteList_t g_CleanupDeleteList = NULL;
static fnNextEnt_t           g_NextEnt           = NULL;
static fnUtilRemove_t        g_UtilRemove        = NULL;
static void*                 g_EntList           = NULL;
static bool*                 g_InCleanupDelete   = NULL;
static const char**          g_PreserveList      = NULL;
static fnRestartRound_t      g_OrigRestartRound  = NULL;
static void**                g_RestartSlot       = NULL;
static int                   g_WipeClearRuns     = 0;

static int CountFreeEdicts() {
    if (!gpGlobals || !gpGlobals->pEdicts || !g_num_edicts) return -1;
    int n = (int)*g_num_edicts;
    if (n > EXT_LIMIT) n = EXT_LIMIT;
    uint8_t* arr = (uint8_t*)gpGlobals->pEdicts;
    int f = 0;
    for (int i = 0; i < n; i++)
        if (*(uint32_t*)(arr + i * EDICT_SIZE) & FL_FREE) f++;
    return f;
}

// Preserve list that cua L4D2: 38 chuoi + mot chuoi RONG lam END marker.
// Khong ro classname => GIU (an toan hon la xoa nham).
static bool InPreserveList(const char* cls) {
    if (!g_PreserveList || !cls || !*cls) return true;
    for (int i = 0; i < 64; i++) {
        const char* s = g_PreserveList[i];
        if (!s || !*s) break;
        if (strcmp(s, cls) == 0) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// DANH SACH GIU BO SUNG - wipekeep.txt
//
// Preserve list cua game (0x7ACE40) la thu game DUNG. Nhung co nhung lop game
// san sang xoa ma XOA SOM lai sinh loi phia client. Ca dau tien gap:
//   viec giu lai thuc the cua nguoi choi gay loi MAT BONG.
//
// Nen can mot danh sach GIU THEM, sua duoc bang file, khong phai build lai -
// dung kieu nhu serveronly.txt:
//   dong ket thuc bang '_'  -> khop TIEN TO ca ho   (vi du "weapon_")
//   con lai                 -> khop CHINH XAC ten lop
//
// Dat o: left4dead2/addons/edictbudget/wipekeep.txt
// Thieu file = khong giu them gi (chi dung preserve list cua game).
// ---------------------------------------------------------------------------
#define KEEP_MAX 128
static char g_KeepExtra[KEEP_MAX][48];
static int  g_KeepCount = 0;

static void LoadWipeKeep() {
    g_KeepCount = 0;
    FILE* f = OpenPluginFile("wipekeep.txt", "r");
    if (!f) {
        EL_LOG("[EdictBudget] WIPECLEAR: khong co wipekeep.txt - chi dung preserve list cua game");
        return;
    }
    // Buffer PHAI du rong, va phai xa phan du neu dong dai hon buffer.
    // line[64] lam dong chu thich dai bi cat doi; phan duoi khong
    // bat dau bang '#' nen lot vao danh sach nhu mot ten lop rac.
    // File 15 dong ma nap ra 31 muc.
    char line[256];
    while (fgets(line, sizeof(line), f) && g_KeepCount < KEEP_MAX) {
        size_t L = strlen(line);
        if (L > 0 && line[L - 1] != '\n') {          // dong bi cat -> xa het dong
            int c; while ((c = fgetc(f)) != EOF && c != '\n') {}
        }
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\r' || *p == '\n' || !*p) continue;
        char* e = p + strlen(p) - 1;
        while (e >= p && (*e == '\r' || *e == '\n' || *e == ' ' || *e == '\t')) *e-- = 0;
        if (!*p) continue;
        // Chi nhan ky tu hop le cho classname - chan not moi thu rac con sot
        bool ok = true;
        for (const char* q = p; *q; q++) {
            if (!((*q >= 'a' && *q <= 'z') || (*q >= 'A' && *q <= 'Z') ||
                  (*q >= '0' && *q <= '9') || *q == '_')) { ok = false; break; }
        }
        if (!ok) {
            EL_LOG("[EdictBudget] WIPECLEAR: wipekeep.txt bo qua dong khong hop le: '%s'", p);
            continue;
        }
        strncpy(g_KeepExtra[g_KeepCount], p, sizeof(g_KeepExtra[0]) - 1);
        g_KeepExtra[g_KeepCount][sizeof(g_KeepExtra[0]) - 1] = 0;
        g_KeepCount++;
    }
    fclose(f);

    for (int i = 0; i < g_KeepCount; i++)
        EL_LOG("[EdictBudget] WIPECLEAR:   giu them [%d] '%s'", i, g_KeepExtra[i]);
    EL_LOG("[EdictBudget] WIPECLEAR: wipekeep.txt - giu them %d lop", g_KeepCount);
}

static bool InKeepExtra(const char* cls) {
    if (g_KeepCount <= 0 || !cls || !*cls) return false;
    for (int i = 0; i < g_KeepCount; i++) {
        const char* k = g_KeepExtra[i];
        size_t n = strlen(k);
        if (n == 0) continue;
        if (k[n - 1] == '_') {                       // khop tien to ca ho
            if (strncmp(cls, k, n) == 0) return true;
        } else if (strcmp(cls, k) == 0) {            // khop chinh xac
            return true;
        }
    }
    return false;
}

static void __fastcall Hook_RestartRound(void* thisptr, void* edx) {
    if (!g_OrigRestartRound) return;

    // --- CONG CHAN (KHOI PHUC 07/08 sau khi do that te) ---
    //
    // Da tung bo cong nay, dua tren suy luan tu disassembly rang mission_lost
    // "chi ban khi thua finale" (ham 0x10268CA0 co push trigger_finale /
    // FinaleLost). SUY LUAN DO SAI - do that te tren c6m1_riverbank (KHONG phai
    // finale) cho thay mission_lost VAN BAN, luc t=63.47.
    // Lai dung cai bay muc 0-BAI-HOC: suy tu chuoi nam gan nhau.
    //
    // Hau qua khi khong co cong (log 07/08, wipeclear=2):
    //   RestartRound duoc goi ngay luc t=1.00 KHI MAP VUA NAP (vong choi dau
    //   tien, khong phai wipe). Ban va da xoa 1155 entity cua map ngay tai do,
    //   va slot trong SAU RestartRound van o 1462 (nen la 474) => map KHONG
    //   duoc dung lai. Pha map.
    //
    // => Chi don khi CO mission_lost dang cho. Co mot-lan, khong cua so gio.
    float now = gpGlobals ? gpGlobals->curtime : 0.0f;
    float ago = (g_LastLossTime >= 0.0f) ? (now - g_LastLossTime) : -1.0f;

    EL_LOG("[EdictBudget] WIPECLEAR: RestartRound goi luc t=%.2f | "
             "mission_lost gan nhat: %s (cach %.2fs, tong %d) | cong: %s | slot trong=%d",
             now, g_LastLossName[0] ? g_LastLossName : "(chua bao gio ban)",
             ago, g_LossSignals, g_LossPending ? "MO -> SE DON" : "DONG -> BO QUA",
             CountFreeEdicts());

    if (!g_LossPending) { g_OrigRestartRound(thisptr, edx); return; }
    g_LossPending = false;               // dung mot lan cho moi wipe

    // wipeclear=1: chi quan sat. Van log day du hai dau moc quanh RestartRound
    // de biet num_edicts cham 2048 truoc hay sau, nhung KHONG xoa gi.
    if (g_WipeClear < 2) {
        EL_LOG("[EdictBudget] WIPECLEAR[quan sat]: TRUOC RestartRound "
                 "slot trong=%d num_edicts=%d", CountFreeEdicts(),
                 g_num_edicts ? (int)*g_num_edicts : -1);
        g_OrigRestartRound(thisptr, edx);
        EL_LOG("[EdictBudget] WIPECLEAR[quan sat]: SAU RestartRound "
                 "slot trong=%d num_edicts=%d", CountFreeEdicts(),
                 g_num_edicts ? (int)*g_num_edicts : -1);
        return;
    }

    int before = CountFreeEdicts();
    int removed = 0, kept = 0, keptExtra = 0;
    bool did = false;

    // Tai nhap: CleanupDeleteList KHONG co bao ve tai nhap tren ban retail
    // (g_fInCleanupDelete chi duoc doc trong #ifdef DEBUG). Phai tu kiem.
    if (g_InCleanupDelete && *g_InCleanupDelete) {
        EL_LOG("[EdictBudget] WIPECLEAR: dang trong CleanupDeleteList -> bo qua luot nay");
    } else if (g_NextEnt && g_UtilRemove && g_EntList && g_CleanupDeleteList) {
        void* e = g_NextEnt(g_EntList, NULL, NULL);          // FirstEnt = NextEnt(NULL)
        while (e) {
            void* next = g_NextEnt(g_EntList, NULL, e);      // lay truoc khi go
            const char* cls = *(const char**)((uint8_t*)e + 0x74);   // m_iClassname
            if (InPreserveList(cls))      kept++;            // game von giu
            else if (InKeepExtra(cls))  { kept++; keptExtra++; }  // ta giu them
            else { g_UtilRemove(e); removed++; }
            e = next;
        }
        g_CleanupDeleteList(g_EntList, NULL);                // tra edict ngay
        if (engine) engine->AllowImmediateEdictReuse();      // bo thoi gian cho
        did = true;
    }

    int after = CountFreeEdicts();
    g_WipeClearRuns++;
    EL_LOG("[EdictBudget] WIPECLEAR #%d %s: slot trong %d -> %d (chenh %d) | "
             "go %d, giu %d (trong do %d do wipekeep.txt) | num_edicts=%d",
             g_WipeClearRuns, did ? "DA DON" : "BO QUA", before, after,
             (before >= 0 && after >= 0) ? (after - before) : -1,
             removed, kept, keptExtra, g_num_edicts ? (int)*g_num_edicts : -1);

    g_OrigRestartRound(thisptr, edx);

    EL_LOG("[EdictBudget] WIPECLEAR #%d: SAU RestartRound slot trong=%d num_edicts=%d",
             g_WipeClearRuns, CountFreeEdicts(), g_num_edicts ? (int)*g_num_edicts : -1);
}

// ==========================================================================
// MAPCLEAR - don entity TRUOC KHI ENGINE don, luc CHUYEN MAN
// ==========================================================================
//
// XXX KHONG PHAI HUONG 4096. Khong bigarray/snapshot/pinmax/pinglobals/markfree.
//    Khong va mot byte nao. Chi moc vtable, giong het wipeclear.
//
// !!  KHAC WIPECLEAR O DIEM SINH TU - DOC TRUOC KHI SUA:
//   wipeclear: cung map, xoa nham thi entity DUOC DUNG LAI tu lump
//              => giu TOI THIEU moi dung (wipekeep.txt de RONG)
//   mapclear : map khac, xoa nham la MAT VINH VIEN do nguoi choi
//              => giu TOI DA. Khong chac thi GIU.
//   => TUYET DOI khong bung tap giu cua hai ben cho nhau.
//
// CO CHE (da doc tren binary):
//   - Map moi bat dau bang bang edict MOI. Rac NGOAI vung chuyen tiep tu bien
//     mat => don no vo ich. Chi thu NAM TRONG danh sach mang sang moi dang don.
//   - CBaseEntity::ObjectCaps() 0x10056160 MAC DINH tra FCAP_ACROSS_TRANSITION
//     => gan nhu MOI THU trong vung deu duoc mang sang, ke ca xac/manh vo.
//   - Hai duong mang sang:
//       (a) do TREN TAY -> CTerrorGameRules slot 38 serialize thanh KeyValues
//           (weaponID, currentMagazine, extraAmmo...). KHONG ton edict.
//       (b) do ROI DUOI DAT -> trigger_transition chuan cua Source. TON EDICT.
//     => chi (b) la van de.
//   - the_hive_m4 vao map chi con 31 slot trong => mang sang ~32 la cham tran.
//
// DIEM MOC: CTerrorGameRules vtable slot 38 = 0x102B8140, hook POST.
//   In ra "Preparing player entities for changelevel". __thiscall, ret 4.
//   Nam tren MAP CU, SAU anh chup nguoi choi, TRUOC khi bo may save khoi dong.
//   Cung vtable wipeclear dang moc (slot 178 = RestartRound = 0x102E0650).
//
//   XXX DA BO slot 27 (BuildAdjacentMapList): no chay 3 cho, 2 cho o MAP MOI
//      (CSaveRestore::LoadAdjacentEnts + duong nap .HL2). Hook mu = xoa entity
//      map moi ngay luc nap. Va tai slot 27 thi SaveGameState da goi PreSave
//      => bang entity da dung => xoa co nguy co con tro treo.
//
// XXX DUNG chay g_debug_transitions de "xem engine in ra": cvar do CHAN LUON
//    viec chuyen man, dat m_pfnTouch = 0 => cua phong an toan thanh cua chet.
//
// TAP GIU MAC DINH (an toan, doc them tu mapkeep.txt):
//   - toan bo preserve list CUA GAME (dung chung ham voi wipeclear) - bao thu
//   - player, weapon_ (trum ca weapon_*_spawn, gascan/propanetank/oxygentank)
//   - prop_fuel_barrel (trum ca _piece)
//   - ha tang chuyen man: info_landmark, trigger/info_changelevel, trigger_transition
//     (xoa may cai nay la hong CHINH viec chuyen man)
//
// CONG TAC: mapclear = 0 tat | 1 CHI QUAN SAT (dem+ghi log, khong xoa) | 2 don that
// (g_MapClear khai bao o dau file, canh cac cong tac khac)
// ---------------------------------------------------------------------------

#define MAPKILL_MAX 96
static char  g_MapKill[MAPKILL_MAX][40];
static int   g_MapKillCount = 0;

typedef void (__fastcall *fnPrepChangelevel_t)(void*, void*, void*);
static fnPrepChangelevel_t g_OrigPrepChangelevel = NULL;
static void**              g_PrepSlot            = NULL;
static int                 g_MapClearRuns        = 0;

#define MAPHIST_MAX 64
static char g_MapHistName[MAPHIST_MAX][40];
static int  g_MapHistCount[MAPHIST_MAX];
static int  g_MapHistN = 0;

static void MapHistAdd(const char* cls) {
    if (!cls) return;
    for (int i = 0; i < g_MapHistN; i++)
        if (strcmp(g_MapHistName[i], cls) == 0) { g_MapHistCount[i]++; return; }
    if (g_MapHistN >= MAPHIST_MAX) return;
    strncpy(g_MapHistName[g_MapHistN], cls, sizeof(g_MapHistName[0])-1);
    g_MapHistName[g_MapHistN][sizeof(g_MapHistName[0])-1] = 0;
    g_MapHistCount[g_MapHistN] = 1;
    g_MapHistN++;
}

static void LoadMapKill() {
    g_MapKillCount = 0;
    // !!  MAC DINH RONG - CO Y, day la diem khac cot loi so voi ban 1.
    //    Khong co mapkill.txt = KHONG XOA GI CA.
    FILE* f = OpenPluginFile("mapkeep.txt", "r");
    if (!f) {
        EL_LOG("[EdictBudget] MAPCLEAR: khong co mapkeep.txt - chi dung danh sach giu ghi cung");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f) && g_MapKillCount < MAPKILL_MAX) {
        size_t L = strlen(line);
        if (L > 0 && line[L-1] != '\n') { int c; while ((c=fgetc(f)) != EOF && c != '\n') {} }
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\n' || !*p) continue;
        char* e = p + strlen(p) - 1;
        while (e >= p && (*e=='\n'||*e=='\n'||*e==' '||*e=='\t')) *e-- = 0;
        if (!*p) continue;
        bool ok = true;
        for (const char* q = p; *q; q++)
            if (!((*q>='a'&&*q<='z')||(*q>='A'&&*q<='Z')||(*q>='0'&&*q<='9')||*q=='_')) { ok=false; break; }
        if (!ok) continue;
        strncpy(g_MapKill[g_MapKillCount], p, sizeof(g_MapKill[0])-1);
        g_MapKill[g_MapKillCount][sizeof(g_MapKill[0])-1] = 0;
        g_MapKillCount++;
    }
    fclose(f);
    EL_LOG("[EdictBudget] MAPCLEAR: mapkeep.txt - giu them %d lop", g_MapKillCount);
    for (int i = 0; i < g_MapKillCount; i++)
        EL_LOG("[EdictBudget] MAPCLEAR:   [%d] '%s'", i, g_MapKill[i]);
}

// Hoi chinh engine: entity nay CO duoc mang sang map moi khong?
// ObjectCaps() la ham ao slot 40 (+0xA0). Bit 0x2 = FCAP_ACROSS_TRANSITION.
// Ca InTransitionVolume (0x101FEFB0) lan ComputeEntitySaveFlags (0x101F8D80)
// deu goi dung cho nay => hoi thang, khoi phai truyen ten vung.
//
// *** DAY LA KHAC BIET COT LOI SO VOI BAN 1 (ban 1 lam CHET server):
//    ban 1 quet CA 1659 entity roi xoa 1497 -> dung ca thu engine dang can.
//    ban nay CHI dung toi entity that su se di theo -> pham vi nho hon han.
#define FCAP_ACROSS_TRANSITION 0x00000002
static bool WillCarryOver(void* ent) {
    if (!ent) return false;
    void** vt = *(void***)ent;
    if (!vt) return false;
    typedef int (__fastcall *fnObjectCaps_t)(void*, void*);
    fnObjectCaps_t caps = (fnObjectCaps_t)vt[40];      // +0xA0
    if (!caps) return false;
    return (caps(ent, NULL) & FCAP_ACROSS_TRANSITION) != 0;
}

// Danh sach KHONG DUOC DON. Ngoai ra don het - phan con lai engine tu lo.
// Ghi cung trong ma nguon phan toi thieu, doc them tu mapkeep.txt.
static bool InMapKeep(const char* cls) {
    if (!cls) return true;                        // khong ro ten => GIU
    static const char* k[] = {
        "weapon_",              // vu khi, vat pham, bom, weapon_*_spawn,
                                // weapon_gascan / propanetank / oxygentank
        "prop_fuel_barrel",     // thung xang / binh ga
        "prop_fuel_barrel_piece",
        "player",
        // ha tang chuyen man - xoa la hong CHINH viec chuyen man
        "info_landmark", "trigger_changelevel", "info_changelevel", "trigger_transition",
        NULL };
    for (int i = 0; k[i]; i++) {
        size_t len = strlen(k[i]);
        if (k[i][len-1] == '_') { if (strncmp(cls, k[i], len) == 0) return true; }
        else                    { if (strcmp(cls, k[i]) == 0)      return true; }
    }
    // them tu mapkeep.txt (dung chung bang voi g_MapKill, doi ten y nghia)
    for (int i = 0; i < g_MapKillCount; i++) {
        size_t len = strlen(g_MapKill[i]);
        if (len == 0) continue;
        if (g_MapKill[i][len-1] == '_') { if (strncmp(cls, g_MapKill[i], len) == 0) return true; }
        else                            { if (strcmp(cls, g_MapKill[i]) == 0)      return true; }
    }
    return false;
}

// Danh sach XOA THANG - thang ca dieu kien "se mang sang". Doc tu mapkill.txt.
// Dung cho RAC ma engine van dinh mang sang: xac zombie, xac nguoi, hat, lua.
// Rac an toan de don: xac zombie, hat, lua.
#define MAPFKILL_MAX 64
static char g_MapFKill[MAPFKILL_MAX][40];
static int  g_MapFKillCount = 0;

static void LoadMapForceKill() {
    g_MapFKillCount = 0;
    FILE* f = OpenPluginFile("mapkill.txt", "r");
    if (!f) {
        EL_LOG("[EdictBudget] MAPCLEAR: khong co mapkill.txt - khong xoa thang lop nao");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f) && g_MapFKillCount < MAPFKILL_MAX) {
        size_t L = strlen(line);
        if (L > 0 && line[L-1] != '\n') { int c; while ((c=fgetc(f)) != EOF && c != '\n') {} }
        char* q = line;
        while (*q == ' ' || *q == '\t') q++;
        if (*q == '#' || *q == '\n' || *q == '\n' || !*q) continue;
        char* e2 = q + strlen(q) - 1;
        while (e2 >= q && (*e2=='\n'||*e2=='\n'||*e2==' '||*e2=='\t')) *e2-- = 0;
        if (!*q) continue;
        bool ok = true;
        for (const char* z = q; *z; z++)
            if (!((*z>='a'&&*z<='z')||(*z>='A'&&*z<='Z')||(*z>='0'&&*z<='9')||*z=='_')) { ok=false; break; }
        if (!ok) continue;
        strncpy(g_MapFKill[g_MapFKillCount], q, sizeof(g_MapFKill[0])-1);
        g_MapFKill[g_MapFKillCount][sizeof(g_MapFKill[0])-1] = 0;
        g_MapFKillCount++;
    }
    fclose(f);
    EL_LOG("[EdictBudget] MAPCLEAR: mapkill.txt - %d lop XOA THANG (thang ca mang sang)",
             g_MapFKillCount);
    for (int i = 0; i < g_MapFKillCount; i++)
        EL_LOG("[EdictBudget] MAPCLEAR:   xoa thang [%d] '%s'", i, g_MapFKill[i]);
}

static bool InMapForceKill(const char* cls) {
    if (!cls) return false;
    for (int i = 0; i < g_MapFKillCount; i++) {
        size_t len = strlen(g_MapFKill[i]);
        if (len == 0) continue;
        if (g_MapFKill[i][len-1] == '_') { if (strncmp(cls, g_MapFKill[i], len) == 0) return true; }
        else                            { if (strcmp(cls, g_MapFKill[i]) == 0)      return true; }
    }
    return false;
}

static void __fastcall Hook_PrepChangelevel(void* thisptr, void* edx, void* a1) {
    // POST: de ban goc chup anh trang bi nguoi choi TRUOC da.
    if (g_OrigPrepChangelevel) g_OrigPrepChangelevel(thisptr, edx, a1);

    g_MapClearRuns++;
    if (g_MapClear < 1) return;

    if (!g_NextEnt || !g_UtilRemove || !g_EntList || !g_CleanupDeleteList) {
        EL_LOG("[EdictBudget] MAPCLEAR #%d: thieu cong cu go entity - BO QUA",
                 g_MapClearRuns);
        return;
    }

    int freeBefore = CountFreeEdicts();
    int total = 0, carry = 0, kept = 0, removed = 0, removedForce = 0, wouldRemove = 0, capped = 0;
    int transient = 0;      // khong mang sang -> bo qua, chet theo map
    g_MapHistN = 0;

    void* e = g_NextEnt(g_EntList, NULL, NULL);
    while (e) {
        void* next = g_NextEnt(g_EntList, NULL, e);
        const char* cls = *(const char**)((uint8_t*)e + 0x74);   // m_iClassname
        total++;

        bool willCarry = WillCarryOver(e);
        if (willCarry) carry++;

        // 1. DANH SACH KHONG DUOC DON:
        //      - vat pham cua nguoi choi   -> weapon_ , prop_fuel_barrel*
        //      - diem chuyen map           -> info_landmark, trigger/info_changelevel,
        //                                     trigger_transition
        //      - cua an toan               -> prop_door_rotating_checkpoint
        //      - nguoi choi                -> player
        //    + toan bo preserve list CUA GAME (worldspawn, terror_gamerules,
        //      soundent, scene_manager... xoa may cai nay la chet ngay)
        if (InPreserveList(cls) || InMapKeep(cls)) { kept++; e = next; continue; }

        // 2. g_MapClearCarryOnly: THI NGHIEM DA THAT BAI, mac dinh 0 nen nhanh nay
        //    khong chay. Xem khoi giai thich dai o cho khai bao g_MapClearCarryOnly.
        //    Bat len = xoa dung nhung cai engine da ghi vao danh sach chuyen man = sap.
        if (g_MapClearCarryOnly && !willCarry) { transient++; e = next; continue; }

        // 3. XOA.
        //
        // !!  TRAN SO LUONG (g_MapClearMax): luoi an toan, 0 = khong gioi han.
        //    Luu y tran nay dem theo SO LUOT XOA, khong theo so cai mang sang - ma
        //    cai mang sang moi la thu giet server. Tran cang cao thi cang vo phai
        //    nhieu cai mang sang. Do la ly do cap 100 song ma >1300 chet.
        if (g_MapClear >= 2) {
            if (g_MapClearMax > 0 && removed >= g_MapClearMax) { capped++; e = next; continue; }
            g_UtilRemove(e); removed++;
            if (willCarry) removedForce++;      // dem rieng: xoa ca thu se mang sang
        } else {
            wouldRemove++; MapHistAdd(cls);
        }
        e = next;
    }

    if (g_MapClear >= 2) {
        if (removed > 0) {
            g_CleanupDeleteList(g_EntList, NULL);
            if (engine) engine->AllowImmediateEdictReuse();
        }
        int freeAfter = CountFreeEdicts();
        EL_LOG("[EdictBudget] MAPCLEAR #%d (che do %d, chi-mang-sang=%d) DA DON: tong %d "
                 "| mang sang %d | go %d (mang sang %d), giu %d, bo qua vi khong mang "
                 "sang %d, cham tran %d | slot trong %d -> %d | num_edicts=%d",
                 g_MapClearRuns, g_MapClear, g_MapClearCarryOnly, total, carry,
                 removed, removedForce, kept, transient, capped, freeBefore, freeAfter,
                 g_num_edicts ? (int)*g_num_edicts : -1);
        return;
    }

    EL_LOG("[EdictBudget] MAPCLEAR #%d QUAN SAT (chi-mang-sang=%d): tong %d "
             "| **SE MANG SANG %d** | se go %d, giu %d, bo qua vi khong mang sang %d "
             "| slot trong=%d num_edicts=%d",
             g_MapClearRuns, g_MapClearCarryOnly, total, carry, wouldRemove, kept,
             transient, freeBefore, g_num_edicts ? (int)*g_num_edicts : -1);
    // Histogram nay duoc nap tu nhanh wouldRemove, tuc la DUNG nhung lop sap bi go.
    // Voi g_MapClearCarryOnly=1 thi day chinh la cac lop mang sang; voi =0 thi no la
    // TAT CA cac lop bi go, khong rieng gi mang sang - dung doc nham nhu lan truoc.
    EL_LOG("[EdictBudget] MAPCLEAR: cac lop SE BI GO%s:",
             g_MapClearCarryOnly ? " (deu la lop MANG SANG)" : " (ca mang sang lan khong)");
    for (int shown = 0; shown < 25; shown++) {
        int best = -1;
        for (int i = 0; i < g_MapHistN; i++)
            if (g_MapHistCount[i] > 0 && (best < 0 || g_MapHistCount[i] > g_MapHistCount[best])) best = i;
        if (best < 0 || g_MapHistCount[best] <= 0) break;
        EL_LOG("[EdictBudget] MAPCLEAR:   %5d  %s", g_MapHistCount[best], g_MapHistName[best]);
        g_MapHistCount[best] = 0;
    }
}

static bool InstallMapClear() {
    if (g_OrigPrepChangelevel) return true;       // da cai
    if (g_MapClear <= 0) return false;

    HMODULE h = GetModuleHandle("server.dll");
    if (!h) return false;
    MODULEINFO mi;
    GetModuleInformation(GetCurrentProcess(), h, &mi, sizeof(mi));
    uint8_t* b = (uint8_t*)mi.lpBaseOfDll;

    void*** grPtr = (void***)(b + 0x7F7F6C);      // g_pGameRules
    void** gr = *grPtr;
    if (!gr) { EL_LOG("[EdictBudget] MAPCLEAR: g_pGameRules con NULL, hoan lai"); return false; }
    void** vt = *(void***)gr;

    // Cong cu go: nap neu wipeclear chua nap (mapclear chay doc lap duoc).
    if (!g_CleanupDeleteList) g_CleanupDeleteList = (fnCleanupDeleteList_t)(b + 0x0B5D10);
    if (!g_NextEnt)           g_NextEnt           = (fnNextEnt_t)          (b + 0x0B4270);
    if (!g_UtilRemove)        g_UtilRemove        = (fnUtilRemove_t)       (b + 0x2071E0);
    if (!g_EntList)           g_EntList           = (void*)                (b + 0x7E0760);
    if (!g_PreserveList)      g_PreserveList      = (const char**)         (b + 0x7ACE40);

    // Cong an toan 1: prologue cua 0x102B8140 phai dung.
    //
    // !!  BAI HOC 09/08: prologue nay CO DIA CHI TUYET DOI, KHONG duoc so ca 16 byte.
    //   55 8B EC 56 8B 35 | E0 7A 89 10 | 8B 06 8B 50 68 8B
    //                       ^^^^^^^^^^^ mov esi,[0x10897AE0]
    //   Bon byte do bi TRINH NAP GHI LAI khi server.dll nap o base khac
    //   => so nguyen 16 byte thi KHONG BAO GIO khop, cong an toan chan oan.
    //   (wipeclear khong dinh vi prologue cua no khong co dia chi tuyet doi.)
    // => Dung mat na: '?' = byte bi relocate, bo qua.
    static const uint8_t kSig[]  = {
        0x55,0x8B,0xEC,0x56,0x8B,0x35, 0,0,0,0, 0x8B,0x06,0x8B,0x50,0x68,0x8B };
    static const char    kMask[] = "xxxxxx????xxxxxx";
    uint8_t* fn = b + 0x2B8140;
    bool sigOk = true;
    for (size_t i = 0; i < sizeof(kSig); i++)
        if (kMask[i] == 'x' && fn[i] != kSig[i]) { sigOk = false; break; }
    if (!sigOk) {
        EL_LOG("[EdictBudget] MAPCLEAR: prologue slot38 KHONG khop - "
                 "server.dll khac ban? BO QUA, khong dong gi.");
        return false;
    }
    // Cong an toan 2: slot 38 phai dang tro dung ham do.
    void** slot = &vt[38];
    if (*slot != (void*)fn) {
        EL_LOG("[EdictBudget] MAPCLEAR: slot38=%p != %p - BO QUA.", *slot, (void*)fn);
        return false;
    }

    DWORD old;
    if (!VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &old)) return false;
    g_OrigPrepChangelevel = (fnPrepChangelevel_t)*slot;
    *slot = (void*)&Hook_PrepChangelevel;
    VirtualProtect(slot, sizeof(void*), old, &old);
    g_PrepSlot = slot;

    LoadMapKill();
    LoadMapForceKill();
    EL_LOG("[EdictBudget] MAPCLEAR: da moc vtable slot 38 (0x102B8140 @ %p), "
             "mapclear=%d (%s)", (void*)fn, g_MapClear,
             g_MapClear >= 2 ? "DON THAT" : "CHI QUAN SAT");
    return true;
}

static void RemoveMapClear() {
    if (g_PrepSlot && g_OrigPrepChangelevel) {
        DWORD old;
        if (VirtualProtect(g_PrepSlot, sizeof(void*), PAGE_READWRITE, &old)) {
            *g_PrepSlot = (void*)g_OrigPrepChangelevel;
            VirtualProtect(g_PrepSlot, sizeof(void*), old, &old);
        }
    }
    g_PrepSlot = NULL;
    g_OrigPrepChangelevel = NULL;
}


// ==========================================================================
// NOEDICT - khien mot so lop KHONG BAO GIO duoc cap edict
// ==========================================================================
//
// XXX DAY KHONG PHAI HUONG 4096. Khong dung bigarray/snapshot/pinmax/pinglobals/
//    markfree. Khong va mot byte nao cua engine.dll. Neu ai do sua ham nay ma
//    thay minh can bat mot trong nam cong tac do => DA DI SAI DUONG, dung lai.
//
// CO CHE (da xac minh tren binary):
//   CBaseEntity::PostConstructor @ 0x10055620  (RVA 0x55620) la noi QUYET DINH:
//       mov  eax, [esi+0x138]      ; m_iEFlags
//       shr  edx, 9
//       test dl, 1                 ; bit 9 = EFL_SERVER_ONLY
//       je   <nhanh CAP EDICT>     ; = 0 -> AddNetworkableEntity, dai 0-2047
//       mov  ecx, gEntList
//       call AddNonNetworkableEntity   ; = 1 -> dai 2049-4095, KHONG EDICT
//   Ta chi can bat bit 9 TRUOC khi ham goc chay.
//
// DIEM MOC: PostConstructor la HAM AO, o vtable slot 29 (+0x74). Moi lop co
// vtable RIENG, nen thay slot 29 cua rieng vtable lop muc tieu => chi tac dong
// dung lop do. Khong detour byte, khong dung toi lop khac.
//
// Factory Create cua moi lop co dang:
//     push <sizeof>              ; operator new
//     call operator new
//     push 0                     ; <- bServerOnly = FALSE (co Valve noi toi)
//     call <ctor>
//     mov  dword ptr [esi], <VTABLE>   ; <- ta tim con so nay
//     ...
//     call [vtable+0x74]         ; PostConstructor
//
// XXX CAM DUA VAO DANH SACH:
//   - lop SOLID hoac CO DI CHUYEN: IVEngineServer::SolidMoved / TriggerMoved
//     deu nhan edict_t*. Khong edict => khong cap nhat phan vung khong gian.
//   - moi lop trigger_*  (cung ly do)
//   - lop co ServerClass RIENG (co DT_ rieng) - client can dung lai chung.
// AN TOAN da kiem cho: infodecal (StaticDecal dung chi so BE MAT, khong phai
// chi so cua chinh no) va ho light (LightStyle khong kem chi so entity nao).
// --------------------------------------------------------------------------
typedef void (__fastcall *PostCtor_t)(void*, void*, const char*);
static PostCtor_t g_OrigPostCtor = NULL;

#define NOEDICT_MAX 32
static char  g_NoEdictList[NOEDICT_MAX][40];
static int   g_NoEdictCount = 0;
static void* g_PatchedVt[NOEDICT_MAX];      // vtable da sua, de go khi unload
static int   g_PatchedVtCount = 0;

static void LoadNoEdictList() {
    g_NoEdictCount = 0;
    // !!  MAC DINH RONG - co y. Mac dinh cung tay cua nonetkill
    //    ({infodecal,light,light_spot}) lam sai anh sang ch04_pripyat03.
    //    Khong co noedict.txt = khong dong vao lop nao.
    FILE* f = OpenPluginFile("noedict.txt", "r");
    if (!f) {
        EL_LOG("[EdictBudget] NOEDICT: khong co noedict.txt - khong lam gi");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f) && g_NoEdictCount < NOEDICT_MAX) {
        size_t L = strlen(line);
        if (L > 0 && line[L-1] != '\n') { int c; while ((c=fgetc(f)) != EOF && c != '\n') {} }
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\r' || *p == '\n' || !*p) continue;
        char* e = p + strlen(p) - 1;
        while (e >= p && (*e=='\r'||*e=='\n'||*e==' '||*e=='\t')) *e-- = 0;
        if (!*p) continue;
        bool ok = true;
        for (const char* q = p; *q; q++)
            if (!((*q>='a'&&*q<='z')||(*q>='A'&&*q<='Z')||(*q>='0'&&*q<='9')||*q=='_')) { ok=false; break; }
        if (!ok) continue;
        strncpy(g_NoEdictList[g_NoEdictCount], p, sizeof(g_NoEdictList[0])-1);
        g_NoEdictList[g_NoEdictCount][sizeof(g_NoEdictList[0])-1] = 0;
        g_NoEdictCount++;
    }
    fclose(f);
    EL_LOG("[EdictBudget] NOEDICT: %d lop trong danh sach", g_NoEdictCount);
    for (int i = 0; i < g_NoEdictCount; i++)
        EL_LOG("[EdictBudget] NOEDICT:   [%d] '%s'", i, g_NoEdictList[i]);
}

static int g_NoEdictHits = 0;

static void __fastcall Hook_PostCtor(void* thisptr, void* edx, const char* cls) {
    if (thisptr) {
        // bit 9 = EFL_SERVER_ONLY. Bat TRUOC khi ham goc doc no.
        *(unsigned int*)((uint8_t*)thisptr + 0x138) |= (1u << 9);
        g_NoEdictHits++;
    }
    if (g_OrigPostCtor) g_OrigPostCtor(thisptr, edx, cls);
}

// Quet mot ham tim  mov dword ptr [reg], imm32  (= luu con tro vtable).
// Tra NULL neu khong thay, HOAC thay nhieu hon mot (khong chac chan -> bo).
static void** ScanForVtableStore(uint8_t* base, uint8_t* fn) {
    void** found = NULL;
    for (int i = 0; i < 80; i++) {
        if (fn[i] != 0xC7) continue;
        uint8_t modrm = fn[i+1];
        if (modrm > 0x07 || modrm == 0x04 || modrm == 0x05) continue;   // bo SIB/disp32
        unsigned int imm = *(unsigned int*)(fn + i + 2);
        if (imm < (unsigned int)(uintptr_t)base) continue;
        if (found) return NULL;
        found = (void**)(uintptr_t)imm;
    }
    return found;
}

// Tim vtable cua mot lop qua factory cua no. Tra NULL neu khong chac chan.
static void** ResolveClassVtable(uint8_t* base, const char* cls) {
    typedef void* (__cdecl *GetDict_t)(void);
    typedef void* (__thiscall *FindFactory_t)(void*, const char*);

    GetDict_t getDict = (GetDict_t)(base + 0x20CA70);
    void* dict = getDict();
    if (!dict) return NULL;

    void** dvt = *(void***)dict;
    if (!dvt) return NULL;
    FindFactory_t findFac = (FindFactory_t)dvt[3];          // slot 3 = FindFactory
    void* fac = findFac(dict, cls);
    if (!fac) return NULL;

    void** fvt = *(void***)fac;
    if (!fvt) return NULL;
    uint8_t* create = (uint8_t*)fvt[0];                     // slot 0 = Create
    if (!create) return NULL;

    // Quet ~80 byte dau tim  C7 /r imm32  voi modrm dang [reg] khong displacement
    void** found = ScanForVtableStore(base, create);
    if (found) return found;

    // Khong thay: mot so lop co Create chi la THUNK uy quyen het cho ham con.
    // Vi du path_track (0x1012D680):
    //     mov eax,[ebp+8] ; push eax ; push 0 ; call <helper> ; add eax,0x1c ; ret 4
    // Helper do la ban dac hoa cho rieng lop nay, nen quet trong do la DUNG.
    // Chi lan theo MOT CAP, va chi khi Create ngan (<0x30 byte den ret).
    for (int i = 0; i < 0x30; i++) {
        if (create[i] == 0xE8) {                              // call rel32
            int32_t rel = *(int32_t*)(create + i + 1);
            uint8_t* target = create + i + 5 + rel;
            if (target < base) continue;
            return ScanForVtableStore(base, target);
        }
        if (create[i] == 0xC3 || create[i] == 0xC2) break;     // gap ret truoc -> thoi
    }
    return NULL;
}

static bool InstallNoEdict() {
    if (g_NoEdictCount <= 0) return false;

    HMODULE h = GetModuleHandle("server.dll");
    if (!h) return false;
    MODULEINFO mi;
    GetModuleInformation(GetCurrentProcess(), h, &mi, sizeof(mi));
    uint8_t* base = (uint8_t*)mi.lpBaseOfDll;

    void* pPostCtor = (void*)(base + 0x55620);

    // Cong an toan 1: prologue PostConstructor phai dung.
    // 0x55620: 55 8B EC 8B 45 08 56 8B F1 85 C0
    static const uint8_t kSig[] = { 0x55,0x8B,0xEC,0x8B,0x45,0x08,0x56,0x8B,0xF1,0x85,0xC0 };
    if (memcmp(pPostCtor, kSig, sizeof(kSig)) != 0) {
        EL_LOG("[EdictBudget] NOEDICT: prologue PostConstructor KHONG khop "
                 "- server.dll khac ban? Bo qua, khong dong gi.");
        return false;
    }
    g_OrigPostCtor = (PostCtor_t)pPostCtor;

    for (int i = 0; i < g_NoEdictCount; i++) {
        const char* cls = g_NoEdictList[i];
        void** vt = ResolveClassVtable(base, cls);
        if (!vt) {
            EL_LOG("[EdictBudget] NOEDICT: '%s' - khong tim duoc vtable, BO QUA", cls);
            continue;
        }
        // Cong an toan 2: slot 29 phai dang tro dung PostConstructor.
        // Neu da bi sua (vd lop nay dung chung vtable voi lop truoc) -> bo qua.
        if (vt[29] != pPostCtor) {
            EL_LOG("[EdictBudget] NOEDICT: '%s' vtable=%p slot29=%p != PostConstructor "
                     "- da sua roi hoac lop ghi de, BO QUA", cls, (void*)vt, vt[29]);
            continue;
        }
        // Cong an toan 3: DIEU KIEN 1 - lop KHONG duoc co SendTable rieng.
        //
        // Day la dieu kien loc manh nhat trong 6 dieu kien, va la dieu kien DUY NHAT
        // may kiem duoc thay nguoi. Truoc day no nam trong ghi chu cua noedict.txt,
        // ai khong doc thi them bua vao va hong khi chay.
        //
        // GetServerClass() = vtable slot 9. Than ham la `mov eax, imm32 ; ret`:
        //     B8 <imm32> C3
        //   imm32 == 0x107D78A8  -> ServerClass CBaseEntity / DT_BaseEntity -> CHO PHEP
        //   imm32 != 0x107D78A8  -> lop co SendTable rieng                  -> TU CHOI
        //
        // Da quet 24 lop bang cach nay, hieu chuan voi 8 gia tri biet chac, khop 100%.
        // Bon lop dang chay (infodecal/light/light_spot/path_track) deu tra 0x107D78A8.
        // Vi du bi tu choi: spotlight_end (CSpotlightEnd), beam (CBeam),
        //                   env_sprite (CSprite), light_dynamic (DT_DynamicLight),
        //                   ca ho trigger_* (CBaseTrigger).
        {
            const uint8_t* gsc = (const uint8_t*)vt[9];
            uint32_t sc = 0;
            bool readable = false;
            if (gsc && gsc[0] == 0xB8 && gsc[5] == 0xC3) {
                sc = *(const uint32_t*)(gsc + 1);
                readable = true;
            }
            if (!readable) {
                // Than ham khac khuon mau => khong ket luan duoc. Khong doan, tu choi.
                EL_LOG("[EdictBudget] NOEDICT: '%s' vtable=%p GetServerClass=%p khong "
                         "dang 'mov eax,imm32; ret' - KHONG XAC DINH DUOC, BO QUA",
                         cls, (void*)vt, (void*)gsc);
                continue;
            }
            uint32_t want = (uint32_t)(uintptr_t)(base + DT_BASEENTITY_RVA);
            if (sc != want) {
                EL_LOG("[EdictBudget] NOEDICT: '%s' CO SENDTABLE RIENG "
                         "(ServerClass=0x%08X, can 0x%08X) - TRUOT DIEU KIEN 1, TU CHOI. "
                         "Go lop nay se lam client khong nhan duoc no.",
                         cls, sc, want);
                continue;
            }
        }
        void* thunk = (void*)&Hook_PostCtor;
        WriteProtected(&vt[29], &thunk, sizeof(void*));
        if (g_PatchedVtCount < NOEDICT_MAX) g_PatchedVt[g_PatchedVtCount++] = vt;
        EL_LOG("[EdictBudget] NOEDICT: '%s' vtable=%p slot29 -> thunk (OK)", cls, (void*)vt);
    }

    EL_LOG("[EdictBudget] NOEDICT: da sua %d vtable / %d lop yeu cau",
             g_PatchedVtCount, g_NoEdictCount);
    return g_PatchedVtCount > 0;
}

// =====================================================================
//  SWAP: doi mot lop entity thanh lop khac RE HON ngay luc tao
// =====================================================================
//
// Bai toan: `point_spotlight` khi spawn TU TAO THEM `spotlight_end` + `beam`
//   => 3 edict cho moi dong trong lump BSP.
//   `beam_spotlight` lam viec tuong tu nhung VE HOAN TOAN PHIA CLIENT,
//   khong sinh entity con => 1 edict.
//   Chinh tac gia the_hive da dung ca hai lop trong cung chien dich
//   (m1 co 2 beam_spotlight, m5 co 21, m4 co 312 point_spotlight).
//   Doi duoc: m4 937 -> 313, m3 240 -> 80, m5 41 -> 33. Tong 792 edict.
//
// KHAC HAN noedict: day KHONG phai go mang. Client van nhan entity, van ve
//   tia sang. Chi la mot lop re hon. Nen khong dinh 6 dieu kien nao het.
//
// CHO MOC — vi sao cho nay sach:
//   CreateEntityByName (0x101196B0) khong tu tao gi, no goi qua
//     EntityFactoryDictionary()->vtable[1]:
//       101196E7 call 0x1020CA70 ; mov eax,[edx+4] ; call eax
//   Quet ca .text: 562 cho goi 0x1020CA70, trong do
//     558 dung slot 0 (InstallFactory), 1 dung slot 4 (GetCannonicalName),
//     va DUNG 3 cho dung slot 1 (Create): CreateEntityByName + 2 nhanh cua
//     bo phan tich lump BSP.
//   => Va MOT con tro vtable phu ca luc nap lump lan luc choi.
//   0x1020CA70 la dia chi plugin DA dung san trong ResolveClassVtable.
//   Dung SourceHook-style vtable swap, KHONG detour byte.
//
// ANH XA KEYVALUE (doc datamap cua ca hai lop):
//   SpotlightLength / SpotlightWidth / HDRColorScale  TRUNG TEN TUYET DOI
//   input LightOn / LightOff, output OnLightOn        trung ten
//   cung baseMap = CBaseEntity                        moi khoa ke thua giong het
//   MAT DUNG MOT KHOA: HaloScale - client.dll ghi cung halo = 60.0 tai 1006CC80.
//     => map nao dat HaloScale 10 (vd the_hive_m4) se thay halo TO GAP 6.
//     43/517 point_spotlight tren 50 map von da dat 60 = dung mac dinh, khong doi gi.
//
// spawnflags: bit 0 (bat san) va bit 1 (khong den dong) GIONG HET giua hai lop.
//   Quet 517 point_spotlight tren 45 map goc + 5 map hive: chi tung la 2 hoac 3,
//   chua cai nao dat bit 2/3/6 => rui ro bat nham xoay/nofog = 0.
//
// m_iClassname tu lump se ghi de lai thanh "point_spotlight". Da chung minh
//   server.dll khong co cho nao tra cuu chuoi do ngoai InstallFactory => vo hai,
//   va giu tuong thich nguoc cho plugin SourceMod dang loc theo ten lop.
//
// CHUA LAM (co y, de test dan):
//   Tu so datamap cua hai lop luc khoi dong de bao khoa nao bi mat.
//   GetDataDescMap() = vtable slot 11 (+0x2C), cung khuon `B8 imm32 C3` nhu slot 9.
//   datamap_t 24 byte {dataDesc, nFields, className, baseMap};
//   typedescription_t 60 byte, ten keyvalue o +0x10.
//   Chua viet vi day la doc con tro chua kiem chung tren ban nay - them sau,
//   khi co che doi lop da chay on.

#define SWAP_MAX 16
static char g_SwapFrom[SWAP_MAX][40];
static char g_SwapTo[SWAP_MAX][40];
static int  g_SwapSeen[SWAP_MAX];      // gap bao nhieu lan
static int  g_SwapHits[SWAP_MAX];      // doi that bao nhieu lan
static int  g_SwapCount = 0;
static int  g_SwapDone  = 0;

// Ten map cua lan nap hien tai, dat o Hook_LevelInit. In kem bao cao SWAP de
// khoi phai doan map nao qua con so.
static const char* g_SwapMapName = "?";
static void** g_DictVt = NULL;
typedef void* (__fastcall *DictCreate_t)(void*, void*, const char*);
static DictCreate_t g_OrigDictCreate = NULL;

static void* __fastcall Hook_DictCreate(void* thisptr, void* edx, const char* cls) {
    if (cls && g_Swap >= 1) {
        for (int i = 0; i < g_SwapCount; i++) {
            if (_stricmp(cls, g_SwapFrom[i]) != 0) continue;
            g_SwapSeen[i]++;
            // che do 1 = chi dem, khong doi. Dung de biet map co bao nhieu cai
            // truoc khi dam doi that.
            if (g_Swap >= 2 && (g_SwapMax <= 0 || g_SwapDone < g_SwapMax)) {
                g_SwapDone++;
                g_SwapHits[i]++;
                cls = g_SwapTo[i];
            }
            break;
        }
    }
    return g_OrigDictCreate(thisptr, edx, cls);
}

static void LoadSwapList() {
    g_SwapCount = 0;
    // Mac dinh RONG. Khong co swap.txt = khong doi lop nao.
    FILE* f = OpenPluginFile("swap.txt", "r");
    if (!f) { EL_LOG("[EdictBudget] SWAP: khong co swap.txt - khong doi lop nao"); return; }
    char line[256];
    while (fgets(line, sizeof(line), f) && g_SwapCount < SWAP_MAX) {
        size_t L = strlen(line);
        if (L > 0 && line[L-1] != '\n') { int c; while ((c=fgetc(f)) != EOF && c != '\n') {} }
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\r' || *p == '\n' || !*p) continue;
        // dinh dang:  tu_lop  ->  thanh_lop     (dau -> tuy chon)
        char a[40] = {0}, b[40] = {0};
        int n = sscanf(p, "%39s -> %39s", a, b);
        if (n != 2) { n = sscanf(p, "%39s %39s", a, b); }
        if (n != 2 || !a[0] || !b[0]) continue;
        strncpy(g_SwapFrom[g_SwapCount], a, 39);
        strncpy(g_SwapTo  [g_SwapCount], b, 39);
        g_SwapSeen[g_SwapCount] = 0;
        g_SwapHits[g_SwapCount] = 0;
        g_SwapCount++;
    }
    fclose(f);
    EL_LOG("[EdictBudget] SWAP: %d cap trong danh sach", g_SwapCount);
}

static bool InstallSwap() {
    if (g_Swap <= 0 || g_SwapCount <= 0) return false;

    HMODULE h = GetModuleHandle("server.dll");
    if (!h) return false;
    uint8_t* base = (uint8_t*)h;

    typedef void* (__cdecl *GetDict_t)(void);
    typedef void* (__thiscall *FindFactory_t)(void*, const char*);
    GetDict_t getDict = (GetDict_t)(base + 0x20CA70);
    void* dict = getDict();
    if (!dict) { EL_LOG("[EdictBudget] SWAP: khong lay duoc factory dictionary, BO QUA"); return false; }

    void** dvt = *(void***)dict;
    if (!dvt || !dvt[1] || !dvt[3]) {
        EL_LOG("[EdictBudget] SWAP: vtable dictionary khong hop le, BO QUA");
        return false;
    }
    FindFactory_t findFac = (FindFactory_t)dvt[3];

    // Cong an toan: CA HAI lop cua moi cap phai ton tai that trong dictionary.
    // Doi sang mot lop khong ton tai = CreateEntityByName tra NULL = mat entity.
    int valid = 0;
    for (int i = 0; i < g_SwapCount; i++) {
        void* fa = findFac(dict, g_SwapFrom[i]);
        void* fb = findFac(dict, g_SwapTo[i]);
        if (!fa || !fb) {
            EL_LOG("[EdictBudget] SWAP: bo cap '%s' -> '%s' (%s khong ton tai trong "
                     "factory dictionary)", g_SwapFrom[i], g_SwapTo[i],
                     !fa ? g_SwapFrom[i] : g_SwapTo[i]);
            g_SwapFrom[i][0] = 0;          // vo hieu cap nay
            continue;
        }
        EL_LOG("[EdictBudget] SWAP:   [%d] '%s' -> '%s' (ca hai lop deu ton tai)",
                 i, g_SwapFrom[i], g_SwapTo[i]);
        valid++;
    }
    if (valid == 0) { EL_LOG("[EdictBudget] SWAP: khong cap nao hop le, BO QUA"); return false; }

    g_DictVt = dvt;
    g_OrigDictCreate = (DictCreate_t)dvt[1];
    void* thunk = (void*)&Hook_DictCreate;
    WriteProtected(&dvt[1], &thunk, sizeof(void*));

    EL_LOG("[EdictBudget] SWAP: da moc dictionary slot 1 (Create) @%p, che do %d (%s), "
             "tran %d, %d cap hop le",
             (void*)g_OrigDictCreate, g_Swap,
             g_Swap >= 2 ? "DOI THAT" : "CHI QUAN SAT",
             g_SwapMax, valid);
    return true;
}

static void UninstallSwap() {
    if (!g_DictVt || !g_OrigDictCreate) return;
    void* orig = (void*)g_OrigDictCreate;
    WriteProtected(&g_DictVt[1], &orig, sizeof(void*));
    g_OrigDictCreate = NULL;
    g_DictVt = NULL;
}

// Bao cao roi DAT LAI VE 0. Ban 16:01 khong dat lai nen so cong don qua cac map,
// phai tu tru moi ra so tung map (0->80->392->404...). Kho doc va de nham.
static void SwapReport() {
    if (g_SwapCount <= 0 || g_Swap <= 0) return;
    for (int i = 0; i < g_SwapCount; i++) {
        if (!g_SwapFrom[i][0]) continue;
        if (g_SwapSeen[i] == 0 && g_SwapHits[i] == 0) continue;   // map nay khong co
        // KHONG in "tiet kiem = doi x 2". Con so do SAI.
        //   point_spotlight chi sinh 2 con khi spawnflags & 1. Cai nao khong co bit
        //   do thi von da la 1 edict, doi sang beam_spotlight khong tiet kiem gi.
        //   Do 14/08 tren the_hive_m5: 12 cai (8 co spawnflags=2, 4 co =3)
        //     truoc  8x1 + 4x3 = 20 edict | sau  12x1 = 12 | tiet kiem THAT = 8
        //   ma cong thuc doi-x2 lai bao 24.
        // Luc Create chua doc keyvalue nen khong biet spawnflags => chi in can tren.
        EL_LOG("[EdictBudget] SWAP [%s]: '%s' -> '%s' | gap %d | doi %d | "
                 "tiet kiem toi da %d edict (that su it hon neu co cai khong bat san)",
                 g_SwapMapName, g_SwapFrom[i], g_SwapTo[i],
                 g_SwapSeen[i], g_SwapHits[i], g_SwapHits[i] * 2);
    }
    if (g_SwapMax > 0 && g_SwapDone >= g_SwapMax)
        EL_LOG("[EdictBudget] SWAP: DA CHAM TRAN %d - phan con lai khong doi", g_SwapMax);
    // KHONG dat lai o day nua - da chuyen sang Hook_LevelInit, xem giai thich o do.
}

static void UninstallNoEdict() {
    if (!g_OrigPostCtor) return;
    void* orig = (void*)g_OrigPostCtor;
    for (int i = 0; i < g_PatchedVtCount; i++) {
        void** vt = (void**)g_PatchedVt[i];
        if (vt && vt[29] == (void*)&Hook_PostCtor)
            WriteProtected(&vt[29], &orig, sizeof(void*));
    }
    g_PatchedVtCount = 0;
    g_OrigPostCtor = NULL;
}


static bool InstallWipeClear() {
    if (g_OrigRestartRound) return true;              // da cai roi
    if (g_WipeClear <= 0) return false;               // 0 = no-op that su, khong dung vtable

    HMODULE h = GetModuleHandle("server.dll");
    if (!h) return false;
    MODULEINFO mi;
    GetModuleInformation(GetCurrentProcess(), h, &mi, sizeof(mi));
    uint8_t* b = (uint8_t*)mi.lpBaseOfDll;

    // Xac minh prologue RestartRound truoc khi dong vao vtable.
    // 0x2E0650: 55 8B EC 0F 57 C0 83 EC 08 53 33 DB 56 57 8B F9
    static const uint8_t kSig[] = {
        0x55,0x8B,0xEC,0x0F,0x57,0xC0,0x83,0xEC,0x08,0x53,0x33,0xDB,0x56,0x57,0x8B,0xF9 };
    uint8_t* fn = b + 0x2E0650;
    if (memcmp(fn, kSig, sizeof(kSig)) != 0) {
        EL_LOG("[EdictBudget] WIPECLEAR: prologue RestartRound KHONG khop "
                 "- server.dll khac ban? Bo qua, khong dong gi.");
        return false;
    }

    void*** grPtr = (void***)(b + 0x7F7F6C);          // g_pGameRules
    void** gr = *grPtr;
    if (!gr) { EL_LOG("[EdictBudget] WIPECLEAR: g_pGameRules con NULL, hoan lai"); return false; }

    void** vtbl = *(void***)gr;
    void** slot = &vtbl[178];                         // CTerrorGameRules::RestartRound
    if (*slot != (void*)fn) {
        EL_LOG("[EdictBudget] WIPECLEAR: vtable slot 178 = %p, khong phai RestartRound %p "
                 "- co plugin khac da moc? Bo qua.", *slot, fn);
        return false;
    }

    g_CleanupDeleteList = (fnCleanupDeleteList_t)(b + 0x0B5D10);
    g_NextEnt           = (fnNextEnt_t)          (b + 0x0B4270);
    g_UtilRemove        = (fnUtilRemove_t)       (b + 0x2071E0);
    g_EntList           = (void*)                (b + 0x7E0760);
    g_InCleanupDelete   = (bool*)                (b + 0x7E0730);
    g_PreserveList      = (const char**)         (b + 0x7ACE40);

    // Kiem preserve list doc dung khong: phan tu 0 phai la "ai_network",
    // phan tu 33 phai la "predicted_viewmodel".
    const char* p0  = g_PreserveList[0];
    const char* p33 = g_PreserveList[33];
    if (!p0 || !p33 || strcmp(p0, "ai_network") != 0 || strcmp(p33, "predicted_viewmodel") != 0) {
        EL_LOG("[EdictBudget] WIPECLEAR: preserve list sai ([0]=%s [33]=%s) - bo qua.",
                 p0 ? p0 : "(null)", p33 ? p33 : "(null)");
        g_PreserveList = NULL;
        return false;
    }

    DWORD old;
    if (!VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &old)) return false;
    g_OrigRestartRound = (fnRestartRound_t)*slot;
    *slot = (void*)&Hook_RestartRound;
    VirtualProtect(slot, sizeof(void*), old, &old);
    g_RestartSlot = slot;

    LoadWipeKeep();

    // Dang ky nghe tin hieu thua. mission_lost = "survivor team that bai"
    // (chu thich cua Valve trong modevents.res). round_end kem theo de doi chieu.
    if (gameevents && !g_LossHooked) {
        bool a = gameevents->AddListener(&g_LossListener, "mission_lost", true);
        bool b = gameevents->AddListener(&g_LossListener, "round_end",    true);
        g_LossHooked = (a || b);
        EL_LOG("[EdictBudget] WIPECLEAR: nghe su kien mission_lost=%d round_end=%d", a, b);
    }

    EL_LOG("[EdictBudget] WIPECLEAR: da moc vtable slot 178 (RestartRound @ %p), "
             "preserve list OK, wipeclear=%d, cong = co mot-lan tu mission_lost",
             fn, g_WipeClear);
    return true;
}

static void RemoveWipeClear() {
    if (gameevents && g_LossHooked) {
        gameevents->RemoveListener(&g_LossListener);
        g_LossHooked = false;
    }
    if (g_RestartSlot && g_OrigRestartRound) {
        DWORD old;
        if (VirtualProtect(g_RestartSlot, sizeof(void*), PAGE_READWRITE, &old)) {
            *g_RestartSlot = (void*)g_OrigRestartRound;
            VirtualProtect(g_RestartSlot, sizeof(void*), old, &old);
        }
    }
    g_RestartSlot = NULL;
    g_OrigRestartRound = NULL;
}

static bool InstallDetour() {
    HMODULE h = GetModuleHandle("server.dll");
    if (!h) return false;
    MODULEINFO mi;
    GetModuleInformation(GetCurrentProcess(), h, &mi, sizeof(mi));
    uint8_t* base = (uint8_t*)mi.lpBaseOfDll;
    size_t size = mi.SizeOfImage;

    const char* anchor = "CreateEntityByName( %s, %d ) - CreateEdict failed.";
    size_t alen = strlen(anchor);
    uint8_t* str = NULL;
    for (size_t i = 0; i + alen + 1 < size; i++) {
        if (memcmp(base + i, anchor, alen + 1) == 0) { str = base + i; break; }
    }
    if (!str) {
        EL_LOG("[EdictBudget] detour: anchor string not found.");
        return false;
    }

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
    PIMAGE_NT_HEADERS nt  = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
    PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt);
    uint8_t* text = NULL; size_t textLen = 0;
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        if (strncmp((char*)sec[i].Name, ".text", 5) == 0) {
            text = base + sec[i].VirtualAddress;
            textLen = sec[i].Misc.VirtualSize;
            break;
        }
    }
    if (!text) return false;

    uint32_t strVal = (uint32_t)str;
    uint8_t* fn = NULL;
    for (size_t i = 0; i + 5 < textLen; i++) {
        if (text[i] == 0x68 && *(uint32_t*)(text + i + 1) == strVal) {
            for (int back = 1; back < 200 && (size_t)back <= i; back++) {
                uint8_t* c = text + i - back;
                if (c[0] == 0x55 && c[1] == 0x8B && c[2] == 0xEC) { fn = c; break; }
            }
            if (fn) break;
        }
    }
    if (!fn) {
        EL_LOG("[EdictBudget] detour: prologue not located.");
        return false;
    }

    static const uint8_t expect[7] = { 0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x75, 0x0C };
    if (memcmp(fn, expect, sizeof(expect)) != 0) {
        EL_LOG("[EdictBudget] detour: prologue mismatch at %p - aborted (safe).", fn);
        return false;
    }

    g_DetourAt = fn;
    memcpy(g_DetourSaved, fn, sizeof(g_DetourSaved));

    g_Trampoline = (uint8_t*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_Trampoline) return false;
    memcpy(g_Trampoline, g_DetourSaved, sizeof(g_DetourSaved));
    g_Trampoline[7] = 0xE9;
    *(uint32_t*)(g_Trampoline + 8) = (uint32_t)(fn + 7) - (uint32_t)(g_Trampoline + 12);
    g_OrigCreateEntity = (CreateEntityByName_t)g_Trampoline;

    uint8_t patch[7];
    patch[0] = 0xE9;
    *(uint32_t*)(patch + 1) = (uint32_t)Detour_CreateEntityByName - (uint32_t)(fn + 5);
    patch[5] = 0x90;
    patch[6] = 0x90;
    WriteProtected(fn, patch, sizeof(patch));

    EL_LOG("[EdictBudget] detour installed at %p", fn);
    return true;
}

static void RemoveDetour() {
    if (!g_DetourAt || !g_OrigCreateEntity) return;
    WriteProtected(g_DetourAt, g_DetourSaved, sizeof(g_DetourSaved));
    if (g_Trampoline) { VirtualFree(g_Trampoline, 0, MEM_RELEASE); g_Trampoline = NULL; }
    g_OrigCreateEntity = NULL;
    g_DetourAt = NULL;
}

// ==========================================================================
// Hook CreateEdict - toan bo muc dich cua nhom cong tac 4096
// ==========================================================================
static edict_t* Hook_CreateEdict(int forceIndex) {
    // Ben goi da yeu cau MOT o cu the; tuyet doi khong doan lai y ho.
    if (forceIndex > 0) RETURN_META_VALUE(MRES_IGNORED, nullptr);

    // Moi thu khong nam trong danh sach cho phep deu de nguyen cho engine lo,
    // ma bo cap phat cua no bi chan boi max_edicts = 2048 - tuc hanh vi goc.
    if (!g_ExtReady || !MayLiveHigh(g_PendingClass)) {
        RETURN_META_VALUE(MRES_IGNORED, nullptr);
    }
    if (!gpGlobals || !gpGlobals->pEdicts) RETURN_META_VALUE(MRES_IGNORED, nullptr);

    // Cap phat DI XUONG tu dinh cua dai.
    //
    // CBaseEntityList cua server.dll co DUY NHAT mot m_EntPtrArray[4096], ma nua
    // tren duoc danh rieng cho entity KHONG CO EDICT: chi so 0..2047 do
    // AddNetworkableEntity(entindex) ghi, con 2048..4095 do
    // AddNonNetworkableEntity() cap ra tu m_freeNonNetworkableList.
    //
    // Ep mot chi so edict, vi du 3000, se lam server.dll ghi vao
    // m_EntPtrArray[3000] qua duong CO MANG - dung cai o ma be chua khong-co-mang
    // cua no co the giao cho MOT entity khac. The la hai entity dung chung mot o,
    // cai thu hai de len cai thu nhat, va mot lan tra cuu EHANDLE ve sau se qua
    // duoc phep kiem serial nhung tra ve con tro treo. Do chinh la vu sap quan sat
    // duoc: "call [vtable+0x324]" nhay vao vung du lieu chuoi.
    //
    // Danh sach trong duoc dung theo thu tu TANG DAN va lay ra tu dau, nen
    // server.dll tieu thu 2048 di LEN. Cap phat tu 4095 di XUONG giu hai ben tach
    // nhau cho toi khi tong muc dung cua ca hai vuot 2048 o - ma server.dll thi
    // xua nay chi can vai tram.
    for (int n = 0; n < EXT_LIMIT - NET_LIMIT; n++) {
        int i = g_Cursor - n;
        if (i < NET_LIMIT) i += (EXT_LIMIT - NET_LIMIT);
        edict_t* e = gpGlobals->pEdicts + i;
        if (!e->IsFree()) continue;

        // O day KHONG con nang gioi han nao nua. Nhanh ep-chi-so cua ED_Alloc gio
        // so chi so voi mot hang so nuong thang vao ma (xem PatchForcedIndexCheck),
        // nen max_edicts giu nguyen 2048 suot doi may chu. Nang no du chi trong
        // choc lat cung tung du de num_edicts bo qua 2048 va tran VINH VIEN danh
        // sach snapshot 2048 muc cua engine nam tren ngan xep.
        edict_t* got = SH_CALL(engine, &IVEngineServer::CreateEdict)(i);

        if (got) {
            AuditAdd(g_PendingClass);
            g_Cursor = (i - 1 < NET_LIMIT) ? (EXT_LIMIT - 1) : (i - 1);
            RETURN_META_VALUE(MRES_SUPERCEDE, got);
        }
    }

    // Extension range full - fall back to the engine. Costs a networked slot
    // but never corrupts anything.
    RETURN_META_VALUE(MRES_IGNORED, nullptr);
}

// ==========================================================================
// Plugin lifecycle
// ==========================================================================
static void ReadStage() {
    FILE* f = OpenPluginFile("stage.txt", "r");
    if (!f) return;
    int v = -1;
    if (fscanf(f, "%d", &v) == 1 && v >= 0 && v <= 1) g_Stage = v;
    fclose(f);
}

bool SamplePlugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();
    GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
    GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
    GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
    GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
    gpGlobals = ismm->GetCGlobals();

    ReadStage();
    EL_LOG("[EdictBudget] ===== stage %d (0=inert, 1=active) =====", g_Stage);

    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelInit, server, this, &SamplePlugin::Hook_LevelInit, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, server, this, &SamplePlugin::Hook_ServerActivate, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &SamplePlugin::Hook_GameFrame, false);

    if (g_Stage == 0) return true;

    if (!ResolveEngineGlobals()) return true;

    LoadPatchSwitches();
    LoadAllowList();
    // Dong trang thai - PHAI o day, ngoai moi nhanh dieu kien.
    // Truoc do no nam trong khoi "if (mo duoc serveronly.txt)" nen khi khong
    // co file do thi khong bao gio in ra.
    EL_LOG("[EdictBudget] noedict=%d mapclear=%d mapclearmax=%d mapclearcarry=%d "
             "heartbeat=%d loadprobe=%d swap=%d swapmax=%d logconsole=%d",
             g_NoEdict ? 1 : 0, g_MapClear, g_MapClearMax, g_MapClearCarryOnly,
             g_Heartbeat, g_LoadProbe, g_Swap, g_SwapMax, g_LogConsole ? 1 : 0);
    if (g_PatchFreetime)    PatchFreetime();
    if (g_PatchIndexBounds) PatchIndexOfEdictBounds();
    if (g_PatchForcedIndex) PatchForcedIndexCheck();
    if (g_PatchFreeGate)    PatchFreetimeGate();
    if (g_PatchTrap) PatchAllocFailTrap();

    // NOEDICT phai cai o DAY (luc Load), khong phai ServerActivate: vtable cua
    // server.dll co san ngay khi module nap, va thunk phai co mat TRUOC khi
    // entity dau tien cua map duoc tao.
    if (g_NoEdict) { LoadNoEdictList(); InstallNoEdict(); }
    // SWAP phai cai TRUOC khi map dau tien phan tich lump entity, vi chinh bo phan
    // tich lump cung di qua dictionary slot 1. Cai o day la som nhat co the.
    if (g_Swap > 0) { LoadSwapList(); InstallSwap(); }

    // CA HAI deu phai thanh cong. Mot mang edict 4096 muc di kem bang snapshot chi
    // 2048 muc thi TE HON HAN viec khong lam gi ca: nhung edict them ra se dung
    // duoc binh thuong cho toi dung luc chung am tham ghi de len trang thai engine
    // nam ke ben.
    g_BigArrayOn = g_PatchBigArray ? PatchEngineAllocSize() : false;
    bool bigTables = g_PatchSnapshot ? PatchSnapshotTables() : false;
    g_EngineArray4096 = g_BigArrayOn && bigTables;

    if (g_EngineArray4096 || g_ImmediateReuse) {
        SH_ADD_HOOK(IVEngineServer, CreateEdict, engine, SH_STATIC(Hook_CreateEdict), false);
        SH_ADD_HOOK(IVEngineServer, CreateEdict, engine, SH_STATIC(Hook_CreateEdict_Post), true);
    }
    if (!g_EngineArray4096) {
        EL_LOG("[EdictBudget] segregation disabled (edict array %s, snapshot tables %s).",
                 g_BigArrayOn ? "ok" : "FAILED", bigTables ? "ok" : "FAILED");
    }
    return true;
}

bool SamplePlugin::Unload(char *error, size_t maxlen)
{
    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelInit, server, this, &SamplePlugin::Hook_LevelInit, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, server, this, &SamplePlugin::Hook_ServerActivate, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &SamplePlugin::Hook_GameFrame, false);
    if (g_EngineArray4096 || g_ImmediateReuse) {
        SH_REMOVE_HOOK(IVEngineServer, CreateEdict, engine, SH_STATIC(Hook_CreateEdict), false);
        SH_REMOVE_HOOK(IVEngineServer, CreateEdict, engine, SH_STATIC(Hook_CreateEdict_Post), true);
    }
    RemoveDetour();
    RemoveWipeClear();
    RemoveMapClear();
    EL_LOG_CLOSE();
    UninstallNoEdict();
    UninstallSwap();
    return true;
}

bool SamplePlugin::Hook_LevelInit(char const *pMapName, char const *pMapEntities,
                                  char const *pOldLevel, char const *pLandmarkName,
                                  bool loadGame, bool background)
{
    // Bo dem SWAP ve 0 tai DAY, khong phai o SwapReport() (ServerActivate).
    //
    // 15/08: log cho thay co lan bao "gap 392" = 312 (m4) + 80 (m3), tuc MOT bao cao
    // gom HAI lan nap map - ServerActivate khong chay dung nhip voi moi lan nap.
    // Cung the voi "gap 82" (80 + 2). Chi sai con so trong log, khong sai viec doi lop
    // (`doi` luon bang `gap`), nhung doc log de suy ra map nao thi bi nham.
    // LevelInit chay dung mot lan cho moi map va TRUOC khi lump duoc phan tich,
    // nen dat lai o day moi khop.
    if (g_Stage != 0 && g_Swap > 0) {
        for (int i = 0; i < g_SwapCount; i++) { g_SwapSeen[i] = 0; g_SwapHits[i] = 0; }
        g_SwapDone = 0;
        g_SwapMapName = pMapName ? pMapName : "?";
    }

    // ---- NONETKILL: sua entity lump TRUOC KHI server.dll parse ----
    // Phai dat TRUOC chot g_BigArrayOn ben duoi - viec nay khong lien quan gi
    // toi huong 4096, va g_BigArrayOn dang la 0.
    if (g_Stage != 0 && g_NoNetKill && pMapEntities) {
        if (g_KillCount == 0) LoadKillList();

        int hits = 0;
        const char* fixed = RewriteLump(pMapEntities, &hits);
        if (fixed) {
            if (g_LumpCopy) free(g_LumpCopy);      // giai phong ban cua map truoc
            g_LumpCopy = (char*)fixed;
            EL_LOG("[EdictBudget] NONETKILL: map '%s' - da doi ten %d entity "
                     "(chuoi lump %u byte, do dai GIU NGUYEN)",
                     pMapName ? pMapName : "?", hits, (unsigned)strlen(g_LumpCopy));
            RETURN_META_VALUE_NEWPARAMS(MRES_IGNORED, true,
                &IServerGameDLL::LevelInit,
                (pMapName, g_LumpCopy, pOldLevel, pLandmarkName, loadGame, background));
        }
        EL_LOG("[EdictBudget] NONETKILL: map '%s' - khong co entity nao khop, "
                 "khong doi gi", pMapName ? pMapName : "?");
    }

    // Chu y dieu kien: viec GHIM la mon no phai tra moi khi mang duoc noi rong,
    // ke ca khi ban than phan tach dang tat.
    if (g_Stage == 0 || !g_BigArrayOn) return true;

    // Den luc nay server.dll da duoc anh xa day du, khac voi luc Load().
    if (g_PatchDetour && !g_OrigCreateEntity) InstallDetour();
    if (!g_SMListHead) ResolveSMListHead();

    AuditReset();
    CensusReset();
    g_Tripped = false;
    g_WarnedNum = g_WarnedMax = g_WarnedGlob = false;
    g_MinFreeSeen = 999999;
    g_EdgeLines   = 0;
    g_MaxBurst    = 0;

    // Lam thay phan xoa m_pPackedData moi map cua engine. Ta KHONG con sua
    // CFrameSnapshotManager::LevelChanged nua, vi mot extension cua ben thu ba
    // moc vao dung ham do va cac byte ta sua khien no khong nap duoc.
    // Serial number thi CO Y de nguyen - engine cung chua bao gio xoa chung, boi
    // mot serial chi duoc doc khi con tro packed cua no khac NULL.
    if (g_PackedTable) memset(g_PackedTable, 0, EXT_LIMIT * sizeof(uint32_t));
    g_Cursor = EXT_LIMIT - 1;
    g_ExtReady = false;

    // SV_AllocateEdicts da chay xong cho map nay va, nho ban va o tren, giao cho
    // ta mot mang 4096 muc that su. No de vung nho KHONG KHOI TAO, ma FL_EDICT_FREE
    // la mot bit BAT - nen mot o toan so 0 doc ra thanh "dang chiem dung". Phai
    // danh dau nua cua ta la TRONG mot cach tuong minh, neu khong thi khong bao gio
    // dat duoc gi vao do.
    // Dong dau len nua tren MOI KHI mang duoc noi rong, khong phai chi khi phan
    // tach dang bat.
    //
    // SV_AllocateEdicts tra ve vung malloc tho, va FL_EDICT_FREE la bit BAT - nen
    // mot o chua khoi tao doc ra thanh DANG CHIEM DUNG, kem mot con tro m_pUnk
    // ngau nhien. Dat dieu kien nay theo phan tach co nghia la khi mang da noi rong
    // ma phan tach lai tat, cac o 2048-4095 giu rac trong-nhu-that cho bat cu ai
    // cham toi chung. Danh dau chung la trong ton mot luot duyet moi map va xoa bo
    // ca mot lop loi kieu "doc ra mot entity trong rat co ly ma chua tung ton tai".
    if (g_MarkFree && gpGlobals && gpGlobals->pEdicts && *g_max_edicts >= EXT_LIMIT) {
        uint8_t* arr = (uint8_t*)gpGlobals->pEdicts;
        for (int i = NET_LIMIT; i < EXT_LIMIT; i++) {
            memset(arr + i * EDICT_SIZE, 0, EDICT_SIZE);
            *(uint32_t*)(arr + i * EDICT_SIZE) = FL_FREE;
        }
        // Gio cac o da AN TOAN de nhin vao, nhung chi DUNG DUOC khi bang snapshot
        // cung da duoc doi cho.
        g_ExtReady = g_EngineArray4096;
    }

    // Giu CA HAI gioi han o gia tri goc. Mang van la 4096 muc, nhung cac vong lap
    // cua engine, server.dll va SourceMod deu nhin thay dung 2048 - va do la con
    // so ma tat ca bon ho duoc xay dung de lam viec cung.
    // Bao cao xem engine THUC SU dang co gi TRUOC khi dong vao bat cu thu gi. Viec
    // ep maxEntities ve 2048 la mot GIA DINH, chua bao gio la mot PHEP DO, va no
    // la cau lenh duy nhat chi chay khi bigarray dang bat - dung cai nhom lam hong
    // vong hoi sinh luc wipe.
    EL_LOG("[EdictBudget] before pin: sv.max_edicts=%d sv.num_edicts=%d "
             "gpGlobals->maxEntities=%d",
             (int)*g_max_edicts, g_num_edicts ? (int)*g_num_edicts : -1,
             gpGlobals ? gpGlobals->maxEntities : -1);

    if (g_PinMax)     *g_max_edicts = NET_LIMIT;
    if (g_PinGlobals && gpGlobals) gpGlobals->maxEntities = NET_LIMIT;

    EL_LOG("[EdictBudget] LevelInit(%s): array=%p size=%d, limits held at %d, extension %s",
        pMapName, gpGlobals ? gpGlobals->pEdicts : NULL, EXT_LIMIT, NET_LIMIT,
        g_ExtReady ? "ready" : "UNAVAILABLE");
    return true;
}

// --------------------------------------------------------------------------
// Bo canh cho DUY NHAT mot tinh huong giet tien trinh ma khong de lai dump
// --------------------------------------------------------------------------
//
// TakeTickSnapshot (engine RVA 0x11D900) mo mot khung ngan xep co dinh 0x1010
// byte va gom chi so cua cac edict dang song vao mot mang WORD tai [ebp-0x1004],
// con stack cookie thi nam o [ebp-4]. Cho do du dung
// (0x1004 - 4) / 2 = 2048 muc - dung bang max_edicts goc, day chat khong du mot o.
//
// Vong lap bi chan boi num_edicts, ma con so do chi tang chu khong giam, va tran
// cua no la max_edicts. Nen chi mot KHOANH KHAC max_edicts == 4096 la du de
// num_edicts vuot 2048 VINH VIEN cho map do, va ban snapshot ke tiep se ghi muc
// thu 2049 de thang len cookie. __security_check_cookie sau do that bai tuc thi:
// tien trinh bien mat, khong minidump, khong mot dong nao tren console - dung cai
// vu sap ma ta dang truy tim.
//
// Hook_CreateEdict co nang max_edicts len 4096 quanh loi goi SH_CALL cua no, nen
// khoanh khac do CHUNG MINH DUOC la co that. Thay vi cu doan tiep, hay canh ca hai
// gia tri moi frame va len tieng ngay lan dau mot trong hai roi khoi tam an toan.
// --------------------------------------------------------------------------
// Canh dung cai cau truc ma may chu chet tren no
// --------------------------------------------------------------------------
//
// Moi lan sap deu roi vao sourcemod.2.l4d2.dll+0x13B63:
//
//     mov ecx, [sourcemod+0xAADFC]   ; PlayerManager::m_hooks.m_Head
//     mov esi, [ecx + 4]             ; m_Head->next   <-- loi o day
//
// m_hooks la mot SourceHook List<IClientListener*>; dau danh sach la mot nut 12
// byte cap phat mot lan luc nap DLL, voi next/prev tro vao chinh no khi rong.
// Co thu gi do lam hong no TU RAT LAU truoc luc sap, va cu sap chi xay ra khi
// PlayerManager::MaxPlayersChanged chay lan ke tiep - nen dia chi loi KHONG noi
// len dieu gi ve thu pham.
//
// Viec chia doi de tim ban va da cham day: moi cong tac CO THE tat deu da tat, ma
// cu sap van song qua het. Vay thi thoi dung hoi ban va nao sai nua, ma di tim
// KHI NAO cau truc do vo. Kiem tinh nhat quan cua vong danh sach mot lan moi frame
// bien mot cu sap tri hoan thanh mot su kien co dau thoi gian, nam ngay canh moi
// thu khac ma log ghi lai o dung khoanh khac do.
static bool g_WarnedSM = false;
// (khai bao o dau file)

static void ResolveSMListHead() {
    HMODULE h = GetModuleHandle("sourcemod.2.l4d2.dll");
    if (!h) return;
    g_SMListHead = (uint32_t*)((uint8_t*)h + 0xAADFC);
    EL_LOG("[EdictBudget] watching SourceMod PlayerManager::m_hooks head at %p",
             g_SMListHead);
}

static bool Readable(const void* p, size_t n) {
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(p, &mbi, sizeof(mbi))) return false;
    if (mbi.State != MEM_COMMIT) return false;
    if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) return false;
    return (uint8_t*)p + n <= (uint8_t*)mbi.BaseAddress + mbi.RegionSize;
}

static void CheckSMList() {
    if (g_WarnedSM || !g_SMListHead) return;
    if (!Readable(g_SMListHead, 4)) return;

    uint32_t head = *g_SMListHead;
    const char* why = NULL;

    if (head == 0)                            why = "head is NULL";
    else if (!Readable((void*)head, 12))      why = "head points at unreadable memory";
    else {
        uint32_t next = ((uint32_t*)head)[1];
        uint32_t prev = ((uint32_t*)head)[2];
        if (!Readable((void*)next, 12))       why = "head->next unreadable";
        else if (!Readable((void*)prev, 12))  why = "head->prev unreadable";
        else if (((uint32_t*)prev)[1] != head) why = "ring broken: prev->next is not head";
    }

    if (why) {
        g_WarnedSM = true;
        EL_LOG("[EdictBudget] *** SourceMod m_hooks list CORRUPTED (%s), head=%08X - "
                 "the server will die in PlayerManager::MaxPlayersChanged the next time "
                 "maxplayers changes ***", why, head);
    }
}

// Nha thoi gian cho tai dung, NHUNG chi khi dai co mang thuc su dang bi ep, va
// nhieu lam vai lan mot giay. Goi vo dieu kien moi frame se lam tai dung chi so
// ngay khoanh khac bat cu thu gi chet, va lam ngap client bang cac lenh xoa tuong
// minh ma khong duoc loi gi.
static int g_LastReuseTick = -1000;

// (khai bao o dau file)

static void ReleaseReuseCooldown()
{
    if (!g_ImmediateReuse || !engine || !g_num_edicts) return;
    if (*g_num_edicts < NET_LIMIT - 192) return;          // con rat nhieu cho

    // Tiet che thi khong sao khi con cho, nhung chinh no da CHE GIAU cu hong: moi
    // lan lay mau deu thay 844+ o trong, roi may chu chet GIUA hai lan lay mau.
    // Error() ben trong ED_Alloc khong bao gio tra ve, nen mot POST hook cung khong
    // the nao quan sat duoc loi goi that bai - cho duy nhat con lai de nhin la NGAY
    // TRUOC moi lan cap phat, va bo tiet che khi da gan mep.
    int tick = gpGlobals ? gpGlobals->tickcount : 0;
    bool nearEdge = (g_MinFreeSeen < 64);
    if (!nearEdge) {
        if (tick - g_LastReuseTick < 15) return;
        g_LastReuseTick = tick;
    }

    // Phep do truoc do cho ra mot ket qua MAU THUAN voi ma may: num_edicts=2048
    // voi 880 edict mang co FL_EDICT_FREE, ma engine van bao "no free edicts".
    // ED_Alloc ghi nho MOI edict trong no di ngang qua (mov ebx,esi tai 0x1E0209)
    // va chi bao loi khi ebx van con -1; nen voi 880 o trong thi nhanh do khong
    // the toi duoc.
    //
    // Chi co MOT cach de ca hai su that cung dung: vong quet KHONG HE CHAY. No bat
    // dau tu
    //     esi = sv.GetMaxClients() + 1
    // va lenh tai 0x1E01E8 nhay qua ca vong lap khi esi >= num_edicts. Vay con so
    // quan trong KHONG PHAI la co bao nhieu o trong - ma la co bao nhieu o trong
    // NAM TRONG CUA SO ma engine thuc su nhin vao.
    //
    // Goi chinh GetMaxClients cua engine (RVA 0x134640 tren doi tuong sv) de doc
    // DUNG cai ma ED_Alloc doc, thay vi tin vao gpGlobals.
    // Hoa ra sv.GetMaxClients() (RVA 0x134640) chi la mot getter mot dong:
    //     mov eax, [ecx+0x104] ; ret
    // nen doc thang truong do - khong goi ham, khong rui ro ve quy uoc goi.
    //
    // Dang luu y: L4DToolZ ghi vao sv[+0x180] (slots_idx 0x60 cua no), mot truong
    // KHAC HAN. Hai truong nay co dong y voi nhau khong, chinh la cau dang hoi.
    uint8_t* svObj = (uint8_t*)g_num_edicts - 0x214;
    int engMaxClients = *(int*)(svObj + 0x104);
    int toolzSlots    = *(int*)(svObj + 0x180);

    int scanStart = engMaxClients + 1;
    int n = (int)*g_num_edicts;
    if (n > NET_LIMIT) n = NET_LIMIT;

    // Dem qua sv.edicts (0x10645774) - dung con tro ma chinh ED_Alloc duyet - chu
    // KHONG phai gpGlobals->pEdicts. Hai cai duoc GIA DINH la bang nhau vi
    // SV_AllocateEdicts gan ca hai, nhung dieu do chua bao gio duoc xac minh luc
    // chay; va neu chung khac nhau thi moi con so "o trong" do duoc hom nay deu
    // dang dem tren SAI mang.
    uint8_t* svEdicts  = g_edicts ? (uint8_t*)*g_edicts : NULL;
    uint8_t* glbEdicts = gpGlobals ? (uint8_t*)gpGlobals->pEdicts : NULL;

    int freeBelow = 0, freeInWindow = 0;
    if (svEdicts) {
        uint8_t* arr = svEdicts;
        for (int i = 0; i < n; i++) {
            if (!(*(uint32_t*)(arr + i * EDICT_SIZE) & FL_FREE)) continue;
            if (i < scanStart) freeBelow++; else freeInWindow++;
        }
    }

    if (freeInWindow < 64) g_MinFreeSeen = freeInWindow;   // arm unthrottled mode

    if (freeInWindow < g_MinFreeSeen || (freeInWindow < 64 && g_EdgeLines++ < 60)) {
        if (freeInWindow < g_MinFreeSeen) g_MinFreeSeen = freeInWindow;
        EL_LOG("[EdictBudget] pressure: num_edicts=%d | sv[0x104]=%d -> engine quet %d..%d | "
                 "TRONG trong cua so=%d, TRONG duoi cua so=%d | sv.edicts=%p gpGlobals->pEdicts=%p%s",
                 (int)*g_num_edicts, engMaxClients, scanStart, n - 1,
                 freeInWindow, freeBelow, svEdicts, glbEdicts,
                 (svEdicts != glbEdicts) ? "  <<< HAI CON TRO KHAC NHAU!" : "");
    }

    engine->AllowImmediateEdictReuse();
}

// --------------------------------------------------------------------------
// Bat DUNG khoanh khac ED_Alloc bo cuoc
// --------------------------------------------------------------------------
//
// Lay mau tu mot hook co tiet che thi khong bao gio bat duoc cu hong: moi lan lay
// mau deu thay num_edicts=2012 (duoi tran 2048) voi 861 edict trong nam trong cua
// so quet - mot trang thai ma ED_Alloc CHUNG MINH DUOC la khong the that bai. Dot
// bung no cua wipe xay ra GON TRONG mot frame, tuc la giua hai lan lay mau.
//
// Mot POST hook tren CreateEdict nhin thay dung MOT thu quan trong: chinh loi goi
// da tra ve NULL. Ghi lai toan bo trang thai ngay tai do, khong tiet che.
static int g_NullReports = 0;

edict_t* Hook_CreateEdict_Post(int forceIndex)
{
    edict_t* ret = META_RESULT_ORIG_RET(edict_t*);
    if (ret == NULL && g_NullReports < 8 && g_num_edicts && g_max_edicts) {
        g_NullReports++;

        uint8_t* svObj = (uint8_t*)g_num_edicts - 0x214;
        int maxClients = *(int*)(svObj + 0x104);
        int scanStart  = maxClients + 1;
        int n = (int)*g_num_edicts;
        if (n > EXT_LIMIT) n = EXT_LIMIT;

        int freeWin = 0, freeLow = 0, freeHigh = 0;
        if (gpGlobals && gpGlobals->pEdicts) {
            uint8_t* arr = (uint8_t*)gpGlobals->pEdicts;
            for (int i = 0; i < n; i++) {
                if (!(*(uint32_t*)(arr + i * EDICT_SIZE) & FL_FREE)) continue;
                if (i < scanStart)      freeLow++;
                else if (i < NET_LIMIT) freeWin++;
                else                    freeHigh++;
            }
        }
        EL_LOG("[EdictBudget] *** CreateEdict(%d) TRA VE NULL *** "
                 "num_edicts=%d max_edicts=%d maxClients=%d | TRONG: duoi=%d trong-cua-so=%d tren2047=%d",
                 forceIndex, (int)*g_num_edicts, (int)*g_max_edicts, maxClients,
                 freeLow, freeWin, freeHigh);
    }
    RETURN_META_VALUE(MRES_IGNORED, ret);
}

// ==========================================================================
// HEARTBEAT - ghi dinh ky so lieu thuc the vao console.log
// ==========================================================================
//
// Muc dich: may chu chinh thuc chay dai ngay cho nhieu du lieu hon test cuc bo.
// trap=1 chi do bang kiem ke LUC CHET - tuc chi biet ket qua, khong biet dien
// bien. Heartbeat cho biet lop nao TANG DAN theo thoi gian, la thu can de thiet
// ke co che thu hoi entity trong luc choi.
//
// CHI GHI LOG. Khong dong vao entity nao.
//
// Moi lan cham nhip:
//   - mot dong tong hop:  song / num_edicts / trong / bien do
//   - cac lop CO THAY DOI so voi lan truoc, sap theo muc tang giam dan
//     (chi in thay doi, khong in ca bang => log khong phinh)
//
// CONG TAC: heartbeat = so GIAY giua hai lan ghi. 0 = tat.
//           Khuyen nghi 300 (5 phut).
// --------------------------------------------------------------------------
#define HB_MAX 96
static char  g_HbName[HB_MAX][40];
static int   g_HbCount[HB_MAX];
static int   g_HbN       = 0;
static bool  g_HbHasPrev = false;
static float g_HbNext    = 0.0f;

static void HeartbeatSample() {
    if (!g_NextEnt || !g_EntList) return;

    char  name[HB_MAX][40];
    int   cnt[HB_MAX];
    int   n = 0, live = 0;

    void* e = g_NextEnt(g_EntList, NULL, NULL);
    while (e) {
        live++;
        const char* cls = *(const char**)((uint8_t*)e + 0x74);
        if (cls) {
            int i = 0;
            for (; i < n; i++) if (strcmp(name[i], cls) == 0) { cnt[i]++; break; }
            if (i == n && n < HB_MAX) {
                strncpy(name[n], cls, sizeof(name[0])-1);
                name[n][sizeof(name[0])-1] = 0;
                cnt[n] = 1; n++;
            }
        }
        e = g_NextEnt(g_EntList, NULL, e);
    }

    int num  = g_num_edicts ? (int)*g_num_edicts : -1;
    int free = CountFreeEdicts();
    EL_LOG("[EdictBudget] NHIP: song=%d num_edicts=%d trong=%d bien do=%d lop=%d",
             live, num, free, NET_LIMIT - live, n);

    if (g_HbHasPrev) {
        // chenh lech theo lop, chi in cai co thay doi
        char dn[HB_MAX][40];
        int  dv[HB_MAX];
        int  dn_n = 0;
        for (int i = 0; i < n && dn_n < HB_MAX; i++) {
            int old = 0;
            for (int j = 0; j < g_HbN; j++)
                if (strcmp(g_HbName[j], name[i]) == 0) { old = g_HbCount[j]; break; }
            if (cnt[i] != old) {
                strncpy(dn[dn_n], name[i], sizeof(dn[0])-1);
                dn[dn_n][sizeof(dn[0])-1] = 0;
                dv[dn_n] = cnt[i] - old; dn_n++;
            }
        }
        for (int j = 0; j < g_HbN && dn_n < HB_MAX; j++) {
            bool still = false;
            for (int i = 0; i < n; i++) if (strcmp(name[i], g_HbName[j]) == 0) { still = true; break; }
            if (!still) {
                strncpy(dn[dn_n], g_HbName[j], sizeof(dn[0])-1);
                dn[dn_n][sizeof(dn[0])-1] = 0;
                dv[dn_n] = -g_HbCount[j]; dn_n++;
            }
        }
        for (int a = 0; a < dn_n && a < 12; a++) {
            int best = a;
            for (int b = a + 1; b < dn_n; b++) if (dv[b] > dv[best]) best = b;
            if (best != a) {
                int t = dv[a]; dv[a] = dv[best]; dv[best] = t;
                char tn[40]; strncpy(tn, dn[a], sizeof(tn)); tn[sizeof(tn)-1]=0;
                strncpy(dn[a], dn[best], sizeof(dn[0])); dn[a][sizeof(dn[0])-1]=0;
                strncpy(dn[best], tn, sizeof(dn[0])); dn[best][sizeof(dn[0])-1]=0;
            }
            if (dv[a] == 0) break;
            EL_LOG("[EdictBudget] NHIP:   %+5d  %s", dv[a], dn[a]);
        }
    }

    for (int i = 0; i < n; i++) {
        strncpy(g_HbName[i], name[i], sizeof(g_HbName[0])-1);
        g_HbName[i][sizeof(g_HbName[0])-1] = 0;
        g_HbCount[i] = cnt[i];
    }
    g_HbN = n; g_HbHasPrev = true;
}

void SamplePlugin::Hook_GameFrame(bool simulating)
{
    if (g_Stage == 0) RETURN_META(MRES_IGNORED);
    CheckSMList();
    ReleaseReuseCooldown();

    // Lay mau TUNG FRAME ngay sau khi nap map. Xem khoi giai thich o g_LoadProbe.
    // Doc hai so: num_edicts (moc cao nhat, KHONG BAO GIO GIAM) va so slot trong.
    //   song = num_edicts - trong
    // num_edicts tang ma trong cung tang  => co cap phat roi tra lai (dinh tam thoi)
    // num_edicts tang ma trong dung yen   => entity that su duoc sinh them
    if (g_LoadProbeLeft > 0) {
        int ne   = g_num_edicts ? (int)*g_num_edicts : -1;
        int free = CountFreeEdicts();
        if (ne > g_LoadProbePeak) g_LoadProbePeak = ne;
        EL_LOG("[EdictBudget] NAP[frame %d]: num_edicts=%d trong=%d song=%d (dinh=%d)",
                 g_LoadProbeFrame, ne, free, ne - free, g_LoadProbePeak);
        g_LoadProbeFrame++;
        if (--g_LoadProbeLeft == 0)
            EL_LOG("[EdictBudget] NAP: het %d frame lay mau, dinh num_edicts=%d",
                     g_LoadProbeFrame, g_LoadProbePeak);
    }

    if (g_Heartbeat > 0 && gpGlobals) {
        float now = gpGlobals->curtime;
        if (g_HbNext <= 0.0f) g_HbNext = now + (float)g_Heartbeat;
        else if (now >= g_HbNext) { g_HbNext = now + (float)g_Heartbeat; HeartbeatSample(); }
    }

    if (g_AllocThisFrame > g_MaxBurst) {
        g_MaxBurst = g_AllocThisFrame;
        if (g_MaxBurst >= 32) {
            EL_LOG("[EdictBudget] BURST: %d lan cap phat trong mot frame "
                     "(num_edicts=%d)", g_MaxBurst,
                     g_num_edicts ? (int)*g_num_edicts : -1);
        }
    }
    g_AllocThisFrame = 0;
    if (!g_BigArrayOn) RETURN_META(MRES_IGNORED);

    if (!g_WarnedNum && g_num_edicts && *g_num_edicts > NET_LIMIT) {
        g_WarnedNum = true;
        EL_LOG("[EdictBudget] *** num_edicts=%d exceeded %d - the engine's "
                 "snapshot list holds only %d entries, so the next snapshot will "
                 "overwrite the stack cookie and kill the process with no dump ***",
                 (int)*g_num_edicts, NET_LIMIT, NET_LIMIT);
    }
    if (!g_WarnedMax && g_max_edicts && *g_max_edicts != NET_LIMIT) {
        g_WarnedMax = true;
        EL_LOG("[EdictBudget] *** max_edicts=%d outside a frame hook "
                 "(expected %d) - num_edicts is free to grow past the limit ***",
                 (int)*g_max_edicts, NET_LIMIT);
    }
    // LevelInit ghim gia tri nay MOT LAN moi map. Neu ve sau engine ghi lai no,
    // SourceMod va moi extension se bat dau nhin thay 4096 - ma viec ghim thi chua
    // bao gio duoc xac minh la giu duoc qua khoi dung khoanh khac do.
    if (!g_WarnedGlob && g_PinGlobals && gpGlobals && gpGlobals->maxEntities != NET_LIMIT) {
        g_WarnedGlob = true;
        EL_LOG("[EdictBudget] *** gpGlobals->maxEntities=%d during a frame "
                 "(pinned to %d at LevelInit) - the engine reset it ***",
                 gpGlobals->maxEntities, NET_LIMIT);
    }
    RETURN_META(MRES_IGNORED);
}

void SamplePlugin::Hook_ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
    if (g_Stage != 0 && g_BigArrayOn) {
        EL_LOG("[EdictBudget] ServerActivate: num_edicts=%d (networked budget %d)",
            g_num_edicts ? (int)*g_num_edicts : -1, NET_LIMIT - 1);
        AuditDump();
        CensusDump();
    }

    // Duong co so cho phep do WIPECLEAR: dem thuc the theo lop NGAY SAU khi nap
    // map, truoc bat ky lan wipe nao. So nay la moc de so voi bang kiem ke lucA
    // chet. Chay ca khi wipeclear=0 - no chi doc, khong dong gi.
    if (g_Stage != 0) {
        // Map moi => xoa co thua con sot lai, tranh don nham o vong choi dau tien.
        g_LossPending = false;
        EL_LOG("[EdictBudget] MOC CO SO (sau nap map, chua wipe lan nao): "
                 "num_edicts=%d slot trong=%d",
                 g_num_edicts ? (int)*g_num_edicts : -1, CountFreeEdicts());
        if (g_NoEdict)
            EL_LOG("[EdictBudget] NOEDICT: %d entity da duoc dat EFL_SERVER_ONLY "
                     "(khong ton edict) tren %d vtable", g_NoEdictHits, g_PatchedVtCount);
        SwapReport();       // bao cao sau khi lump da phan tich xong
        // Bat dau lay mau tung frame. Nhieu entity chua spawn xong o thoi diem nay.
        g_LoadProbeLeft  = g_LoadProbe;
        g_LoadProbeFrame = 0;
        g_LoadProbePeak  = g_num_edicts ? (int)*g_num_edicts : 0;
        // g_pGameRules chi ton tai sau khi level da chay => cai o day, khong o Load()
        InstallWipeClear();
        InstallMapClear();
    }
    RETURN_META(MRES_IGNORED);
}

void SamplePlugin::OnVSPListening(IServerPluginCallbacks *iface) {}
void SamplePlugin::AllPluginsLoaded() {}
bool SamplePlugin::Pause(char *error, size_t maxlen) { return true; }
bool SamplePlugin::Unpause(char *error, size_t maxlen) { return true; }
const char *SamplePlugin::GetLicense() { return "GPLv3"; }
const char *SamplePlugin::GetVersion() { return "2.0"; }
const char *SamplePlugin::GetDate() { return __DATE__; }
const char *SamplePlugin::GetLogTag() { return "EDICTBUDGET"; }
// Toan bo ma nguon nay do Claude (Anthropic) viet. Xem khoi TAC GIA o dau file.
const char *SamplePlugin::GetAuthor() { return "Claude (Anthropic) - AI"; }
const char *SamplePlugin::GetDescription() { return "Giu so entity dang song duoi tran 2048 edict"; }
const char *SamplePlugin::GetName() { return "EdictBudget"; }
const char *SamplePlugin::GetURL() { return "http://www.sourcemm.net/"; }

uint8_t* FindPattern(const char* module, const char* pattern, const char* mask) {
    HMODULE h = GetModuleHandle(module);
    if (!h) return nullptr;
    MODULEINFO mi;
    GetModuleInformation(GetCurrentProcess(), h, &mi, sizeof(mi));
    uint8_t* base = (uint8_t*)mi.lpBaseOfDll;
    size_t size = mi.SizeOfImage;
    size_t len = strlen(mask);
    for (size_t i = 0; i + len < size; i++) {
        bool ok = true;
        for (size_t j = 0; j < len; j++) {
            if (mask[j] == 'x' && base[i + j] != (uint8_t)pattern[j]) { ok = false; break; }
        }
        if (ok) return base + i;
    }
    return nullptr;
}
