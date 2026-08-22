# Структура проекту

Репозиторій будує єдину спільну бібліотеку - `libImagesAnnotatorDataImporters-0.11.so`, - яка перетворює набір даних для навчання на диску на базу даних анотацій ImagesAnnotator. Виконуваного файлу немає: окрім самої бібліотеки збірка створює лише тестові бінарники.

Бібліотека є оберненням спорідненої [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git), щоб набір даних, створений будь-яким інструментом - і передусім тією бібліотекою експортерів, - можна було прочитати назад до проекту анотацій, і щоб застосунок [ImagesAnnotator](https://github.com/yuriysydor1991/ImagesAnnotator.git) та будь-який інший інструмент поділяли одну реалізацію того читання. Самі записи анотацій тут не визначено: їх будують через бібліотеку [ImagesAnnotator-DataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git), яка раніше забрала базу даних анотацій із того самого застосунку.

Розкладка верхнього рівня:

| Шлях | Вміст |
|---|---|
| [src/lib/facade/public](/src/lib/facade/public) | встановлювані публічні заголовки - увесь інтерфейс `ImagesAnnotatorDataImporters011` |
| [src/lib/facade/LibraryFacade.cpp](/src/lib/facade/LibraryFacade.cpp) | реалізація фасаду, точка входу бібліотеки |
| [src/lib/libmain](/src/lib/libmain) | `LibMain.cpp` та `LibFactory.cpp` - ядро реалізації |
| [src/lib/cmake](/src/lib/cmake) | генерація встановлюваного CMake-пакунка бібліотеки |
| [src/importers](/src/importers) | по одній піддиректорії на родину розкладок наборів даних - `PlainTxt`, `Yolo4`, `Ultralytics`, `Coco`, `PascalVoc`, `CreateML` та `PyTorch`, - кожна несе власні реалізації `IImporter`, свій `CMakeLists.txt` і свої модульні тести, плюс спільні для всіх `ImportersAliases.h` та базу `Folder2DBImporter`, а також `JsonDescriptor2DBImporter`, спільний для двох розкладок, описаних одним документом JSON |
| [src/parsers](/src/parsers) | читачі документів JSON та XML, якими читаються дескрипторні розкладки, по одному підкомпоненту на кожного, і кожен випробовується на власних документах |
| [src/sizers](/src/sizers) | `create_builtin_image_sizer()` і необовʼязковий OpenCV-`IImageSizeFacility` за ним |
| [src/helpers](/src/helpers) | `IHelper.h`, `TypeHelper.h`, `StringHelper.h` та `Utf8Helper.h`, усі складаються лише із заголовків |
| [src/log](/src/log) | макроси журналювання і простий логер, вкомпільований у бібліотеку |
| [cmake](/cmake) | система збірки: опції компіляції та вмикачі залежностей |
| [doc](/doc) | ця документація і діаграми проекту |
| [misc](/misc) | конвеєр Jenkins і файли Docker |

Реалізація живе у просторах імен `iadi0impl`, `iannotator::importers`, `iannotator::importers::helpers`, `iannotator::importers::parsers` та `iannotator::importers::sizers`. Жоден із них не встановлюється - проект-споживач бачить лише `ImagesAnnotatorDataImporters011` (рекомендовано аліас `iadi`).

1. [Діаграми проекту](/doc/sections/uk_UA/4-project-structure/4-0-project-diagrams.md)
1. [Де живе реалізація імпортерів](/doc/sections/uk_UA/4-project-structure/4-1-implement-code-straight-away.md)
1. [API імпортерів наборів даних](/doc/sections/uk_UA/4-project-structure/4-9-the-dataset-importers-api.md)
1. [Розкладки згенерованих наборів даних](/doc/sections/uk_UA/4-project-structure/4-10-the-read-dataset-layouts.md)
1. [Публічні інтерфейсні файли бібліотеки](/doc/sections/uk_UA/4-project-structure/4-8-the-librarys-installable-include-header-files.md)
1. [Версіювання і інші параметри проекту](/doc/sections/uk_UA/4-project-structure/4-3-version-tracking-and-other-project-parameters.md)
1. [Тести проекту](/doc/sections/uk_UA/4-project-structure/4-4-project-tests.md)
    1. [Фреймворк тестів Google Test](/doc/sections/uk_UA/4-project-structure/4-4-1-google-test.md)
