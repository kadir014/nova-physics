<p align="center"><img src="https://raw.githubusercontent.com/kadir014/kadir014.github.io/master/assets/novaphysics.png" width=340></p>
<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg">
  <img src="https://img.shields.io/badge/version-1.0.0-yellow">
  <a href="https://app.codacy.com/gh/kadir014/nova-physics/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade"><img src="https://app.codacy.com/project/badge/Grade/9556f3db17d54b288557d3b2e9dbf366"></a>
</p>
<p align="center">
Nova Physics, oyun geliştirme düşünülerek tasarlanan hafif ve kullanımı kolay bir 2B fizik motorudur.
<br>
<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/introgif.gif" width=400>
</p>
<p align="center">
<sup><i>Bu sayfayı ayrıca şu dillerde okuyabilirsin</i></sub>
<br>
<a href="#"><img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/docs/_static/flag_uk.png" width=30 alt="EN"></a>
<a href="./docs/translations/README_tr.md"><img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/docs/_static/flag_tr.png" width=30></a>
</p>



# Özellikler
- Basit ve kullanıcı-dostu arayüz
- Dış bağımlılığı olmayan portable codebase
- Katı cisim dinamiği
- Süreksiz çarpışma tespiti
  - Daire şekli
  - Dışbükey (konveks) çokgen şekli
  - Şekilleri diğer şekillere karşı test etme
  - Şekilleri noktaya karşı test etme
  - Ray casting
- Şekiller arasında one-shot contact manifoldu oluşturma
- Cisim başına birden fazla şekil
- Broadphase stratejileri
  - Bruteforce
  - Bounding volume hierarchy tree
- Materyal özellikleri (sürtünme, sekme (elastiklik) ve özkütle)
- Cisimler arası kısıtlamalar (constraints)
  - Uzaklık kısıtlaması - yay gibi de davranabilir
  - Menşete (hinge) kısıtlaması
  - Spline yol kısıtlaması
- [Erin Catto'nun](https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf) 'sequential impulses' algoritması
- Gayet iyi stacking stabilitesi
- Semi-implicit (symplectic) Euler integrasyonu
- Maskeler ve gruplama ile çarpışma filtreleme
- Built-in profiler
- Opsiyonel double-precision modu
- SDL2 & OpenGL kullanan çeşitli etkileşimli örnek demoları


# Yükleme & Derleme
Kütüphane sadece C99 standardı ve C STL'ini kullanır.

Daha fazla talimat için [buraya bakın.](BUILDING.md)



# Örnekler

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/examples/assets/examplegif.gif" width=400>

Örnek demoları [examples](https://github.com/kadir014/nova-physics/blob/main/examples/) klasöründe bulunuyor, build sisteminde demoları buildleme opsiyonunu etkinleştirin (eğer zaten değilse).



# Dökümantasyon
API referansını da içeren dökümantasyona [buradan](https://nova-physics.rtfd.io) erişebilirsiniz.

Eğer daha yeni başlıyorsanız, [giriş sayfasını](https://nova-physics.readthedocs.io/en/latest/getting_started/index.html) kullanabilirsiniz.



# Kaynaklar & Referanslar
Nova benim için bir tutku ve öğrenme projesi, ve aşağıdakiler şuana kadar bana yardımcı olan bir sürü harika kaynaktan bazıları.
- **Erin Catto**, [GDC Presentations](https://box2d.org/publications/) ve [Box2D](https://github.com/erincatto/box2c)
- **Chris Hecker**, [Rigid Body Dynamics](https://chrishecker.com/Rigid_Body_Dynamics)
- **Ian Millington**, [Game Physics Engine Development](https://www.r-5.org/files/books/computers/algo-list/realtime-3d/Ian_Millington-Game_Physics_Engine_Development-EN.pdf)
- **Christer Ericson**, [Real-Time Collision Detection](https://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf)
- **Dirk Gregorius**, [Robust Contact Creation for Physics Simulations](http://media.steampowered.com/apps/valve/2015/DirkGregorius_Contacts.pdf)
- **Randy Gaul**, [Game Physics Articles](https://tutsplus.com/authors/randy-gaul)
- **Allen Chou**, [Physics Blogs](https://allenchou.net/category/physics/)
- **Jacco Bikker**, [How to build a BVH](https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/)
- **Marjin Tamis** & **Giuseppe Maggiore**, [Constraint Based Physics Solver](http://mft-spirit.nl/files/MTamis_ConstraintBasedPhysicsSolver.pdf)
- **Micheal Manzke**, [Multiple Contact Resolution](https://www.scss.tcd.ie/~manzkem/CS7057/cs7057-1516-10-MultipleContacts-mm.pdf)



# License
[MIT](LICENSE) © Kadir Aksoy

Nova Physics her zaman ücretsiz ve açık-kaynak olacaktır. Fakat [bağışlara](https://www.buymeacoffee.com/kadir014) her zaman açığız!