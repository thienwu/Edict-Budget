# Do dac - log, kiem ke, bay

*Trich tu `src/sample_mm.cpp`. Giu nguyen tieng Viet khong dau va cac dia chi
ham nhu trong ma nguon, de doi chieu duoc.*

## Ghi log ra file rieng

```
GHI LOG RA FILE RIENG
==========================================================================

META_LOG chi day ra console cua server. Neu may chu khong bat ghi console.log
thi moi so lieu do duoc deu mat. Nen toan bo log cua plugin ghi thang vao
file rieng:

    left4dead2ddons\edictbudget\edictbudget.log

Moi dong co dau thoi gian. File mo o che do noi tiep, khong ghi de.
fflush sau moi dong de neu server chet dot ngot van con du log den phut cuoi
- dung luc can nhat.

CONG TAC logconsole = 1 thi in ra CA console (mac dinh 0).
```

## Dem so lan cap phat trong MOT frame

```
Dem so lan cap phat edict trong MOT frame.

Mau cuoi cung truoc khi chet luon la num_edicts=2012 voi ~904 slot trong -
trang thai ma ED_Alloc KHONG THE bao loi (2012 < 2048 nen no cap moi).
De toi duoc nhanh loi thi trong khoang giua hai lan lay mau (<0.25s) phai
co ~940 lan cap phat: 36 lan day num_edicts len 2048, cong 904 lan chiem
het slot trong. Neu dung, wipe la mot dot bung no ~940 entity CUNG LUC va
map that su vuot 2048 o dinh - luc do moi huong "tai su dung slot" deu vo
nghia vi khong con gi de tai su dung.
```

## loadprobe - ghi so edict trong N frame dau

```
Ghi so edict trong N frame dau sau khi nap map, de bat DINH TAM THOI. 0 = tat.

Vi sao can: `MOC CO SO` ghi tai ServerActivate, luc do nhieu entity CHUA spawn xong.
Vi du point_spotlight tao spotlight_end + beam trong Activate()/Think(), tuc la SAU
ServerActivate. Do la ly do m4 ghi num_edicts=1463 luc do trong khi dem tu lump ra
2067 - phan chenh xuat hien o may frame ke tiep.
Ngoai ra ~35 lop weapon_*_spawn tao entity that roi UTIL_Remove chinh no; UTIL_Remove
hoan den cuoi frame nen moi cai chiem 2 edict cung luc trong frame nap.
Ca hai gia thuyet deu chi kiem duoc bang cach lay mau TUNG FRAME.
swap: doi lop entity thanh lop re hon luc tao. Xem khoi giai thich o InstallSwap().
0 = tat | 1 = CHI QUAN SAT (dem, khong doi) | 2 = doi that
```

## Kiem ke moi lop map tao ra

```
Kiem ke MOI lop ma map tao ra, du ta co doi cho no hay khong.

Chon danh sach cho phep bang truc giac thi KHONG AN THUA: tap than trong
logic_/math_/ai_ chi giai phong duoc 10 o tren c1m1_hotel, vi L4D2 dat phan
lon logic cua map trong VScript chu khong phai trong entity. Muon chon co ich
thi phai biet map THUC SU sinh ra nhung gi va bao nhieu cai, sap theo so
luong, de nhung nhom phia may chu lon nhat lo ra ngay.
```

## trap - bay tai chinh nhanh loi cua ED_Alloc

```
Bay tai CHINH nhanh loi cua ED_Alloc
==========================================================================

Moi phep do dat tai IVEngineServer::CreateEdict deu MU: bo dem burst khong
thay frame nao co >=32 lan cap phat, va hook chua bao gio duoc goi cho lan
that bai. Nghia la ED_Alloc duoc goi tu duong noi bo cua engine.

Cho duy nhat con nhin duoc la chinh nhanh loi:
    1E0247  85 DB              test ebx, ebx
    1E0249  0F 88 84 00 00 00  js   1E02D3      -> bao "no free edicts"
    1E024F  ...                                 -> tai su dung ebx

Tam 8 byte do bang mot JMP 5 byte toi stub cua ta + 3 NOP. Stub ghi log roi
dung lai dung hai nhanh goc. Day la duong LANH - chi chay khi engine sap
chet - nen rui ro thap hon han detour tren duong nong.

ebx = chi so edict trong CUOI CUNG ma vong quet nhin thay (-1 = khong thay
cai nao). Do chinh la con so can biet: engine co that su khong thay slot
trong nao khong, trong khi ta dem duoc ~912.
```

## Kiem ke tai thoi diem het edict

```
Kiem ke TAI DUNG THOI DIEM NAY: cai gi dang chiem 2048 slot?

Moi lan kiem ke truoc day deu dem luc BINH YEN va cho ra buc tranh khac
han - chinh no lam ca hai ngay di sai huong. Day la thoi diem duy nhat
co nghia: engine vua xac nhan khong con mot slot trong nao.

Lan truoc bang nay KHONG in ra duoc: tieu de duoc log SAU vong lap, va
vong lap goi ham ao GetClassName() tren 2048 edict trong luc engine dang
hap hoi nen cham phai con tro hong va chet truoc khi kip in. Nay:
  - in tieu de TRUOC
  - boc SEH quanh moi lan doc mot edict
  - in tung dong ngay khi gom xong, khong doi toi cuoi
```

## Mau thuan giua so do va ma may

```
Phep do truoc do cho ra mot ket qua MAU THUAN voi ma may: num_edicts=2048
voi 880 edict mang co FL_EDICT_FREE, ma engine van bao "no free edicts".
ED_Alloc ghi nho MOI edict trong no di ngang qua (mov ebx,esi tai 0x1E0209)
va chi bao loi khi ebx van con -1; nen voi 880 o trong thi nhanh do khong
the toi duoc.

Chi co MOT cach de ca hai su that cung dung: vong quet KHONG HE CHAY. No bat
dau tu
    esi = sv.GetMaxClients() + 1
va lenh tai 0x1E01E8 nhay qua ca vong lap khi esi >= num_edicts. Vay con so
quan trong KHONG PHAI la co bao nhieu o trong - ma la co bao nhieu o trong
NAM TRONG CUA SO ma engine thuc su nhin vao.

Goi chinh GetMaxClients cua engine (RVA 0x134640 tren doi tuong sv) de doc
DUNG cai ma ED_Alloc doc, thay vi tin vao gpGlobals.
Hoa ra sv.GetMaxClients() (RVA 0x134640) chi la mot getter mot dong:
    mov eax, [ecx+0x104] ; ret
nen doc thang truong do - khong goi ham, khong rui ro ve quy uoc goi.

Dang luu y: L4DToolZ ghi vao sv[+0x180] (slots_idx 0x60 cua no), mot truong
KHAC HAN. Hai truong nay co dong y voi nhau khong, chinh la cau dang hoi.
```

## Bat dung khoanh khac ED_Alloc bo cuoc

```
Bat DUNG khoanh khac ED_Alloc bo cuoc
--------------------------------------------------------------------------

Lay mau tu mot hook co tiet che thi khong bao gio bat duoc cu hong: moi lan lay
mau deu thay num_edicts=2012 (duoi tran 2048) voi 861 edict trong nam trong cua
so quet - mot trang thai ma ED_Alloc CHUNG MINH DUOC la khong the that bai. Dot
bung no cua wipe xay ra GON TRONG mot frame, tuc la giua hai lan lay mau.

Mot POST hook tren CreateEdict nhin thay dung MOT thu quan trong: chinh loi goi
da tra ve NULL. Ghi lai toan bo trang thai ngay tai do, khong tiet che.
```

## heartbeat - ghi dinh ky so lieu

```
HEARTBEAT - ghi dinh ky so lieu thuc the vao console.log
==========================================================================

Muc dich: may chu chinh thuc chay dai ngay cho nhieu du lieu hon test cuc bo.
trap=1 chi do bang kiem ke LUC CHET - tuc chi biet ket qua, khong biet dien
bien. Heartbeat cho biet lop nao TANG DAN theo thoi gian, la thu can de thiet
ke co che thu hoi entity trong luc choi.

CHI GHI LOG. Khong dong vao entity nao.

Moi lan cham nhip:
  - mot dong tong hop:  song / num_edicts / trong / bien do
  - cac lop CO THAY DOI so voi lan truoc, sap theo muc tang giam dan
    (chi in thay doi, khong in ca bang => log khong phinh)

CONG TAC: heartbeat = so GIAY giua hai lan ghi. 0 = tat.
          Khuyen nghi 300 (5 phut).
```
