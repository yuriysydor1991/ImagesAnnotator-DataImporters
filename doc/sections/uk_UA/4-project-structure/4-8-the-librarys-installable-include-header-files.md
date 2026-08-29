## Публічні інтерфейсні файли бібліотеки

Директорія [src/lib/facade/public](/src/lib/facade/public) містить увесь встановлюваний інтерфейс бібліотеки - пʼятнадцять заголовків, усі у просторі імен `ImagesAnnotatorDataImporters014`. Шість із них є власне API і лежать у корені директорії:

| Заголовок | Оголошує |
|---|---|
| [IADataImportersFacade.h](/src/lib/facade/public/IADataImportersFacade.h) | статичні методи `IADataImportersFacade`, точку входу бібліотеки |
| [IADataImportersLib.h](/src/lib/facade/public/IADataImportersLib.h) | `IADataImportersLib::perform_import()` - виконує імпорт, описаний `IADataImportersContext` |
| [IADataImportersContext.h](/src/lib/facade/public/IADataImportersContext.h) | методи доступу до даних `perform_import()` та `import_db()`, разом із вихідними `get_importer()` та `get_imported_records()` |
| [IImporter.h](/src/lib/facade/public/IImporter.h) | `IImporter::import_db()` - окремий імпортер, використаний самостійно |
| [IImageSizeFacility.h](/src/lib/facade/public/IImageSizeFacility.h) | інтерфейс, який реалізує проект-споживач, щоб вимірювати зображення |
| [ImportersAPI.h](/src/lib/facade/public/ImportersAPI.h) | макрос видимості `IADI_API` |

Решта девʼять є нащадками `IADataImportersContext`, специфічними для розкладок - по одному на кожну розкладку набору даних, яку читає бібліотека, - і лежать разом у піддиректорії [contexts](/src/lib/facade/public/contexts), а не поруч із шістьма вище; саме таку розкладку має сусідня бібліотека [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) для власних девʼяти:

| Заголовок | Оголошує |
|---|---|
| [contexts/PlainTxtImportContext.h](/src/lib/facade/public/contexts/PlainTxtImportContext.h) | `IADataImportersContext` розкладки простого тексту |
| [contexts/Yolo4ImportContext.h](/src/lib/facade/public/contexts/Yolo4ImportContext.h) | `IADataImportersContext` розкладки YOLO v4 (darknet) |
| [contexts/UltralyticsDetectImportContext.h](/src/lib/facade/public/contexts/UltralyticsDetectImportContext.h) | `IADataImportersContext` розкладки виявлення Ultralytics YOLO |
| [contexts/UltralyticsObbImportContext.h](/src/lib/facade/public/contexts/UltralyticsObbImportContext.h) | `IADataImportersContext` розкладки орієнтованих обмежувальних рамок Ultralytics YOLO |
| [contexts/UltralyticsSegmentImportContext.h](/src/lib/facade/public/contexts/UltralyticsSegmentImportContext.h) | `IADataImportersContext` розкладки сегментації примірників Ultralytics YOLO |
| [contexts/CocoImportContext.h](/src/lib/facade/public/contexts/CocoImportContext.h) | `IADataImportersContext` розкладки виявлення обʼєктів COCO |
| [contexts/PascalVocImportContext.h](/src/lib/facade/public/contexts/PascalVocImportContext.h) | `IADataImportersContext` розкладки Pascal VOC |
| [contexts/CreateMLImportContext.h](/src/lib/facade/public/contexts/CreateMLImportContext.h) | `IADataImportersContext` розкладки виявлення обʼєктів Create ML |
| [contexts/PyTorchImportContext.h](/src/lib/facade/public/contexts/PyTorchImportContext.h) | `IADataImportersContext` розкладки PyTorch Vision |

Кожен із тих девʼяти дістається шістьох вище через підключення з `../`, і саме це лишає їх розв'язними після встановлення: коренем підключення споживача є директорія, яка містить `ImagesAnnotatorDataImporters-0.14/`, тож звичайне `#include "IADataImportersContext.h"` зсередини `contexts/` шукало б його у тому корені й не знайшло б. Споживача це не стосується: він дістається всіх девʼяти через `IADataImportersFacade.h`, який їх підключає, і лише проект, що виписує заголовок розкладки напряму, називає складову `contexts/` сам.

[src/lib/facade/CMakeLists.txt](/src/lib/facade/CMakeLists.txt) встановлює директорію цілком під `include/${PROJECT_LIBRARY_NAME}`, що для поточної назви і версії дає `include/ImagesAnnotatorDataImporters-0.14/`. І ту піддиректорію, і звичайний корінь підключення експортує ціль бібліотеки, тож споживач може писати будь-яку з двох форм:

```cpp
#include <ImagesAnnotatorDataImporters-0.14/IADataImportersFacade.h>  // рекомендована
#include <IADataImportersFacade.h>                                    // теж працює
```

Форма з префіксом є безпечною - імена на кшталт `IImporter.h` достатньо загальні, щоб зіткнутися у насиченому шляху підключення, а префікс закріплює версію бібліотеки, з якою розмовляє одиниця трансляції.

### Чому видно лише ці заголовки

Бібліотека будується з `CXX_VISIBILITY_PRESET hidden`, тож спільний обʼєкт залишають лише сутності, позначені `IADI_API`. Це вимога коректності, а не оптимізація розміру. Бібліотека ImagesAnnotatorDataDrivers, з якою ця лінкується, походить із того самого шаблону проекту і експортує власні символи `default_logger::DefaultLogger` та `project_decls`. Якби обидва набори експортувалися, динамічний компонувальник звʼязав би виклики однієї бібліотеки з визначеннями іншої.

Сама лише видимість лишає одну дірку. Інстанціація `std::make_shared` називає свій клас у власному спотвореному імені й лишається слабкою та експортованою, якою б не була видимість, тож простір імен реалізації тут - `iadi0impl`, а не `lib0impl`, який використовує шаблон проекту, а разом із ним і бібліотека драйверів даних. Два класи `LibFactory` не поділяють навіть розкладки vtable, і компонувальник інакше міг би звʼязати `std::make_shared<lib0impl::LibFactory>()` однієї бібліотеки з визначенням іншої.

Отже, новий публічний клас належить до [src/lib/facade/public](/src/lib/facade/public) - новий контекст розкладки набору даних до її піддиректорії [contexts](/src/lib/facade/public/contexts), звідки він дістається заголовків над собою через `../`, - і має бути позначений `IADI_API`; кожен інший компонент під [src](/src) лишається приватним для спільного обʼєкта, і дістатися до нього можна лише через абстрактні інтерфейси вище.

### Встановлюваний CMake-пакунок

Разом із бінарником і заголовками збірка встановлює CMake-пакунок, згенерований [src/lib/cmake/lib-cmake-module-gen.cmake](/src/lib/cmake/lib-cmake-module-gen.cmake) із [src/lib/cmake/ImportersLibraryConfig.cmake.in](/src/lib/cmake/ImportersLibraryConfig.cmake.in). Він потрапляє до `<libdir>/cmake/ImagesAnnotatorDataImporters-0.14/` і складається з трьох файлів: експортовані цілі, `ImagesAnnotatorDataImporters-0.14ConfigVersion.cmake`, записаний `write_basic_package_version_file()` із сумісністю `SameMajorVersion`, та `ImagesAnnotatorDataImporters-0.14Config.cmake`, який виконує `find_dependency()` пакунка драйверів даних перед підключенням цілей - публічні заголовки називають його типи бази даних та записів, тож його треба розвʼязати першим.

Тому проекту нижче за течією не потрібно нічого більшого за:

```cmake
find_package(ImagesAnnotatorDataImporters-0.14 0.14 REQUIRED)

target_link_libraries(
  your_target
  PRIVATE ImagesAnnotatorDataImporters-0.14::ImagesAnnotatorDataImporters-0.14
)
```

Див. [Встановлення](/doc/sections/uk_UA/7-installing/7-installing.md) та [Використання бібліотеки у власному проекті](/doc/sections/uk_UA/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md).
