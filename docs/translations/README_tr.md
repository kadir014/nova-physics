<p align="center"><img src="https://raw.githubusercontent.com/kadir014/kadir014.github.io/master/assets/novaphysics.png" width=340></p>
<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg">
  <img src="https://img.shields.io/badge/version-0.7.0-yellow">
  <a href="https://app.codacy.com/gh/kadir014/nova-physics/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade"><img src="https://app.codacy.com/project/badge/Grade/9556f3db17d54b288557d3b2e9dbf366"></a>
</p>
<p align="center">
Nova Physics, hafif ve kullanımı kolay bir 2B fizik motorudur.
</p>
<p align="center">
<sup><i>Bu sayfayı ayrıca şu dillerde okuyabilirsin</i></sub>
<br>
<a href="./../../README.md"><img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/docs/_static/flag_uk.png" width=30></a>
<a href="#"><img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/docs/_static/flag_tr.png" width=30 alt="TR"></a>
</p>



# Özellikler
- Basit ve kullanıcı-dostu arayüz
- Katı cisim dinamiği
- Basit şekil çarpışmaları (daire, dikdörtgen, çokgen, AABB)
- Broad-phase stratejileri (Spatial hashing & BVH-ağacı)
- Fiziksel materyal özellikleri (sürtünme kuvveti, elastiklik ve yoğunluk)
- Eklem kısıtlamaları (yay, uzaklık, menteşe ..)
- Gayet iyi yığın dengesi ve çarpışma sürekliliği
- [Erin Catto'nun](https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf) iteratif "sequential impulse" çözücü algoritması
- Semi-implicit (symplectic) Euler integrasyonu
- Çarpışma eventleri
- CPU yükünü azaltmak için cisimleri uyutma
- Çekici güçler
- Birleşik profiler
- Dış bağımlılığı olmayan taşınabilir codebase
- SDL2 kullanan çeşitli etkileşimli örnek demolar



# Yol Haritası & Gelecek
Nova Physics `0.x.x` sürümleri boyunca hala daha erken aşamalarında. Motorun ve API'ın hala optimizasyon ve gelişme anlamında kat edecek yolu var. `1.x.x` kilometre taşından önce değinilmesi gereken bazı noktalar:

- ### Daha iyi ve hızlı broad-phase
  Nova'da şuan kullanılabilir olan broad-phase stratejileri spatial hash grid ve BVH (kaplayan alan hiyerarşisi) ağacı. İkisi de hızlı ama hala geliştirilebilirler, özellikle BVH-ağacının oluşumu ve multi-thread kullanan SHG.

- ### Python Binding
  Nova Physics'in Python modülü ([burada](https://github.com/kadir014/nova-physics-python)) hala WIP. Kullanımı kolay ve Pythonic bir arayüzü olmasını istiyorum. Başka diller için bindinglere her zaman açığız!



# Yükleme & Derleme
Geliştirme kütüphaneleri her zaman son sürümle beraber `nova-physics-X.X.X-devel.zip` (veya `.tar.gz`) ismiyle yüklenirler. Arşivi [buradan](https://github.com/kadir014/nova-physics/releases) indirip `libnova.a` (veya `libnova.lib`) 'i favori derleyicinizle kullanabilirsiniz.
<br><br>
Fakat eğer Nova Physics'i baştan kendiniz derlemek istiyor (veya gerek duyuyorsanız), [derleme kılavuzunu](https://github.com/kadir014/nova-physics/blob/main/BUILDING.md#building-nova-physics-static-libraries) kullanın.



# Örnekler

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/examplegif.gif" width=400>

Örnek demoları [examples](https://github.com/kadir014/nova-physics/blob/main/examples/) klasöründe, demoları çalıştırmak için [demoları derleme kılavuzunu](https://github.com/kadir014/nova-physics/blob/main/BUILDING.md#running-nova-physics-example-demos) kullanın



# Dökümantasyon
Dökümantasyona [buradan](https://nova-physics.rtfd.io) erişebilirsiniz.



# Kaynaklar & Referanslar
Aşağıdakiler Nova Physics'i yazarken bana yardımcı olan müthiş kaynaklardan bazıları.
- **Erin Catto**, [GDC Presentations](https://box2d.org/publications/)
- **Chris Hecker**, [Rigid Body Dynamics](https://chrishecker.com/Rigid_Body_Dynamics)
- **Randy Gaul**, [Game Physics Articles](https://tutsplus.com/authors/randy-gaul)
- **Allen Chou**, [Physics Blogs](https://allenchou.net/category/physics/)
- **Marjin Tamis** & **Giuseppe Maggiore**, [Constraint Based Physics Solver](http://mft-spirit.nl/files/MTamis_ConstraintBasedPhysicsSolver.pdf)
- **Micheal Manzke**, [Multiple Contact Resolution](https://www.scss.tcd.ie/~manzkem/CS7057/cs7057-1516-10-MultipleContacts-mm.pdf)
- **Dirk Gregorius**, [Robust Contact Creation for Physics Simulations](http://media.steampowered.com/apps/valve/2015/DirkGregorius_Contacts.pdf)
- **Andrew Sevenson**, [Separating Axis Theorem Explanation](https://www.sevenson.com.au/programming/sat/)



# Lisans
[MIT](LICENSE) © Kadir Aksoy

Nova Physics her zaman ücretsiz ve açık-kaynak olacaktır. Fakat [bağışlara](https://www.buymeacoffee.com/kadir014) her zaman açığız!

# Çeviri Notu
Türkçeye çevrilirken bozulmak zorunda kalan veya anlamını koruyamayan / karşılığı olmayan kelimeler ve teknik terimler (alfabetik sırayla):
- **AABB (Axis Aligned Bounding Box)**: Bir cismi veya şekli kapsayan olabilecek en küçük, eksenlere hizalanmış kutu.
- **Kısıtlama (constraint)**: Kısıtlama fizik motorlarının temelidir diyebiliriz. Cisimlerin uzay içerisinde nasıl davranacağını, nasıl davranamayacağını belirler. Cisimleri "kısıtlar".
- **Çarpışma**: İki cismin birbirine değmesi.
- **Eklem (joint)**: Eklemler, iki cismin birbirine bağlı olduğu bir kısıtlamadır. Cisimlerin özgürlük derecelerinden (degrees of freedom) birini veya birden çoğunu kıstlar.
- **Stacking (yığın)**: Basitçe birbirinin üstüne oturtulmuş cisimlerdir. Fizik motorlarının dengesini ve stabilitesini ölçmek için iyi bir benchmarktır.