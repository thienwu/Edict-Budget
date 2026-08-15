# nonetkill - doi ten classname trong lump (DA LOAI BO)

*Trich tu `src/sample_mm.cpp`. Giu nguyen tieng Viet khong dau va cac dia
chi ham nhu trong ma nguon, de doi chieu duoc.*

```
nonetkill: DOI TEN classname TAI CHO trong entity lump, o LevelInit.

CO CHE (da xac minh tren binary):
  classname la  -> CEntityFactoryDictionary::Create (0x10206A40)
                   -> DevWarning("Attempted to create unknown entity type %s!")
                   -> tra NULL
                -> MapEntity_ParseEntity (0x101198F0): DevWarning("Can't init %s"),
                   KHONG deref NULL
                -> MapEntity_ParseAllEntities (0x1011A600): bo qua NULL
  => entity im lang khong duoc spawn, KHONG ton edict, KHONG ton han muc nao.
  Khop tai lieu Valve: "Entities... not recognized by the server do not create
  edicts... they are simply not spawned."

XXX HAI DUONG GIET SERVER - PHAI TRANH:
  0x1011A6C0  khoi khong mo bang '{'      -> tier0!Error  (import 0x105C1224)
  0x10119943  khoi thieu key "classname"  -> tier0!Error
  => TUYET DOI khong xoa khoi, khong doi do dai chuoi. CHI ghi de gia tri.

CACH DOI: thay DUNG MOT ky tu dau thanh '~'.
  infodecal -> ~nfodecal
  Bao dam cung do dai, va khong classname nao cua L4D2 bat dau bang '~'
  (557 classname da liet ke, khong cai nao).

###########################################################################
XXX DANH SACH MAC DINH: RONG. DUNG THEM 'light*' HAY 'infodecal' VAO DAY.
###########################################################################
Mac dinh cung tay { infodecal, light, light_spot } la SAI:
  -> ch04_pripyat03 HIEN THI SAI ANH SANG.

NGUYEN NHAN GOC - nonetkill khac nonethigh o mot diem sinh tu:
  nonethigh : entity VAN DUOC TAO, VAN chay Spawn()/Activate(), chi la khong
              cap edict. Moi TAC DUNG PHU van xay ra. -> anh sang DUNG.
  nonetkill : entity KHONG BAO GIO TON TAI. Spawn()/Activate() khong chay.
              -> MAT SACH tac dung phu.

=> nonetkill SAI VE BAN CHAT voi moi entity ma GIA TRI CUA NO NAM O TAC DUNG
   PHU LUC SPAWN. Da xac minh tren binary (output/binscan/step_light.py):

  CLight::Spawn 0x1010FA10  (dung chung cho light / light_spot /
                             light_directional; light_environment = jmp toi day)
    [esi+0x140] m_iszName == 0  -> UTIL_Remove(this)      // den "tro", tu xoa
    [esi+0x140] m_iszName != 0  -> neu m_iStyle >= 32:
                                     engine->LightStyle(m_iStyle, pattern)
                                     (0x107F7698 = g_pEngineServer, vt +0xA0)
    Den CO TEN = den BAT/TAT DUOC. VRAD nuong no thanh mot lightstyle rieng
    luc compile; entity luc chay la thu DUY NHAT dat trang thai dau cho lop
    lightmap do. Cat entity -> LightStyle() khong chay -> lop do giu mac dinh
    -> SANG SAI. Den KHONG ten thi da tu xoa san, cat cung KHONG duoc gi.

  CDecal::Spawn 0x102362A0 / CDecal::Activate 0x10236D10
    Spawn:    m_nTexture < 0 hoac (deathmatch && lowprio) -> UTIL_Remove
              con lai -> SONG. Server dedicated khong phai deathmatch => SONG.
    Activate: khong targetname -> jmp StaticDecal() (dan decal roi TU XOA)
    => infodecal CHUA TUNG giu edict lau dai. Cat no tiet kiem GAN NHU BANG 0,
       doi lay TOAN BO decal cua map. Lo von nang.
    (infodecal do VScript tao thi sinh luc chay, khong qua lump -> khong dinh.)

!!  Muon giam edict cho ho light/infodecal thi dung NONETHIGH, khong phai day.

Doc tu nonetkill.txt neu co (moi dong mot classname). Truoc khi them BAT CU
lop nao, phai tra loi duoc: "Spawn()/Activate() cua no co lam gi khong?"
Neu co -> KHONG duoc cat.
XXX KHONG them lop nhom "song lau dai" (logic_auto, func_nav_attribute_region,
   info_gamemode, info_survivor_position...).
```
