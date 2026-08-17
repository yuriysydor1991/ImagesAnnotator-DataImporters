## Публічні інтерфейсні файли бібліотеки

Директорія [src/lib/facade/public](/src/lib/facade/public) містить увесь встановлюваний інтерфейс бібліотеки - девʼять заголовків, усі у просторі імен `ImagesAnnotatorDataImporters011`:

| Заголовок | Оголошує |
|---|---|
| [LibraryFacade.h](/src/lib/facade/public/LibraryFacade.h) | статичні методи `LibraryFacade`, точку входу бібліотеки |
| [ILib.h](/src/lib/facade/public/ILib.h) | `ILib::perform_import()` - виконує імпорт, описаний `LibraryContext` |
| [LibraryContext.h](/src/lib/facade/public/LibraryContext.h) | методи доступу до даних `perform_import()` та `import_db()`, разом із вихідними `get_importer()` та `get_imported_records()` |
| [PlainTxtImportLibraryContext.h](/src/lib/facade/public/PlainTxtImportLibraryContext.h) | `LibraryContext` розкладки простого тексту |
| [Yolo4ImportLibraryContext.h](/src/lib/facade/public/Yolo4ImportLibraryContext.h) | `LibraryContext` розкладки YOLO v4 (darknet) |
| [PyTorchImportLibraryContext.h](/src/lib/facade/public/PyTorchImportLibraryContext.h) | `LibraryContext` розкладки PyTorch Vision |
| [IImporter.h](/src/lib/facade/public/IImporter.h) | `IImporter::import_db()` - окремий імпортер, використаний самостійно |
| [IImageSizeFacility.h](/src/lib/facade/public/IImageSizeFacility.h) | інтерфейс, який реалізує проект-споживач, щоб вимірювати зображення |
| [ImportersAPI.h](/src/lib/facade/public/ImportersAPI.h) | макрос видимості `IADI_API` |

[src/lib/facade/CMakeLists.txt](/src/lib/facade/CMakeLists.txt) встановлює директорію цілком під `include/${PROJECT_LIBRARY_NAME}`, що для поточної назви і версії дає `include/ImagesAnnotatorDataImporters-0.11/`. І ту піддиректорію, і звичайний корінь підключення експортує ціль бібліотеки, тож споживач може писати будь-яку з двох форм:

```cpp
#include <ImagesAnnotatorDataImporters-0.11/LibraryFacade.h>  // рекомендована
#include <LibraryFacade.h>                                    // теж працює
```

Форма з префіксом є безпечною - імена на кшталт `IImporter.h` достатньо загальні, щоб зіткнутися у насиченому шляху підключення, а бібліотеки драйверів даних та експортерів цієї ж родини встановлюють власні `LibraryFacade.h`.

### Чому видно лише ці заголовки

Бібліотека будується з `CXX_VISIBILITY_PRESET hidden`, тож спільний обʼєкт залишають лише сутності, позначені `IADI_API`. Це вимога коректності, а не оптимізація розміру. Бібліотека ImagesAnnotatorDataDrivers, з якою ця лінкується, походить із того самого шаблону проекту і експортує власні символи `default_logger::DefaultLogger` та `project_decls`. Якби обидва набори експортувалися, динамічний компонувальник звʼязав би виклики однієї бібліотеки з визначеннями іншої.

Сама лише видимість лишає одну дірку. Інстанціація `std::make_shared` називає свій клас у власному спотвореному імені й лишається слабкою та експортованою, якою б не була видимість, тож простір імен реалізації тут - `iadi0impl`, а не `lib0impl`, який використовує шаблон проекту, а разом із ним і бібліотека драйверів даних. Два класи `LibFactory` не поділяють навіть розкладки vtable, і компонувальник інакше міг би звʼязати `std::make_shared<lib0impl::LibFactory>()` однієї бібліотеки з визначенням іншої.

Отже, новий публічний клас належить до [src/lib/facade/public](/src/lib/facade/public) і має бути позначений `IADI_API`; кожен інший компонент під [src](/src) лишається приватним для спільного обʼєкта, і дістатися до нього можна лише через абстрактні інтерфейси вище.

### Встановлюваний CMake-пакунок

Разом із бінарником і заголовками збірка встановлює CMake-пакунок, згенерований [src/lib/cmake/lib-cmake-module-gen.cmake](/src/lib/cmake/lib-cmake-module-gen.cmake) із [src/lib/cmake/ImportersLibraryConfig.cmake.in](/src/lib/cmake/ImportersLibraryConfig.cmake.in). Він потрапляє до `<libdir>/cmake/ImagesAnnotatorDataImporters-0.11/` і складається з трьох файлів: експортовані цілі, `ImagesAnnotatorDataImporters-0.11ConfigVersion.cmake`, записаний `write_basic_package_version_file()` із сумісністю `SameMajorVersion`, та `ImagesAnnotatorDataImporters-0.11Config.cmake`, який виконує `find_dependency()` пакунка драйверів даних перед підключенням цілей - публічні заголовки називають його типи бази даних та записів, тож його треба розвʼязати першим.

Тому проекту нижче за течією не потрібно нічого більшого за:

```cmake
find_package(ImagesAnnotatorDataImporters-0.11 0.11 REQUIRED)

target_link_libraries(
  your_target
  PRIVATE ImagesAnnotatorDataImporters-0.11::ImagesAnnotatorDataImporters-0.11
)
```

Див. [Встановлення](/doc/sections/uk_UA/7-installing/7-installing.md) та [Використання бібліотеки у власному проекті](/doc/sections/uk_UA/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md).
