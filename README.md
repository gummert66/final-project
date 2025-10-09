# ระบบจัดการการชำระเงิน (final-project)

- โปรแกรมสำหรับจัดการข้อมูลการชำระเงินจากไฟล์ `paymentinfo.csv` (เพิ่ม/ค้นหา/แก้ไข/ลบ) ผ่านเมนูบนคอนโซล
- ไฟล์หลัก: `payment.c`, `payment.h` และข้อมูลตัวอย่าง `paymentinfo.csv`

**วิธีคอมไพล์และรัน (Windows + GCC/MinGW)**
- คอมไพล์โปรแกรมหลัก:
  - `gcc -std=c11 -Wall -Wextra -O2 -o payment.exe payment.c`
- รันโปรแกรม:
  - `./payment.exe`

หมายเหตุคอมไพล์ (ตัวเลือกด้วย MSVC)
- หากใช้ MSVC Developer Command Prompt:
  - `cl /std:c11 /W4 payment.c /Fe:payment.exe`

**รันทดสอบหน่วย (Unit Test)**
- สร้างไบนารีทดสอบและรัน:
  - `gcc -DUNIT_TEST -std=c11 -Wall -Wextra -O0 -g -o test_payment_unit.exe test_payment_unit.c payment.c`
  - `./test_payment_unit.exe`

**รันทดสอบปลายทาง (E2E)**
- รันสคริปต์ PowerShell:
  - `powershell -ExecutionPolicy Bypass -File .\test_payment_e2e.ps1`

ภายในโปรแกรม
- ในเมนูหลัก กด `5` เพื่อรันทดสอบหน่วย และ `6` เพื่อรันทดสอบ E2E

ข้อควรรู้และความปลอดภัยของข้อมูล
- จำกัดจำนวนระเบียนสูงสุดไว้ที่ `MAX` (100) หากเกินจะถูกละเว้น
- ตรวจสอบรูปแบบรหัสการชำระเงิน (ตัวอย่าง `P001`) ก่อนโหลด/บันทึก
- รับค่าตัวเลขอย่างปลอดภัยด้วย `fgets` และตรวจสอบช่วงค่าที่อนุญาต
- บันทึกไฟล์แบบอะตอมมิก: เขียนไปยังไฟล์ชั่วคราว `*.tmp` แล้วเปลี่ยนชื่อเป็นไฟล์จริง

ธงคอมไพล์ที่แนะนำ
- โหมดดีบัก: `-std=c11 -Wall -Wextra -Werror -O0 -g`
- โหมดรีลีส: `-std=c11 -O2 -DNDEBUG`

