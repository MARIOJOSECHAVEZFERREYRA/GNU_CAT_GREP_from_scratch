# Simple Bash Utils

Matn bilan ishlash uchun Bash utilitalarini ishlab chiqish: cat, grep.

>💡 **Ushbu loyiha haqida biz bilan fikr-mulohazalaringizni baham ko’rish uchun [bu yerni bosing](http://opros.so/kAnXy).** Bu anonim bo’lib, jamoamizga ta’limni yaxshilashga yordam beradi. Loyihani tugatgandan so'ng darhol so'rovnomani to'ldirishni tavsiya qilamiz.

## Contents

0. [Preamble](#preamble)
1. [Chapter I](#chapter-i) \
    1.1. [Introduction](#introduction)
2. [Chapter II](#chapter-ii) \
    2.1. [Information](#information)
3. [Chapter III](#chapter-iii) \
    3.1. [Part 1](#part-1-работа-с-утилитой-cat)  
    3.2. [Part 2](#part-2-работа-с-утилитой-grep)  
    3.3. [Part 3](#part-3-дополнительно-реализация-некоторых-флагов-утилиты-grep)  
    3.4. [Part 4](#part-4-дополнительно-реализация-комбинаций-флагов-утилиты-grep) 


## Preamble

![simple_bash_utils](misc/rus/images/bashutils.png)

1993 yilning kuzi, oddiy bulutli kun. Hewlett-Packard dagi qizg’in ish kunidan keyin siz nihoyat uyga qaytdingiz. Sizning N shahar chekkasida kichkina ikki xonali kvartirangiz bor, muzlatgichda esa, pivo deb nomlangan past alkogolli ichimlikning bankachasi bor. Uni va bir pachka krakerni olib, Dell kompyuteri oldidagi xilvat joyingizga o'tirdingiz.

`*Chertki*` – yoqish tugmasini bosdingiz. Kompyuter ishga tushishi uchun bir necha daqiqa kutish kerak bo'ladi, lekin... bu yaxshi tuyg'u. Dellingizni yoqsangiz, doim xursand bo'lasiz. Bir necha soniya harakatsiz zavqlangandan so’ng, Mosaic brauzerini ochasiz, sevimli forumingizga kirib, pivo ichgancha tredlarni varaqlaysiz. To’satdan juda qiziqarli munozaraga duch kelasiz, undagi birinchi xabar quyidagicha:

> Hello everybody out there using minix -
>
>I'm doing a (free) operating system (just a hobby, won't be big and professional like gnu) for 386(486) AT clones. This has been brewing since april, and is starting to get ready. I'd like any feedback on things people like/dislike in minix, as my OS resembles it somewhat (same physical layout of the file-system (due to practical reasons) among other things).
>
>I've currently ported bash(1.08) and gcc(1.40), and things seem to work. This implies that I'll get something practical within a few months, and I'd like to know what features most people would want. Any suggestions are welcome, but I won't promise I'll implement them :-)
>
>Linus (torvalds@kruuna.helsinki.fi)
>
>PS. Yes — it's free of any minix code, and it has a multi-threaded fs. It is NOT portable (uses 386 task switching etc), and it probably never will support anything other than AT-harddisks, as that's all I have :-(.
>
>— Linus Torvalds

"Juda qiziq", – deb o'yladingiz. Quyidagi tredlarni ham varaqlab, siz Bash portingidagi xatolarni va ba'zi funksiyalar ishlamayotganini ko'rdingiz. Xususan, cat va grep fayllarini o'qish buyruqlari bilan bog'liq muammolar tug’ildi. 
"Bu qiziq masala, shu bilan birga men bu Linus Torvaldsga yordam bera olaman", – dedingiz o'zingizga va tredga allaqachon bu muammo ustida ishlashni boshlaganingizni yozdingiz. Qani, ishga kirishamiz!

## Chapter I

## Introduction

Ushbu loyihada siz C dasturlash tilida matnlar bilan ishlash uchun asosiy Bash utilitalari bilan tanishasiz va ularni qanday ishlab chiqishni o'rganasiz. Ushbu utilitalar (cat va grep) Linux terminalida ishlashda juda tez-tez ishlatiladi. Ushbu loyihada siz Bash utilitalari qanday tashkil etilganligini va tuzilmaviy yondashuvni o'rganasiz.


## Chapter II

## Information

### cat tarixi

>cat Unix ning 1-Versiya kabi dastlabki versiyalarining bir qismi boʻlgan va bitta faylni ekranga nusxalash uchun pr, PDP-7 utilitasi va Multics oʻrnini egallagan.

### catdan foydalanish

cat – Unix-ga o'xshash operatsion tizimlarda eng ko'p ishlatiladigan buyruqlardan biridir. Buyruq matnli fayllarga nisbatan uchta o'zaro bog'liq funksiyaga ega: aks ettirish, ularning nusxalarini birlashtirish va yangilarini yaratish.

`cat [OPTION] [FILE]...`

### cat opsiyalari

| № | Opsiyalar | Tavsif |
| ------ | ------ | ------ |
| 1 | -b (GNU: --number-nonblank) | faqat bo'sh bo'lmagan satrlarni raqamlaydi |
| 2 | -e taxmin qiladi va -v (GNU only: -E ham xuddi shunday, lekin -v ishlatmaydi) | shuningdek, satr oxiridagi belgilarini $ sifatida aks ettiradi  |
| 3 | -n (GNU: --number) | barcha chiqish satrlarini raqamlaydi |
| 4 | -s (GNU: --squeeze-blank) | bir nechta yonma-yon bo'sh qatorlarni siqadi |
| 5 | -t taxmin qiladi va -v (GNU: -T ham xuddi shunday, lekin -v ishlatmaydi)  | shuningdek, tablarni ^I sifatida ham aks ettiradi |

### grep tarixi

> Tompson Li E. MakMahonga “Federalist Xatlari” matnini tahlil qilish va alohida maqolalar muallifligini aniqlashga yordam berish uchun PDP-11 assembler tilida birinchi versiyani yozgan. ed matn muharriri (u ham, Tompson tomonidan yaratilgan) muntazam ifoda qo'llab-quvvatloviga ega bo’lgan, ammo undan bunday katta hajmdagi matn uchun foydalanish mumkin emas edi, shuning uchun Tompson bu kodni alohida instrumentga ajratdi. U bu nomni tanladi, chunki “ed”da g / re / p buyrug'i berilgan shablonga mos keladigan barcha satrlarni chop etdi.
grep birinchi marta Unixning 4-Versiyasiga kiritilgan. U "odatda prototip dasturiy vosita sifatida ko’rsatib o’tiladi" deya ta'kidlab, Makilroy grepni Tompsonning instrumentlar falsafasining Unixdagi "qaytarib bo'lmaydigan joriy etilishi" deya qo’shimcha ravishda yozib qo’yadi.


### grep dan foydalanish

`grep [options] template [file_name]`

### grep opsiyalari

| № | Opsiyalar | Tavsif |
| ------ | ------ | ------ |
| 1 | -e | Shablon.. |
| 2 | -i | Registr farqlarini e'tiborsiz qoldiradi. |
| 3 | -v | Mosliklarni qidirishning ma'nosini invertatsiyalaydi. |
| 4 | -c | Faqat mos keladigan satrlar sonini chiqaradi. |
| 5 | -l | Faqat mos keladigan fayllarni chiqaradi. |
| 6 | -n | Har bir chiqish satriga kirish faylidagi satr raqami bilan oldindan xabar beradi. |
| 7 | -h | Mos satrlarni fayl nomlari bilan oldindan xabar bermasdan chiqaradi. |
| 8 | -s | Mavjud bo'lmagan yoki o'qib bo'lmaydigan fayllar haqidagi xato xabarlarini bostiradi. |
| 9 | -f file | Fayldan muntazam ifodalarni oladi. |
| 10 | -o | Faqat mos satrning mos keladigan (bo'sh bo'lmagan) qismini chop etadi |


## Chapter III

- Dasturlar gcc kompilyatoridan foydalangan holda C11 standartidagi C tilida ishlab chiqilishi kerak.
- cat va grep dasturlari uchun kod mos ravishda src/cat/ va src/grep/ papkalaridagi develop tarmog’ida bo'lishi kerak.
- Eskirgan yoki iste’moldan chiqarilgan til konstruksiyalari va kutubxona funksiyalaridan foydalanmang. Til va ishlatiluvchi kutubxonalar bo’yicha rasmiy hujjatlardagi legacy va obsolete belgilariga e'tibor bering. POSIX.1-2017 standartiga qarab mo’ljal oling.
- Kod yozishda C++ uchun Google Style ((havola)[(https://google.github.io/styleguide/cppguide.html)]) ga amal qiling.
- Dasturlar buyruq qatori argumentlariga ega bo’lgan bajariladigan fayl bo'lishi kerak
- Dasturlarni yig’ish tegishli maqsadlarga ega Makefile yordamida sozlanishi kerak: s21_cat, s21_grep.
- Agar siz begona kutubxonalardan foydalansangiz, Makefileda ularning ulanishi/yuklanishini nazarda tutuvchi yig’im ssenariylari joylashgan bo’lishi kerak.
- Real Bash utilitalari xatti-harakatlari bilan taqqoslash asosida bayroqlar va kiritish qiymatlarining barcha variantlari uchun integratsiya testlari bilan qamrab olish talab qilinadi.
- Dastur tuzilmaviy dasturlash tamoyillariga mos ravishda ishlab chiqilishi kerak.
- Kodlarning takrorlanishini bartaraf etish va utilitalar o'rtasida umumiy modullarni qayta ishlatish kerak. Umumiy modullar alohida src/common papkasiga joylashtirilishi mumkin.
- Standart va nostandart C tili kutubxonalaridan yoki boshqa loyihalardan o'zingiz tomoningizdan ishlab chiqilgan kutubxonalardan foydalanish mumkin.
- Xatolik holati yuzaga kelganda xabar ifodasi muhim emas.
- stdin orqali kiritilganlarni qayta ishlash shart emas.


## Part 1. cat utilitasi bilan ishlash

cat utilitasini ishlab chiqishingiz kerak:
- U yuqorida sanab o'tilgan barcha bayroqlarni (jumladan, GNU versiyalarini) [qo'llab-quvvatlashi kerak](#cat-opsiyalari).
- Boshlang’ich, sarlavha va yig’ish fayllari src/cat/ direktoriyasida joylashgan bo'lishi kerak.
- Yakuniy bajariladigan fayl src/cat/ direktoriyasida joylashgan bo’lishi va s21_cat deb nomlanishi kerak.

## Part 2. grep utilitasi bilan ishlash

grep utilitasini ishlab chiqishingiz kerak:
- Quyidagi bayroqlar qo'llab-quvvatlanishi: `-e` `-i` `-v` `-c` `-I` `-n`
- Muntazam ifodalar uchun faqat pcre yoki regex kutubxonalaridan foydalanishingiz mumkin.
- Boshlang’ich, sarlavha va make fayllari src/grep/ direktoriyasida joylashgan bo'lishi kerak.
- Yakuniy bajariladigan fayl src/grep/ direktoriyasida joylashgan bo'lishi va s21_grep deb nomlanishi kerak.

## Part 3. Qo'shimcha. Ba'zi grep utilitasi bayroqlarini amalga oshirish

Bu esa qo'shimcha ball uchun majburiy bo’lmagan topshiriq: grep utilitasini ishlab chiqing:
- Barcha bayroqlar qo'llab-quvvatlanishi, shu jumladan: `-h` `-s` `-f` `-o`
- Muntazam ifodalar uchun faqat pcre yoki regex kutubxonalaridan foydalanishingiz mumkin.
- Boshlang’ich, sarlavha va make fayllari src/grep/ direktoriyasida joylashgan bo'lishi kerak.
- Yakuniy bajariladigan fayl src/grep/ direktoriyasida joylashgan bo'lishi va s21_grep deb nomlanishi kerak.

## Part 4. Qo'shimcha. grep utilitasida bayroqlar kombinatsiyasini amalga oshirish

Bu esa qo'shimcha ball uchun majburiy bo’lmagan topshiriq: grep utilitasini ishlab chiqing:
- Barcha bayroqlar qo'llab-quvvatlanishi, shu jumladan ularning juft kombinatsiyalari ham (masalan, `-iv` `-in`).
- Muntazam ifodalar uchun faqat pcre yoki regex kutubxonalaridan foydalanishingiz mumkin.
- Boshlang’ich, sarlavha va make fayllari src/grep/ direktoriyasida joylashgan bo'lishi kerak.
- Yakuniy bajariladigan fayl src/grep/ direktoriyasida joylashgan bo'lishi va s21_grep deb nomlanishi kerak.
