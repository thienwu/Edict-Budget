# Bon co che dang chay

*Trich tu `src/sample_mm.cpp`. Giu nguyen tieng Viet khong dau va cac dia chi
ham nhu trong ma nguon, de doi chieu duoc.*

## Hai cong tac xu ly thuc the khong dung mang

```
HAI CONG TAC XU LY THUC THE KHONG DUNG MANG

Van de: entity khong can gui cho client VAN chiem edict trong dai 0-2047 -
dai ma giao thuc 11 bit danh cho thu phai gui. Do la lang phi thuan.
Khong go duoc edict cua entity dang song (DetachEdict() la private, chi
destructor goi duoc). Nen chi con hai duong:

  nonetkill = 1   XOA HAN chung sau khi map nap xong.
                  Duyet gEntList, UTIL_Remove lop khop serveronly.txt,
                  roi CleanupDeleteList() de tra edict ngay.
                  Duoc: tra slot ve dai 0-2047 vinh vien.
                  Mat: mat luon chuc nang cua entity do.

  nonethigh = 1   DAY len dai 2048-4095 thay vi xoa.
                  Dung lai duong Hook_CreateEdict san co (cap phat xuong tu
                  4095) - xem khoi chu thich tai ham do.
                  !!  CAN bigarray=1 VA snapshot=1, neu khong g_ExtReady=false
                  va no khong lam gi ca.
                  !!  Muc 0-AAA: huong nay TUNG gay crash trong phep A/B sach
                  nhat cua du an. Bat lai la co y chap nhan rui ro do de do lai.

Ca hai deu doc danh sach lop tu serveronly.txt (quy tac khop: dong ket thuc
'_' = khop tien to, con lai = khop chinh xac).

!!  KHONG bat ca hai cung luc - chung mau thuan. Neu bat ca hai, nonetkill
thang va nonethigh bi bo qua (co canh bao trong log).
```

## noedict - khien mot so lop KHONG BAO GIO duoc cap edict

```
NOEDICT - khien mot so lop KHONG BAO GIO duoc cap edict
==========================================================================

XXX DAY KHONG PHAI HUONG 4096. Khong dung bigarray/snapshot/pinmax/pinglobals/
   markfree. Khong va mot byte nao cua engine.dll. Neu ai do sua ham nay ma
   thay minh can bat mot trong nam cong tac do => DA DI SAI DUONG, dung lai.

CO CHE (da xac minh tren binary):
  CBaseEntity::PostConstructor @ 0x10055620  (RVA 0x55620) la noi QUYET DINH:
      mov  eax, [esi+0x138]      ; m_iEFlags
      shr  edx, 9
      test dl, 1                 ; bit 9 = EFL_SERVER_ONLY
      je   <nhanh CAP EDICT>     ; = 0 -> AddNetworkableEntity, dai 0-2047
      mov  ecx, gEntList
      call AddNonNetworkableEntity   ; = 1 -> dai 2049-4095, KHONG EDICT
  Ta chi can bat bit 9 TRUOC khi ham goc chay.

DIEM MOC: PostConstructor la HAM AO, o vtable slot 29 (+0x74). Moi lop co
vtable RIENG, nen thay slot 29 cua rieng vtable lop muc tieu => chi tac dong
dung lop do. Khong detour byte, khong dung toi lop khac.

Factory Create cua moi lop co dang:
    push <sizeof>              ; operator new
    call operator new
    push 0                     ; <- bServerOnly = FALSE (co Valve noi toi)
    call <ctor>
    mov  dword ptr [esi], <VTABLE>   ; <- ta tim con so nay
    ...
    call [vtable+0x74]         ; PostConstructor

XXX CAM DUA VAO DANH SACH:
  - lop SOLID hoac CO DI CHUYEN: IVEngineServer::SolidMoved / TriggerMoved
    deu nhan edict_t*. Khong edict => khong cap nhat phan vung khong gian.
  - moi lop trigger_*  (cung ly do)
  - lop co ServerClass RIENG (co DT_ rieng) - client can dung lai chung.
AN TOAN da kiem cho: infodecal (StaticDecal dung chi so BE MAT, khong phai
chi so cua chinh no) va ho light (LightStyle khong kem chi so entity nao).
```

## noedict: cong an toan 3 - dieu kien 1 (SendTable rieng)

```
Cong an toan 3: DIEU KIEN 1 - lop KHONG duoc co SendTable rieng.

Day la dieu kien loc manh nhat trong 6 dieu kien, va la dieu kien DUY NHAT
may kiem duoc thay nguoi. Truoc day no nam trong ghi chu cua noedict.txt,
ai khong doc thi them bua vao va hong khi chay.

GetServerClass() = vtable slot 9. Than ham la `mov eax, imm32 ; ret`:
    B8 <imm32> C3
  imm32 == 0x107D78A8  -> ServerClass CBaseEntity / DT_BaseEntity -> CHO PHEP
  imm32 != 0x107D78A8  -> lop co SendTable rieng                  -> TU CHOI

Da quet 24 lop bang cach nay, hieu chuan voi 8 gia tri biet chac, khop 100%.
Bon lop dang chay (infodecal/light/light_spot/path_track) deu tra 0x107D78A8.
Vi du bi tu choi: spotlight_end (CSpotlightEnd), beam (CBeam),
                  env_sprite (CSprite), light_dynamic (DT_DynamicLight),
                  ca ho trigger_* (CBaseTrigger).
```

## freegate - bo thoi gian cho 1 giay

```
Yeu cau engine BO thoi gian cho 1 giay truoc khi mot edict vua giai phong
duoc cap phat lai.

ED_Alloc tu choi tai su dung mot edict cho toi 1 giay sau khi no duoc giai
phong. Ma mot lan wipe giai phong hang tram entity roi tao lai chung trong
CUNG MOT FRAME, nen khong cai nao du dieu kien, va engine buoc phai noi them
edict moi - do chinh la thu lam can kiet mot map dang o muc 2012/2047.

IVEngineServer::AllowImmediateEdictReuse() la cau tra loi cua chinh Valve cho
viec nay ("Tells the engine we can immdiately re-use all edict indices even
though we may not have waited enough time", eiface.h:345). Convar di kem
sv_useexplicitdelete - mac dinh BAT - lam engine bao cho client biet entity cu
da bien mat TRUOC khi chi so cua no duoc tai dung, va do dung la thu ma thoi
gian cho kia dang bao ve.

Huong nay danh dung vao co che hong that su. Phan tach chi bao gio cung chi
them bien do; con cai nay xoa bo NHU CAU phai co bien do.
```

## freegate: chi tiet vong lap ED_Alloc

```
Bo thoi gian cho 1 giay truoc khi tai su dung edict
==========================================================================

ED_Alloc chi nhan mot edict da giai phong khi:
    comiss  2.0f, freetime[i]      ; freetime < 2.0 (giai phong dau map)
    ja      lay_no
    fsub    freetime[i]            ; curtime - freetime
    fcompi  1.0
    jae     lay_no                 ; hoac da qua 1 GIAY   <-- va o day

Do do wipe (xoa roi tao lai hang tram entity trong CUNG mot frame) khong co
edict nao du dieu kien, engine buoc phai cap moi, num_edicts leo toi tran.
Da do thuc te: num_edicts=2012 voi 906-918 edict DANG TRONG ma engine van
bao "ED_Alloc: no free edicts". Day dung la loi engine Source 2009 ma tac
gia CEF mo ta: "running out of edicts when you have 1000 free".

Doi mot byte 73 -> EB (jae -> jmp) lam moi edict trong deu dung lai duoc
ngay. Dich nhay giu nguyen, khong doi do dai lenh, khong trampoline.

An toan: engine da co san sv_useexplicitdelete (mac dinh BAT) - khi mot chi
so duoc tai dung som, no gui lenh xoa tuong minh xuong client truoc. Do
chinh la co che Valve thiet ke thay cho thoi gian cho nay.
```

## wipeclear - don thuc the o dau RestartRound

```
WIPECLEAR: don thuc the o dau CTerrorGameRules::RestartRound (vtable slot 178),
truoc vong hoi sinh player. Xem khoi giai thich day du gan InstallWipeClear().

BA TRANG THAI, khong phai hai - de moi buoc thu chi doi MOT thu:
  0 = TAT HOAN TOAN. Khong moc vtable, khong nghe su kien. No-op that su,
      dung lam moc doi chieu.
  1 = CHI QUAN SAT. Moc vtable + nghe su kien, log day du moc thoi gian va
      so slot, nhung KHONG xoa mot entity nao. Rui ro gan bang khong, va no
      tra loi cau con treo: tin hieu thua ban TRUOC hay SAU RestartRound,
      va num_edicts cham 2048 o doan nao.
  2 = DON THAT. Lam nua "don" cua CleanUpMap ngay dau RestartRound.

Mac dinh 0. Doi trong patches.txt, khong can build lai.
```

## wipeclear: co che day du

```
WIPECLEAR - don thuc the o DAU chuoi restart, TRUOC vong hoi sinh player.

CTerrorGameRules::CleanUpMap() (RVA 0x2DDB10) da tu lam dung viec nay:
    UTIL_Remove(moi thu ngoai preserve list)
      -> CleanupDeleteList() -> AllowImmediateEdictReuse()
      -> MapEntity_ParseAllEntities()
Van de la no chay MUON. Trinh tu that (da kiem bang capstone tren server.dll
cua chinh server nay, 9.130.288 byte, ImageBase 0x10000000):

  CDirector::Restart          0x2700D0
    m_bRestarting = 1         0x27045F
    RestartRound()            0x2704C4   <- vtable slot 178
      VONG HOI SINH PLAYER    0x2E0794..0x2E08A3   <== tieu edict O DAY
      FIRE round_start_pre_entity        0x2E08CE
      CleanUpMap()            0x2E08DF   <== game moi don O DAY
    m_bRestarting = 0         0x2705DF

Moi thu truoc 0x2E08DF chay khi map VAN giu du 2012 entity / 35 slot trong.
Khoi nay lam nua "don" cua CleanUpMap ngay dau RestartRound roi de game chay
tiep binh thuong - CleanUpMap se thay gan nhu khong con gi de xoa, va
MapEntity_ParseAllEntities van dung lai day du tu entity lump.

QUAN TRONG - day vua la BAN VA vua la PHEP DO:
  log "slot trong truoc -> sau" tra loi luon cau hoi con treo:
    +~1100 slot va het crash  => lo nam TRUOC CleanUpMap, ban va dung
    +~1100 slot ma van crash  => lo nam SAU khi dung lai xong; luc do bai
                                 toan tro ve muc 0-KET-LUAN (map that su
                                 can 2012/2047, khong co lang phi de thu hoi)

Giu nguyen tap "preserve" CUA CHINH GAME (doc runtime tu RVA 0x7ACE40) nen
ngu nghia y het CleanUpMap - chi khac THOI DIEM. Do la lua chon co y: doi
mot bien duy nhat.
```

## wipeclear: nghe su kien (chi de chan doan)

```
--- Nghe su kien: CHI DE CHAN DOAN, khong con quyen chan ------------------

Ban dau dinh dung 'mission_lost' lam cong chan. DA BO (phuong an A).
Ly do, xac minh tren binary chu khong phai suy doan:
  mission_lost ban DUY NHAT tai 0x10269096, trong ham 0x10268CA0. Ham do chi
  push bon chuoi: 'trigger_finale', 'finale_trigger', 'FinaleLost',
  'mission_lost' => day la duong THUA FINALE.
  11 vi tri push mission_lost con lai deu la AddListener(+0x0C) hoac so chuoi.
  c6m1_riverbank khong phai finale => cong se khong bao gio mo.

Van giu listener vi no tra loi mot cau van con treo: thuc te mission_lost co
ban khong, va ban truoc hay sau RestartRound. Log se noi.
CO MOT-LAN, KHONG dung cua so thoi gian.

Ban dau dung cua so 5.0s. SAI: do that te cho thay mission_lost ban luc
t=63.47 con RestartRound chay luc t=70.50 - cach 7.03s, VUOT cua so 5s
=> cong se truot luon ca wipe that.
Khoang cach nay do Director quyet dinh (man hinh thua, dem nguoc...), khong
co gia tri nao an toan de doan. Dung co mot-lan thi khong phai doan:
  mission_lost  -> bat co
  RestartRound  -> co bat thi don, roi TAT co ngay
  nap map moi   -> tat co (tranh co cu sot lai)
```

## wipeclear: danh sach giu bo sung - wipekeep.txt

```
DANH SACH GIU BO SUNG - wipekeep.txt

Preserve list cua game (0x7ACE40) la thu game DUNG. Nhung co nhung lop game
san sang xoa ma XOA SOM lai sinh loi phia client. Ca dau tien gap:
  viec giu lai thuc the cua nguoi choi gay loi MAT BONG.

Nen can mot danh sach GIU THEM, sua duoc bang file, khong phai build lai -
dung kieu nhu serveronly.txt:
  dong ket thuc bang '_'  -> khop TIEN TO ca ho   (vi du "weapon_")
  con lai                 -> khop CHINH XAC ten lop

Dat o: left4dead2/addons/edictbudget/wipekeep.txt
Thieu file = khong giu them gi (chi dung preserve list cua game).
```

## wipeclear: cong chan

```
--- CONG CHAN (KHOI PHUC 07/08 sau khi do that te) ---

Da tung bo cong nay, dua tren suy luan tu disassembly rang mission_lost
"chi ban khi thua finale" (ham 0x10268CA0 co push trigger_finale /
FinaleLost). SUY LUAN DO SAI - do that te tren c6m1_riverbank (KHONG phai
finale) cho thay mission_lost VAN BAN, luc t=63.47.
Lai dung cai bay muc 0-BAI-HOC: suy tu chuoi nam gan nhau.

Hau qua khi khong co cong (log 07/08, wipeclear=2):
  RestartRound duoc goi ngay luc t=1.00 KHI MAP VUA NAP (vong choi dau
  tien, khong phai wipe). Ban va da xoa 1155 entity cua map ngay tai do,
  va slot trong SAU RestartRound van o 1462 (nen la 474) => map KHONG
  duoc dung lai. Pha map.

=> Chi don khi CO mission_lost dang cho. Co mot-lan, khong cua so gio.
```

## swap - doi mot lop entity thanh lop RE HON

```
 SWAP: doi mot lop entity thanh lop khac RE HON ngay luc tao
=====================================================================

Bai toan: `point_spotlight` khi spawn TU TAO THEM `spotlight_end` + `beam`
  => 3 edict cho moi dong trong lump BSP.
  `beam_spotlight` lam viec tuong tu nhung VE HOAN TOAN PHIA CLIENT,
  khong sinh entity con => 1 edict.
  Chinh tac gia the_hive da dung ca hai lop trong cung chien dich
  (m1 co 2 beam_spotlight, m5 co 21, m4 co 312 point_spotlight).
  Doi duoc: m4 937 -> 313, m3 240 -> 80, m5 41 -> 33. Tong 792 edict.

KHAC HAN noedict: day KHONG phai go mang. Client van nhan entity, van ve
  tia sang. Chi la mot lop re hon. Nen khong dinh 6 dieu kien nao het.

CHO MOC â vi sao cho nay sach:
  CreateEntityByName (0x101196B0) khong tu tao gi, no goi qua
    EntityFactoryDictionary()->vtable[1]:
      101196E7 call 0x1020CA70 ; mov eax,[edx+4] ; call eax
  Quet ca .text: 562 cho goi 0x1020CA70, trong do
    558 dung slot 0 (InstallFactory), 1 dung slot 4 (GetCannonicalName),
    va DUNG 3 cho dung slot 1 (Create): CreateEntityByName + 2 nhanh cua
    bo phan tich lump BSP.
  => Va MOT con tro vtable phu ca luc nap lump lan luc choi.
  0x1020CA70 la dia chi plugin DA dung san trong ResolveClassVtable.
  Dung SourceHook-style vtable swap, KHONG detour byte.

ANH XA KEYVALUE (doc datamap cua ca hai lop):
  SpotlightLength / SpotlightWidth / HDRColorScale  TRUNG TEN TUYET DOI
  input LightOn / LightOff, output OnLightOn        trung ten
  cung baseMap = CBaseEntity                        moi khoa ke thua giong het
  MAT DUNG MOT KHOA: HaloScale - client.dll ghi cung halo = 60.0 tai 1006CC80.
    => map nao dat HaloScale 10 (vd the_hive_m4) se thay halo TO GAP 6.
    43/517 point_spotlight tren 50 map von da dat 60 = dung mac dinh, khong doi gi.

spawnflags: bit 0 (bat san) va bit 1 (khong den dong) GIONG HET giua hai lop.
  Quet 517 point_spotlight tren 45 map goc + 5 map hive: chi tung la 2 hoac 3,
  chua cai nao dat bit 2/3/6 => rui ro bat nham xoay/nofog = 0.

m_iClassname tu lump se ghi de lai thanh "point_spotlight". Da chung minh
  server.dll khong co cho nao tra cuu chuoi do ngoai InstallFactory => vo hai,
  va giu tuong thich nguoc cho plugin SourceMod dang loc theo ten lop.

CHUA LAM (co y, de test dan):
  Tu so datamap cua hai lop luc khoi dong de bao khoa nao bi mat.
  GetDataDescMap() = vtable slot 11 (+0x2C), cung khuon `B8 imm32 C3` nhu slot 9.
  datamap_t 24 byte {dataDesc, nFields, className, baseMap};
  typedescription_t 60 byte, ten keyvalue o +0x10.
  Chua viet vi day la doc con tro chua kiem chung tren ban nay - them sau,
  khi co che doi lop da chay on.
```

## swap: dat lai bo dem tai LevelInit

```
Bo dem SWAP ve 0 tai DAY, khong phai o SwapReport() (ServerActivate).

15/08: log cho thay co lan bao "gap 392" = 312 (m4) + 80 (m3), tuc MOT bao cao
gom HAI lan nap map - ServerActivate khong chay dung nhip voi moi lan nap.
Cung the voi "gap 82" (80 + 2). Chi sai con so trong log, khong sai viec doi lop
(`doi` luon bang `gap`), nhung doc log de suy ra map nao thi bi nham.
LevelInit chay dung mot lan cho moi map va TRUOC khi lump duoc phan tich,
nen dat lai o day moi khop.
```
