# CEF - da go khoi ke hoach

*Trich tu `src/sample_mm.cpp`. Giu nguyen tieng Viet khong dau va cac dia
chi ham nhu trong ma nguon, de doi chieu duoc.*

```
CEF - DA GO KHOI KE HOACH (07/08). Ghi lai de khong ai them lai nham.

Y dinh ban dau: dua CEF vao chinh plugin nay, vi CEF goc (`mmscef-code`)
VON LA Metamod plugin chu khong phai SourceMod extension.

NGUOI DUNG CHOT: KHONG chep CEF vao day. Ma nguon CEF goc chi de THAM KHAO,
no KHONG ho tro day du L4D2 - can thiet ke lai neu muon co co che nay.

Ly do ky thuat:
  CEF goc dung `PEntityOfEntIndex` de tim slot trong. Tren L4D2, L4D da BO
  ham do khoi IVEngineServer, nen `engine_wrappers.h` thay bang phep tinh
  con tro thuan - LUON khac NULL => vong lap chay toi maxEntities roi bail.
  Tuc CEF goc la mot NO-OP tren L4D2. No "on dinh" vi no khong lam gi ca.
  => Chep nguyen xi sang day la chep mot thu khong chay.

Neu ve sau can co che nay, phai THIET KE LAI cho L4D2:
  - dung `edict_t::IsFree()` that, khong dung PEntityOfEntIndex
  - va DO TRUOC: hien chua co so lieu nao cho thay co dinh nguy hiem luc
    choi thuong. Do duoc 07/08: slot cao nhat tung dung = 682/2048, luon du
    ~950 cho. Moi dot bung do duoc deu nam o nhanh wipe, va `wipeclear` da
    xu ly.
  - va nho rui ro muc 0-AA: tac gia CEF tu canh bao "PROBABLY UNSTABLE...
    random crashing", va crash sourcemod+0x13b63 xuat hien dung khi ep chi so.
```
