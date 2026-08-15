# mapclear - don entity luc chuyen man

*Trich tu `src/sample_mm.cpp`. Giu nguyen tieng Viet khong dau va cac dia chi
ham nhu trong ma nguon, de doi chieu duoc.*

## mapclearcarry - vi sao KHONG duoc xoa cai mang sang

```
1 = CHI xoa entity co FCAP_ACROSS_TRANSITION. 0 = xoa tat ca (mac dinh, hanh vi cu).

!!! DE MAC DINH 0. CHE DO 1 DA DUOC THU 09:44 14/08 VA GIET SERVER NGAY.

  MAPCLEAR #1 (che do 2, chi-mang-sang=1): tong 1551 | mang sang 295
  | go 200 (mang sang 200), giu 156, bo qua vi khong mang sang 1153, cham tran 42
  -> "Server is hibernating" + khoi dong lai. Khong ED_Alloc, khong assert.

Luat dung, giai thich duoc CA BA lan chet (thay cho luat "nguong 1300" da sai):
    cap 100  -> trong do    9 cai mang sang -> SONG
    >1300    -> trong do ~270 cai mang sang -> CHET
    carry=1  -> trong do  200 cai mang sang -> CHET
  KHONG PHAI SO LUONG GIET. LA XOA CAI MANG SANG GIET.
  "Nguong 1300" chi la trung hop: xoa cang nhieu thi cang vo phai nhieu cai mang sang.

Co che: hook chay POST nen PrepareLevelChange goc DA lap xong danh sach chuyen man.
Entity co FCAP_ACROSS_TRANSITION nam san trong danh sach do. Xoa sau khi danh sach
da lap => danh sach tro vao vung da giai phong => sap khi engine xu ly chuyen man.
Hook PRE cung khong cuu duoc: engine van phai doc chinh nhung entity ay de lap danh sach.

He qua: chi cai mang sang moi ton edict o map sau, ma cai mang sang thi khong duoc
dung vao => MAPCLEAR VE NGUYEN TAC KHONG GIAI QUYET DUOC "m3 -> m4".
Giu cong tac nay lai chi de ghi lai thi nghiem, KHONG phai de bat.
```

## mapclear: co che day du

```
MAPCLEAR - don entity TRUOC KHI ENGINE don, luc CHUYEN MAN
==========================================================================

XXX KHONG PHAI HUONG 4096. Khong bigarray/snapshot/pinmax/pinglobals/markfree.
   Khong va mot byte nao. Chi moc vtable, giong het wipeclear.

!!  KHAC WIPECLEAR O DIEM SINH TU - DOC TRUOC KHI SUA:
  wipeclear: cung map, xoa nham thi entity DUOC DUNG LAI tu lump
             => giu TOI THIEU moi dung (wipekeep.txt de RONG)
  mapclear : map khac, xoa nham la MAT VINH VIEN do nguoi choi
             => giu TOI DA. Khong chac thi GIU.
  => TUYET DOI khong bung tap giu cua hai ben cho nhau.

CO CHE (da doc tren binary):
  - Map moi bat dau bang bang edict MOI. Rac NGOAI vung chuyen tiep tu bien
    mat => don no vo ich. Chi thu NAM TRONG danh sach mang sang moi dang don.
  - CBaseEntity::ObjectCaps() 0x10056160 MAC DINH tra FCAP_ACROSS_TRANSITION
    => gan nhu MOI THU trong vung deu duoc mang sang, ke ca xac/manh vo.
  - Hai duong mang sang:
      (a) do TREN TAY -> CTerrorGameRules slot 38 serialize thanh KeyValues
          (weaponID, currentMagazine, extraAmmo...). KHONG ton edict.
      (b) do ROI DUOI DAT -> trigger_transition chuan cua Source. TON EDICT.
    => chi (b) la van de.
  - the_hive_m4 vao map chi con 31 slot trong => mang sang ~32 la cham tran.

DIEM MOC: CTerrorGameRules vtable slot 38 = 0x102B8140, hook POST.
  In ra "Preparing player entities for changelevel". __thiscall, ret 4.
  Nam tren MAP CU, SAU anh chup nguoi choi, TRUOC khi bo may save khoi dong.
  Cung vtable wipeclear dang moc (slot 178 = RestartRound = 0x102E0650).

  XXX DA BO slot 27 (BuildAdjacentMapList): no chay 3 cho, 2 cho o MAP MOI
     (CSaveRestore::LoadAdjacentEnts + duong nap .HL2). Hook mu = xoa entity
     map moi ngay luc nap. Va tai slot 27 thi SaveGameState da goi PreSave
     => bang entity da dung => xoa co nguy co con tro treo.

XXX DUNG chay g_debug_transitions de "xem engine in ra": cvar do CHAN LUON
   viec chuyen man, dat m_pfnTouch = 0 => cua phong an toan thanh cua chet.

TAP GIU MAC DINH (an toan, doc them tu mapkeep.txt):
  - toan bo preserve list CUA GAME (dung chung ham voi wipeclear) - bao thu
  - player, weapon_ (trum ca weapon_*_spawn, gascan/propanetank/oxygentank)
  - prop_fuel_barrel (trum ca _piece)
  - ha tang chuyen man: info_landmark, trigger/info_changelevel, trigger_transition
    (xoa may cai nay la hong CHINH viec chuyen man)

CONG TAC: mapclear = 0 tat | 1 CHI QUAN SAT (dem+ghi log, khong xoa) | 2 don that
(g_MapClear khai bao o dau file, canh cac cong tac khac)
```

## WillCarryOver - hoi chinh engine

```
Hoi chinh engine: entity nay CO duoc mang sang map moi khong?
ObjectCaps() la ham ao slot 40 (+0xA0). Bit 0x2 = FCAP_ACROSS_TRANSITION.
Ca InTransitionVolume (0x101FEFB0) lan ComputeEntitySaveFlags (0x101F8D80)
deu goi dung cho nay => hoi thang, khoi phai truyen ten vung.

*** DAY LA KHAC BIET COT LOI SO VOI BAN 1 (ban 1 lam CHET server):
   ban 1 quet CA 1659 entity roi xoa 1497 -> dung ca thu engine dang can.
   ban nay CHI dung toi entity that su se di theo -> pham vi nho hon han.
```

## Danh sach khong duoc don

```
1. DANH SACH KHONG DUOC DON:
     - vat pham cua nguoi choi   -> weapon_ , prop_fuel_barrel*
     - diem chuyen map           -> info_landmark, trigger/info_changelevel,
                                    trigger_transition
     - cua an toan               -> prop_door_rotating_checkpoint
     - nguoi choi                -> player
   + toan bo preserve list CUA GAME (worldspawn, terror_gamerules,
     soundent, scene_manager... xoa may cai nay la chet ngay)
```

## Cong an toan 1: prologue

```
Cong an toan 1: prologue cua 0x102B8140 phai dung.

!!  BAI HOC 09/08: prologue nay CO DIA CHI TUYET DOI, KHONG duoc so ca 16 byte.
  55 8B EC 56 8B 35 | E0 7A 89 10 | 8B 06 8B 50 68 8B
                      ^^^^^^^^^^^ mov esi,[0x10897AE0]
  Bon byte do bi TRINH NAP GHI LAI khi server.dll nap o base khac
  => so nguyen 16 byte thi KHONG BAO GIO khop, cong an toan chan oan.
  (wipeclear khong dinh vi prologue cua no khong co dia chi tuyet doi.)
=> Dung mat na: '?' = byte bi relocate, bo qua.
```
