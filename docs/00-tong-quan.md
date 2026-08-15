# Tong quan - nhiem vu, gioi han, so lieu

*Trich tu `src/sample_mm.cpp`. Giu nguyen tieng Viet khong dau va cac dia
chi ham nhu trong ma nguon, de doi chieu duoc.*

```
edictbudget - giu server L4D2 khong chet vi "ED_Alloc: no free edicts"
===========================================================================

===========================================================================
 TAC GIA - DOC TRUOC

 Toan bo ma nguon nay do AI viet: Claude (Anthropic), chay trong Claude Code.
 Khong phai mot phan, khong phai "co AI ho tro" - la TOAN BO: thiet ke, doc
 nguoc server.dll/engine.dll, viet ma, do dac, va cac ghi chu ban dang doc.

 Nguoi dung la nguoi VAN HANH may chu that: ho dat bai toan, chay thu, chup
 log, phat hien loi, va bac bo nhieu ket luan sai cua AI. Nhieu doan trong file
 nay ghi ro "SAI, da sua" chinh la vi the.

 Noi ro dieu nay vi hai ly do:
   1. Ai doc ma nen biet no den tu dau de tu quyet dinh muc do tin tuong.
   2. Nhieu ket luan o day rut ra tu doc nguoc nhi phan, khong phai tu tai lieu
      chinh thuc. Chung deu kem dia chi ham va doan lenh de kiem lai duoc.
      Cai gi khong xac minh duoc thi ghi thang la KHONG XAC DINH.

 Giay phep: GPLv3. Xem file LICENSE.
===========================================================================

NHIEM VU: giu so entity DANG SONG duoi tran 2048.
KHONG nang tran - chi so entity trong giao thuc Source rong 11 bit (toi da
2047), nen entity CO MANG nam o chi so >=2048 se bi client giai ma sai.

!!  GIOI HAN - PHAI DOC:
  Ban va nay KHONG NGAN DUOC HOAN TOAN "ED_Alloc: no free edicts".
  No chi lam hai viec: thu hoi edict DUNG LUC, va go edict khoi nhung lop
  THUC SU KHONG DUNG MANG. Neu ban than map can nhieu hon 2048 entity CO
  MANG cung luc thi khong co cach nao cuu - do la tran cua giao thuc, khong
  phai cua ban va.
  Vi du do duoc: mot map cong dung 312 point_spotlight + 312 spotlight_end
  + 312 beam = 936 edict (45,7%) cho rieng hieu ung anh sang. Ca ba lop deu
  phai co mang. Ban va khong dong toi duoc.

---------------------------------------------------------------------------
BA CO CHE

1. wipeclear - don khi doi survivor thua
   Luc wipe, game CO don entity nhung don MUON. Trinh tu that:
     CDirector::Restart -> RestartRound(slot 178)
                             |-- hoi sinh player      <== an het edict O DAY
                             |-- CleanUpMap(slot 179) <== moi don, DA MUON
   Moc vao DAU RestartRound, lam nua "don" cua CleanUpMap truoc doan ngon kia:
     UTIL_Remove(ngoai preserve list) -> CleanupDeleteList()
     -> AllowImmediateEdictReuse()
   roi de game chay tiep; CleanUpMap tu dung lai map tu entity lump.
   Chi don khi co mission_lost dang cho (co mot-lan). Khong co cong thi no
   don ngay luc t=1.00 khi map vua nap va PHA MAP.
   DO DUOC: 5 wipe lien tiep (map thuong), 3 wipe lien tiep (c6m1_riverbank).

2. freegate - cho phep tai dung slot vua giai phong
   ED_Alloc TU CHOI tai dung mot edict trong 1 GIAY sau khi no duoc giai
   phong. Wipe xoa roi tao lai hang tram entity trong CUNG mot khoanh khac,
   nen khong cai nao qua noi cong do => chet trong khi con ~999 slot trong.
   Doi mot byte trong engine.dll: jae -> jmp. Dinh vi bang QUET CHU KY.
   An toan nho sv_useexplicitdelete (mac dinh bat) - Valve thiet ke no THAY
   CHO thoi gian cho nay.
   DO DUOC (doi chung): cung tinh huong num_edicts=2048 + ~999 slot trong,
   freegate=0 -> CHET, freegate=1 -> chay tiep binh thuong.

3. noedict - khien lop khong dung mang KHONG LAY edict
   CBaseEntity::PostConstructor xet bit 9 cua m_iEFlags (EFL_SERVER_ONLY):
     = 0 -> AddNetworkableEntity    -> dai 0-2047, TON edict
     = 1 -> AddNonNetworkableEntity -> dai 2049-4095, KHONG ton edict
   Dai 2049-4095 (2047 o) la THIET KE GOC cua engine. Tran no chi in canh
   bao roi tra handle khong hop le - KHONG giet server.
   Thay vtable slot 29 (+0x74) cua rieng CLight/CDecal, bat bit roi goi ham
   goc. Khong va byte, khong dung engine.dll.
   DO DUOC: mot map tu CHET o 2048 edict -> nap duoc voi num_edicts=1178.

---------------------------------------------------------------------------
KET QUA DO DUOC TREN BA CHIEN DICH CANG NHAT

Tran engine: max_edicts = 2048. Cot "EDICT du kien" doc tu lump 0 cua BSP bang
tools\bsp_cost.py; cot "do that" la num_edicts luc chay tren may chu.
Cong thuc:  EDICT = (entity trong lump) - (lop trong noedict.txt)
                    + 2 x point_spotlight co spawnflags&1

1. chernobyl  (5 map) - chua ch04_pripyat03, map khoi nguon cua ca du an
     map              lump   noedict go   EDICT du kien   tong lump
     ch01_jupiter     1532       316         1216             1532
     ch02_pripyat01   2204      1138         1067             2205
     ch03_pripyat02   1686       869          816             1685
     ch04_pripyat03   2246      1039         1212             2251
     ch05_pripyat04    940       301          648              949

   PHEP KIEM NGUOC:
     ch04_pripyat03 truoc khi co noedict: CHET o 2048 luc nap.
     Cong thuc du doan (co noedict): 1212. Do that tren may chu: 1178.
     Sai lech +34, tuc 2,9%. Cong thuc viet ra SAU, khop voi su co xay ra TRUOC.

   !! GIOI HAN PHAI NHO - DUNG BIEN CON SO NAY THANH PHAN QUYET:
     Cot "tong lump" KHONG PHAI so entity cung song mot luc. No chi la so dong
     trong lump. Thuc te entity duoc KICH HOAT DAN:
       - weapon_*_spawn tu UTIL_Remove chinh no ngay sau khi sinh vu khi
       - StartDisabled chua kich hoat
       - point_template sinh muon
       - Director sinh dan theo tien trinh choi
     ch02_pripyat01 co "tong lump" 2205 nhung KHONG HE CHET, ke ca truoc khi co
     noedict. ch04_pripyat03 o 2251 thi chet. Hai con so chi cach nhau 46 =>
     khong co nguong sach nao o day.
     Sai so do duoc, LAN NAO CUNG THUA:
       the_hive m3  du doan 1688 -> do that 1592  (-96)
       the_hive m4  du doan 2067 -> do that 1955  (-112)
       pripyat03    du doan 1212 -> do that 1178  (-34)
     => Dung cong thuc lam CAN TREN va BANG XEP HANG. Muon biet map co chet
        khong thi phai DO: loadprobe (8 frame dau) va heartbeat (moi 5 phut).

2. the_hive  (5 map)
     map   EDICT du kien   ghi chu
     m1         966
     m2        1834        639 env_sprite - KHONG go duoc, xem duoi
     m3        1688        80 point_spotlight (he so 3)
     m4        2067        VUOT TRAN. 312 point_spotlight = 936 edict
     m5        1343
     Do that tren may chu, m4: dinh num_edicts=1955, trong=0, cho tho 93 slot.
     Sau khi bat swap: song 1954 -> 1330. Cho tho 93 -> 718 slot.
     m3 sau khi bat swap: song 1591 -> 1431.

3. anemoia / backroom  (6 map)
     map           lump   noedict go   EDICT du kien
     arcade        1246       433          812
     kitty         2954      1444         1509   <- noedict CUU map nay
     party         2198       399         1798      964 prop_dynamic
     poolrooms      921       306          640
     poolrooms2     914       313          626
     reality       1351       488          862
     kitty la bang chung manh nhat cho noedict: khong co no thi map ~2953 edict,
     vuot tran 900 slot, chet chac luc nap.

TONG KET GIAM DUOC BAO NHIEU (so entity duoc dat EFL_SERVER_ONLY / doi lop):
     noedict   anemoia kitty   1444 entity  |  chernobyl ch02  1138
               chernobyl ch04  1039         |  chernobyl ch03   869
               anemoia reality  488         |  the_hive m4      465
               the_hive m3      443
     Chi MOT truong hop da CHUNG MINH duoc la "khong co no thi chet":
       ch04_pripyat03 - chet that o 2048 truoc khi co noedict, sau do nap duoc
       voi num_edicts=1178. Cac map khac chi la con so lump lon, CHUA CHUNG MINH.
     swap      the_hive m4   624 edict (312 x 2)
               the_hive m3   160 edict (80 x 2)
               the_hive m5     8 edict (chi 4/12 cai co spawnflags&1)
               anemoia      ~26 edict/map (chi poolrooms co 13 cai) - khong dang

RUI RO CUA `swap` DA DUOC DINH LUONG (15/08) - NHO HON NHIEU SO VOI LO NGAI BAN DAU:
  `beam_spotlight` giu FCAP_ACROSS_TRANSITION con `point_spotlight` thi bo, nen
  ban dau tuong so entity mang sang tang manh. Do lai bang PVS that:
    m4 -> m5 :  +0   (312 beam_spotlight cua m4 KHONG cai nao trong PVS landmark)
    m3 -> m4 : +48   (48 point_spotlight nam trong PVS cua landmark_m4 tren m3)
  Danh sach chuyen man that: m3->m4 = 22 entity, m4->m5 = 32. Tran engine 512.
  Doi 48 edict lay 784 => KHONG SUA.

  Vi sao truoc do uoc nham 739/1051: server.dll co 54 ham ObjectCaps khac nhau,
  31 trong so do cung `and eax,0xFFFFFFFD` (bo co) nhung khuon byte khac
  CPointEntity nen bi bo sot. Rieng the_hive: CSprite@1009A5D0 (env_sprite 236),
  CBeam@10081580 (beam 312), CSpotlightEnd@101DEEB0 (spotlight_end 312) deu bo co.

  CChangeLevel::BuildChangeList @101FF060 KHONG duyet gEntList. No duyet
  UTIL_EntitiesInPVS(landmark) @10209BC0 - chi entity trong PVS cua info_landmark
  (1-4% ban do) - VA co dong `cmp dword [esi+0x28],0 ; je` bo qua entity KHONG CO
  EDICT. => noedict MIEN NHIEM HOAN TOAN voi chuyen man.
  Vuot 512 goi tier0!Warning (KHONG phai Error), giu 512 muc dau, bo phan du.

CHO PLUGIN CHUA XU LY DUOC:
     the_hive m2  = 1834, thu pham 639 env_sprite.
     anemoia party = 1798, thu pham 964 prop_dynamic.
     Ca hai lop deu CO SendTable rieng (CSprite, CDynamicProp) nen KHONG go mang
     duoc, va deu he so 1 nen swap vo dung. Can co che khac - dang nghien cuu
     huong sua entity lump ngay trong Hook_LevelInit (xem RewriteLump).
     KHI NAO CO CONG THUC CHUNG CHO anemoia THI GHI THEM VAO DAY.

---------------------------------------------------------------------------
FILE CAU HINH  (left4dead2\addons\edictbudget\)
  stage.txt      0 = nam im hoan toan  |  1 = hoat dong
  patches.txt    cong tac tung phan, doi xong chi khoi dong lai server
                 wipeclear = 0 tat / 1 chi quan sat / 2 don that
  noedict.txt    lop bat EFL_SERVER_ONLY. Truoc khi them lop moi phai qua
                 du 6 dieu kien - ghi trong chinh file do.
  wipekeep.txt   lop GIU THEM khi wipeclear don. DE TRONG moi dung: o wipe,
                 entity bi xoa se DUOC DUNG LAI tu entity lump, nen giu them
                 chi lam hep bien do.
  mapkeep.txt    lop KHONG DUOC DON khi chuyen man (chi dung khi mapclear>=2).
                 Nguoc voi wipekeep: o chuyen man, xoa nham la MAT VINH VIEN.

BUILD
  SOURCE_ENGINE PHAI = 15 (LEFT4DEAD2) theo cach danh so cua Metamod.
  Build nham 11 (TF2) lam lech moi chi so vtable, SH_CALL goi nham ham engine.

---------------------------------------------------------------------------
HUONG 4096: NANG GIOI HAN LA LAM DUOC. GIOI HAN 11 BIT MOI LA KHONG THE.

Phai noi ro hai chuyen khac nhau, dung gop lam mot:

  (a) NANG SO EDICT len 4096 hoac cao hon  ->  LAM DUOC, ma o duoi day.
  (b) Dat entity CO MANG o chi so >= 2048  ->  KHONG THE, va khong bao gio
      lam duoc bang cach va server.dll/engine.dll.

Ly do (b) khong the: chi so entity duoc ma hoa trong goi tin bang truong
11 bit (toi da 2047). Do la dinh dang GOI TIN, nam o ca hai dau day - client
va server. Va server khong lam client hieu duoc chi so 2048; client se giai
ma ra mot chi so khac han. Muon sua thi phai sua ca client.dll cua tung
nguoi choi, tuc khong kha thi.

=> Cho trong dai 2048-4095 CHI dung duoc cho entity KHONG CO MANG.
   Va engine DA CO san co che cho viec do: EFL_SERVER_ONLY + nua tren cua
   m_EntPtrArray (xem noedict o duoi). Khong can va byte nao.

---------------------------------------------------------------------------
CAC BYTE NANG GIOI HAN - GHI LAI DE DOI CHIEU, MAC DINH TAT HET

  bigarray   SV_AllocateEdicts cap 4096 edict thay vi 2048.
             Chu ky trong engine.dll:
                 B8 00 08 00 00   mov eax, 0x800      <- 2048
                 89 86 18 02 00 00
                 A3 ?? ?? ?? ??
             Ghi de 4 byte tai m+1 bang so o muon cap (EXT_LIMIT = 4096).
             Dat 8192 cung chay - mang cap phat theo so nay.

  snapshot   Doi 7 cho truy cap hai bang m_pPackedData / m_pSerialNumber
             sang bo dem 4096 o. BAT BUOC di kem bigarray: mang 4096 edict
             voi bang snapshot 2048 o thi TE HON la khong lam gi - phan
             edict thua se ghi de len bo nho ben canh.
             7 chu ky dang  8B 84 B1 9C ...  ->  8B 04 B5 <dia chi moi>

  pinmax     LevelInit ghim sv.max_edicts ve 2048.
  pinglobals LevelInit ghim gpGlobals->maxEntities ve 2048.
             Hai cai nay giu TRAN CUA ENGINE o 2048 de bo cap phat cua
             engine khong tu dat entity len dai cao. Thieu chung thi
             num_edicts leo qua 2047 va entity CO MANG tran len dai cao
             - dung dieu (b) noi tren.

  markfree   LevelInit dong dau FL_EDICT_FREE len cac o 2048-4095.

---------------------------------------------------------------------------
VI SAO VAN TAT HET

  1. Nhom nay LAM HONG VONG HOI SINH LUC WIPE - tuc pha luon wipeclear,
     thu duy nhat dang giai quyet duoc dot bung lon nhat.
  2. No khong giai duoc bai toan goc. Cho trong o dai cao chi chua duoc
     entity khong co mang, ma viec do noedict lam duoc bang duong CHINH
     THUC cua engine, khong can va byte.
  3. Do thuc te: bat bigarray+snapshot ma thieu pinmax/pinglobals thi
     num_edicts = 2060, entity NGAU NHIEN tran len tren 2047 - mat on dinh.

  Ma van con de doi chieu va de ai muon do lai thi co san. Khong duoc bat
  trong ban chay.
```
