# Використання бібліотеки у власному проекті

Бібліотека призначена для споживання іншими проектами: вона не створює власного виконуваного файлу, лише спільний обʼєкт `libImagesAnnotatorDataImporters-0.11.so` разом зі своїми встановлюваними заголовками і CMake-пакунком. Дана секція описує, що має зробити проект нижче за течією, щоб побудуватись проти неї.

## Що потрібно встановити спершу

Перед конфігуруванням проекту-споживача мають бути встановлені дві речі:

- бібліотека [ImagesAnnotatorDataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git), яка оголошує базу даних анотацій і записи зображень, що їх заповнює ця бібліотека - переглянь підсекцію [Залежність від бібліотеки драйверів даних](/doc/sections/uk_UA/5-project-build/5-36-the-data-drivers-dependency.md);
- сама ця бібліотека, побудована і встановлена як описано у секції [Встановлення](/doc/sections/uk_UA/7-installing/7-installing.md).

Встановлення розміщує в обраному префіксі наступне:

```
<prefix>/include/ImagesAnnotatorDataImporters-0.11/     публічні заголовки
<prefix>/lib/libImagesAnnotatorDataImporters-0.11.so    спільний обʼєкт, soname .so.0
<prefix>/lib/cmake/ImagesAnnotatorDataImporters-0.11/   файли CMake-пакунка
```

## Пошук пакунка за допомогою CMake

```cmake
cmake_minimum_required(VERSION 3.13)

project(MyImportingTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ImagesAnnotatorDataImporters-0.11 0.11 REQUIRED)

add_executable(my-importing-tool main.cpp)

target_link_libraries(
  my-importing-tool
  PRIVATE ImagesAnnotatorDataImporters-0.11::ImagesAnnotatorDataImporters-0.11
)
```

І імʼя пакунка, і імпортована ціль несуть мажорну та мінорну версії бібліотеки, тож майбутній випуск можна встановити паралельно з цим. Запит версії звіряється зі встановленим файлом `ImagesAnnotatorDataImporters-0.11ConfigVersion.cmake` за правилом сумісності `SameMajorVersion`.

Встановлений файл конфігурації пакунка, згенерований з [src/lib/cmake/ImportersLibraryConfig.cmake.in](/src/lib/cmake/ImportersLibraryConfig.cmake.in), викликає `find_dependency()` для пакунка драйверів даних перед тим як прочитати експортовані цілі. Бібліотека драйверів даних лінкується як `PUBLIC`, оскільки встановлювані заголовки цієї бібліотеки згадують її типи бази даних та записів, тож лінкування наведеної вище цілі приносить із собою і шлях до заголовків, і спільний обʼєкт драйверів даних. Окремий `find_package()` для драйверів даних у споживачі не потрібен, хоча його виклик і не шкодить.

Стандарт C++ `17` не є опціональним: публічні заголовки і згадані у них записи компілюються як C++17.

## Як вказати CMake префікси встановлення

Коли будь-яку з бібліотек було встановлено поза типовими системними префіксами, вкажи їхні префікси через `CMAKE_PREFIX_PATH` під час конфігурування свого проекту:

```
cmake -S . -B build -DCMAKE_PREFIX_PATH="/opt/iadd;/opt/iadi"
```

Ту саму змінну потребує і ця бібліотека під час власного конфігурування, щоб знайти драйвери даних.

## Підключення заголовків

Експортуються два корені підключення, тож компілюються обидва написання:

```cpp
#include <ImagesAnnotatorDataImporters-0.11/LibraryFacade.h>  // рекомендовано
#include <LibraryFacade.h>                                    // теж працює
```

Надавай перевагу написанню з префіксом. Імена заголовків на кшталт `LibraryFacade.h`, `LibraryContext.h` і `ILib.h` достатньо загальні щоб зіткнутись у насиченому шляху підключення - бібліотека драйверів даних встановлює заголовки точно з такими іменами, і за наявності обох простих коренів підключення коротке написання підхоплює той із двох, який компілятор побачить першим.

`LibraryFacade.h` підключає кожен інший публічний заголовок бібліотеки, тож зазвичай це єдиний, який згадує споживач.

## Скорочення просторів імен

Інтерфейсні простори імен обох бібліотек несуть номери своїх мажорної і мінорної версій. Признач їм скорочення один раз - і підняття версії залишиться зміною в один рядок:

```cpp
namespace iadd = ImagesAnnotatorDataDrivers011;
namespace iadi = ImagesAnnotatorDataImporters011;
```

## Що має надати твій проект

- **Базу даних.** `LibraryContext::set_db()` приймає `ImagesAnnotatorDataDrivers011::IAnnotationsDBPtr` - призначення, до якого зливаються відновлені записи. Порожня база походить з `iadd::LibraryFacade::create_annotations_db()`; передай натомість базу з `iadd::LibraryFacade::open_annotations_db("project.json")`, щоб імпортувати **до** наявного проекту, і імпорт долучить зображення, яких той проект ще не містить. У будь-якому разі саме база даних зберігає результат через власний `store_db()`.
- **Директорію-джерело.** `LibraryContext::set_import_path()` має називати наявну директорію з набором даних тієї розкладки, яку представляє контекст. Усередині неї нічого не записується: імпорт є проходом лише на читання.
- **Читач розмірів зображень, для двох із трьох розкладок.** Розкладка YOLO v4 зберігає свої рамки поділеними на розмір їхнього зображення, а PyTorch Vision зберігає анотацію як саме обрізане зображення, тож жодну з них неможливо прочитати назад без вимірювання картинок - а сама бібліотека не декодує жодного формату зображень. Реалізуй `IImageSizeFacility` над тим набором засобів роботи із зображеннями, який твій проект уже лінкує, і передай примірник через `LibraryContext::set_image_sizer()`. Бібліотека, зібрана з OpenCV, несе власний читач і заповнює ним порожнє місце, тож це обовʼязково лише для споживача збірки без OpenCV або для того, хто хоче власного вимірювання. Підсекція [API імпортерів наборів даних](/doc/sections/uk_UA/4-project-structure/4-9-the-dataset-importers-api.md) містить начерк реалізації, а [Вмикання читача розмірів зображень на OpenCV](/doc/sections/uk_UA/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md) описує вбудований. Розкладці простого тексту читач не потрібен зовсім.

## Мінімальний споживач

Наведений нижче `main.cpp` читає назад набір даних простого тексту і зберігає його як файл проекту через одноразову точку входу `ILib::perform_import`:

```cpp
#include <ImagesAnnotatorDataDrivers-0.11/LibraryFacade.h>
#include <ImagesAnnotatorDataImporters-0.11/LibraryFacade.h>

#include <iostream>
#include <memory>

namespace iadd = ImagesAnnotatorDataDrivers011;
namespace iadi = ImagesAnnotatorDataImporters011;

int main()
{
  auto db = iadd::LibraryFacade::create_annotations_db();

  if (db == nullptr) {
    std::cerr << "fail to create the annotations database\n";
    return 1;
  }

  auto ctx = iadi::LibraryFacade::create_plain_txt_library_context();

  ctx->set_import_path("plain-dataset");
  ctx->set_db(db);

  auto lib = iadi::LibraryFacade::create_library(ctx);

  if (lib == nullptr || !lib->perform_import(ctx)) {
    std::cerr << "the import has failed\n";
    return 1;
  }

  if (!db->store_db("project.json")) {
    std::cerr << "fail to store the project file\n";
    return 1;
  }

  std::cout << "imported " << ctx->get_imported_records()
            << " image records with the library version "
            << iadi::LibraryFacade::library_version() << '\n';

  return 0;
}
```

Створи натомість `iadi::Yolo4ImportLibraryContext` чи `iadi::PyTorchImportLibraryContext`, щоб отримати одну з двох інших розкладок, описаних у підсекції [Розкладки наборів даних, які читаються](/doc/sections/uk_UA/4-project-structure/4-10-the-read-dataset-layouts.md) - обом потрібен читач розмірів зображень вище. Побудова імпортера напряму за допомогою `iadi::LibraryFacade::create_importer()` дає той самий результат із дрібнішим контролем - переглянь підсекцію [API імпортерів наборів даних](/doc/sections/uk_UA/4-project-structure/4-9-the-dataset-importers-api.md).

## Запуск результату

Обидва спільні обʼєкти мають бути досяжні для динамічного лінкера під час виконання. Коли їх було встановлено у префікс, який система не переглядає за замовчуванням, або зареєструй той префікс (наприклад додавши його до `/etc/ld.so.conf.d/` і виконавши `ldconfig`), або вкажи його у середовищі:

```
LD_LIBRARY_PATH=/opt/iadi/lib:/opt/iadd/lib ./my-importing-tool
```

`iadi::LibraryFacade::library_version()` повідомляє версію бінарника, який було справді завантажено, і це найшвидший спосіб зʼясувати, яка саме з кількох встановлених копій дісталась твоїй програмі.

## Опрацьований приклад усередині цього проекту

Компонентний тест `CTEST_Importers`, [src/importers/tests/component/Importers/CTEST_Importers.cpp](/src/importers/tests/component/Importers/CTEST_Importers.cpp), лінкує згенеровану спільну бібліотеку і керує нею винятково через встановлювані заголовки, точно як це робить проект нижче за течією - включно з невеликою реалізацією `IImageSizeFacility`. Увімкни його CMake-опцією `ENABLE_COMPONENT_TESTS`, описаною у секції [Побудова проекту](/doc/sections/uk_UA/5-project-build/5-project-build.md).
