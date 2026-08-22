**Бібліотека імпортерів наборів даних анотацій проекту ImagesAnnotator**

# Що це таке

`ImagesAnnotatorDataImporters` - це спільна бібліотека на C++17, яка читає розкладки тренувальних наборів даних, що їх використовують фреймворки машинного навчання, назад до анотацій застосунку [ImagesAnnotator](https://github.com/yuriysydor1991/ImagesAnnotator.git) - анотованих зображень з намальованими поверх них іменованими прямокутниками.

Вона є точним відповідником спорідненої бібліотеки [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git): що та записує, це читає. Експортований набір даних можна повернути до проекту, набір даних, створений якимось іншим інструментом, можна вперше внести до проекту, а застосунок ImagesAnnotator разом із **будь-яким іншим інструментом** поділяють єдину реалізацію того читання.

Усе, чого торкається проект-споживач, приховано за абстрактними інтерфейсами заголовків [src/lib/facade/public](/src/lib/facade/public), тож ані клас реалізації, ані жодна зі сторонніх залежностей бібліотеки не просочуються у код нижче за течією.

Більше за посиланням [kytok.org.ua](http://www.kytok.org.ua/)

💵 Підтримай проект за посиланням [http://kytok.org.ua/page/pozertvy](http://kytok.org.ua/page/pozertvy)

# Можливості

- **Девʼять розкладок наборів даних до однієї бази даних** - обираються створеним нащадком `LibraryContext` і реалізовані окремим класом-імпортером кожна, і це саме ті девʼять, які записує споріднена бібліотека експортерів:
  - `PlainTxtImportLibraryContext` - по одному файлу `<імʼя-анотації>.txt` на кожне імʼя анотації, де кожен рядок називає зображення і його прямокутники у пікселях того зображення;
  - `Yolo4ImportLibraryContext` - ціла тренувальна директорія darknet для детектора YOLO v4: імена класів із `data/obj.names`, перелік зображень із `data/train.txt` і нормалізований файл міток `.txt` кожного зображення, причому дескриптор `data/obj.data` враховується, коли він є, тож директорію, розкладену якимось іншим інструментом, читають так, як той її назвав;
  - `UltralyticsDetectImportLibraryContext`, `UltralyticsObbImportLibraryContext` та `UltralyticsSegmentImportLibraryContext` - розкладка, з якої тренується кожен випуск Ultralytics: дескриптор `data.yaml`, що називає класи та директорії зображень, і по одному файлу міток на зображення під `labels/` поруч із ними. Три розкладки різняться лише тим рядком міток - рамка, чотири її кути чи багатокутник, що окреслює обʼєкт, - і дві останні повертаються прямокутником, який містить намальоване;
  - `CocoImportLibraryContext` - набір даних виявлення обʼєктів COCO: єдиний дескриптор JSON, що називає зображення, їхній розмір, категорії та рамку `[x, y, ширина, висота]` кожної анотації, тобто ті самі чотири числа, які тримає прямокутник бази даних;
  - `PascalVocImportLibraryContext` - форма devkit Pascal VOC: по одному дескриптору XML на зображення під `Annotations/` або поруч із самими зображеннями, як зберігає свою роботу [LabelImg](https://github.com/HumanSignal/labelImg);
  - `CreateMLImportLibraryContext` - набір даних виявлення обʼєктів Create ML: пласка директорія зображень і єдиний дескриптор JSON поруч із ними, чиї рамки записано за центром і які повертаються на той кут, з якого їх намалювали;
  - `PyTorchImportLibraryContext` - класифікаційна розкладка, яку читає набір даних `ImageFolder` з PyTorch Vision: одна директорія на імʼя анотації з обрізаними зображеннями, кожне з яких повертається як запис зображення, чий єдиний прямокутник вкриває його цілком.
- **Одноразова точка входу** - заповни нащадка `LibraryContext` потрібної розкладки директорією-джерелом і базою даних-призначенням, і `ILib::perform_import()` побудує потрібний імпортер та запустить його. `LibraryFacade::create_importer()` дає той самий результат із дрібнішим контролем.
- **Нічого не перезаписується** - імпорт є проходом лише на читання по своїй директорії. Відновлені записи вказують на файли зображень там, де ті вже лежать, і зливаються до бази даних через `IAnnotationsDB::add_images_db()`, який зберігає зображення, що база вже містить. Тож набір даних можна імпортувати до проекту, що редагується, а імпорт одного й того самого набору двічі додає його зображення один раз.
- **Стійкість до неповного набору даних** - зіпсований рядок, файл зображення, який набір даних називає, але не містить, мітка з невідомим класом, картинка, яку не вдалося виміряти: кожне потрапляє до журналу і пропускається, а сам прохід імпорту триває далі.
- **Жодного власного кодека зображень** - розкладки, які не зберігають своїх прямокутників у пікселях їхнього зображення, просять проект-споживач виміряти картинки через інтерфейс `IImageSizeFacility`, за допомогою тих засобів роботи із зображеннями, які той проект уже лінкує. Збірка, яка знайшла OpenCV, несе такий читач сама, тож споживач без власних засобів роботи із зображеннями все одно отримує ті імпорти.
- **Версійований встановлюваний інтерфейс** - простір імен, бінарник, директорія заголовків і CMake-пакунок усі несуть пару мажорної й мінорної версій `0.11`, тож два мінорні випуски встановлюються паралельно.

# Приклад використання

Бібліотека споживається через CMake-пакунок. Наведену нижче програму було скомпільовано, злінковано і запущено проти встановленої бібліотеки:

```cmake
cmake_minimum_required(VERSION 3.13)
project(MyTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ImagesAnnotatorDataImporters-0.11 REQUIRED)

add_executable(mytool main.cpp)
target_link_libraries(mytool ImagesAnnotatorDataImporters-0.11::ImagesAnnotatorDataImporters-0.11)
```

```cpp
#include <ImagesAnnotatorDataDrivers-0.11/LibraryFacade.h>
#include <ImagesAnnotatorDataImporters-0.11/LibraryFacade.h>

#include <iostream>
#include <memory>

namespace iadd = ImagesAnnotatorDataDrivers011;
namespace iadi = ImagesAnnotatorDataImporters011;

int main(int argc, char** argv)
{
  if (argc < 3) { return 1; }

  auto db = iadd::LibraryFacade::create_annotations_db();

  if (db == nullptr) { return 1; }

  auto ctx = iadi::LibraryFacade::create_yolo4_library_context();

  ctx->set_import_path(argv[1]);
  ctx->set_db(db);

  auto lib = iadi::LibraryFacade::create_default_lib();

  if (lib == nullptr || !lib->perform_import(ctx)) {
    std::cerr << "the import has failed\n";
    return 1;
  }

  if (!db->store_db(argv[2])) {
    std::cerr << "fail to store the project file\n";
    return 1;
  }

  std::cout << "imported " << ctx->get_imported_records() << " records with "
            << iadi::LibraryFacade::library_version() << "\n";

  return 0;
}
```

Імʼя простору імен `ImagesAnnotatorDataImporters011` навмисно несе номери мажорної й мінорної версій бібліотеки: дві версії бібліотеки можуть співіснувати всередині однієї одиниці трансляції без жодного зіткнення символів. Признач йому скорочення, як показано вище, і підняття версії залишиться зміною в один рядок на твоєму боці.

Для встановленого споживача працюють обидва написання - і `#include <ImagesAnnotatorDataImporters-0.11/LibraryFacade.h>`, і просте `#include <LibraryFacade.h>`, оскільки бібліотека експортує корінь підключення разом зі своєю версійованою субдиректорією. Рекомендованим є написання з префіксом: імена заголовків на кшталт `LibraryFacade.h`, `LibraryContext.h` чи `ILib.h` достатньо загальні щоб зіткнутись у насиченому шляху підключення - бібліотеки драйверів даних та експортерів цієї ж родини встановлюють заголовки точно з такими іменами.

Програма вище читає розкладку YOLO v4, чиї прямокутники зберігаються поділеними на розмір їхнього зображення, тож їй потрібна бібліотека, зібрана з OpenCV. Інакше передай власний `IImageSizeFacility` до `ctx->set_image_sizer()` або почни з розкладки простого тексту, якій він не потрібен.

Більше про API і про набори даних, які читаються - у секціях документації [API імпортерів наборів даних](/doc/sections/uk_UA/4-project-structure/4-9-the-dataset-importers-api.md), [розкладки наборів даних, які читаються](/doc/sections/uk_UA/4-project-structure/4-10-the-read-dataset-layouts.md) і [використання бібліотеки у власному проекті](/doc/sections/uk_UA/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md).

# Залежності

| CMake-опція | Бібліотека | Навіщо вона потрібна |
| --- | --- | --- |
| (завжди увімкнено) | [ImagesAnnotatorDataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git) | вона оголошує базу даних анотацій, яку заповнюють імпортери, і записи зображень, які вони будують |
| `ENABLE_OPENCV` | [OpenCV](https://opencv.org/) | необовʼязково: з ним бібліотека несе власний читач розмірів зображень, тож споживач без власних засобів роботи із зображеннями все одно отримує розкладки, яким той читач потрібен |

Бібліотека драйверів даних є **обовʼязковою**. Вона має бути встановлена заздалегідь, знаходиться через `find_package(ImagesAnnotatorDataDrivers-0.11 REQUIRED CONFIG)` і лінкується **публічно**, оскільки встановлювані заголовки цієї бібліотеки згадують її тип бази даних. Вкажи конфігуруванню її префікс встановлення за допомогою `-DCMAKE_PREFIX_PATH=<prefix>`, коли вона не міститься у типовому системному префіксі. Секція [залежність від бібліотеки драйверів даних](/doc/sections/uk_UA/5-project-build/5-36-the-data-drivers-dependency.md) розкриває це повністю, включно з іменем пакунка.

OpenCV є **необовʼязковим**, і `ENABLE_OPENCV=ON` означає *перевірити*, а не *вимагати*: система без нього конфігурується і збирається так само, лише без вбудованого читача розмірів зображень. Він лінкується **приватно**: жоден публічний заголовок не відкриває типів OpenCV, тож проекту-споживачу не потрібен власний OpenCV. Див. секцію [вмикання читача розмірів зображень на OpenCV](/doc/sections/uk_UA/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md).

Компонент журналювання компілюється прямо у спільну бібліотеку, тож споживач не має надавати жодної реалізації журналювання.

# Побудова і тестування

Звичайна побудова, проти встановлення драйверів даних у `$HOME/iadd-install`:

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/iadd-install
cmake --build build -j$(nproc)
```

Тести вимкнено за замовчуванням. Щоб побудувати і запустити їх:

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/iadd-install \
  -DENABLE_UNIT_TESTS=ON -DENABLE_COMPONENT_TESTS=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

З обома увімкненими опціями набір містить 90 тестових випадків. Цілі `ENABLE_UNIT_TESTS` компілюються прямо із сирців проти gmock-замінників з [src/tests/mocks](/src/tests/mocks), тоді як `CTEST_Importers` з `ENABLE_COMPONENT_TESTS` лінкує справжню спільну бібліотеку і керує нею винятково через публічні заголовки - точно як це робить проект нижче за течією.

Встановлення - звичайне `sudo cmake --install build`, докладно описане у секції [встановлення](/doc/sections/uk_UA/7-installing/7-installing.md).

# Звідки походить код

Репозиторій почався як гілка `lib` проекту [cpp-app-template](https://github.com/yuriysydor1991/cpp-app-template) і був наповнений імпортерами, написаними під ті розкладки, які записує споріднена бібліотека [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git), щоб дві половини пари замикали коло. Застосунок [ImagesAnnotator](https://github.com/yuriysydor1991/ImagesAnnotator.git) має споживати цю бібліотеку, а не вирощувати власний імпорт.

Сама база даних анотацій - розбирач файлу проекту, серіалізатор і правила злиття - **не** є частиною цієї бібліотеки. Вона живе у спорідненому проекті [ImagesAnnotator-DataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git), через який ця будує свої записи.

Переглянь директорію `doc` щодо можливих перекладів поточного md-документа:
- `en_US` за відносною адресою [README.md](/README.md)

# Зміст документації

**Даний документ у процесі покращення**

1. [Вимоги](/doc/sections/uk_UA/3-requirements/3-requirements.md)
    1. [Обов'язкові інструменти для ОС на базі GNU/Лінукс](/doc/sections/uk_UA/3-requirements/3-1-required-tools-for-the-GNU-Linux-based-OS.md)
    1. [Обов'язкові інструменти для ОС на базі MS Windows](/doc/sections/uk_UA/3-requirements/3-2-required-tools-for-the-MS-Windows-based-OS.md)
    1. [Необов'язкові пакети для тестів](/doc/sections/uk_UA/3-requirements/3-3-optional-for-the-tests.md)
    1. [Необов'язкові пакети для створення документації](/doc/sections/uk_UA/3-requirements/3-4-optional-for-the-documentation.md)
    1. [Необов'язкові пакети для форматування коду](/doc/sections/uk_UA/3-requirements/3-5-optional-for-the-code-formatting.md)
    1. [Необов'язкові пакети для статичного аналізатора коду cppcheck](/doc/sections/uk_UA/3-requirements/3-6-optional-for-the-code-analyzer-cppcheck.md)
    1. [Необов'язкові пакети для статичного аналізатора коду clang-tidy](/doc/sections/uk_UA/3-requirements/3-7-optional-for-the-code-analyzer-with-clang-tidy.md)
1. [Структура проекту](/doc/sections/uk_UA/4-project-structure/4-project-structure.md)
    1. [Діаграми проекту](/doc/sections/uk_UA/4-project-structure/4-0-project-diagrams.md)
    1. [Де живе реалізація імпортерів](/doc/sections/uk_UA/4-project-structure/4-1-implement-code-straight-away.md)
    1. [Публічні інтерфейсні файли бібліотеки](/doc/sections/uk_UA/4-project-structure/4-8-the-librarys-installable-include-header-files.md)
    1. [API імпортерів наборів даних](/doc/sections/uk_UA/4-project-structure/4-9-the-dataset-importers-api.md)
    1. [Розкладки наборів даних, які читаються](/doc/sections/uk_UA/4-project-structure/4-10-the-read-dataset-layouts.md)
    1. [Версіювання і інші параметри проекту](/doc/sections/uk_UA/4-project-structure/4-3-version-tracking-and-other-project-parameters.md)
    1. [Тести проекту](/doc/sections/uk_UA/4-project-structure/4-4-project-tests.md)
        1. [Фреймворк тестів Google Test](/doc/sections/uk_UA/4-project-structure/4-4-1-google-test.md)
1. [Побудова проекту](/doc/sections/uk_UA/5-project-build/5-project-build.md)
    1. [Побудова за допомогою IDE](/doc/sections/uk_UA/5-project-build/5-1-IDE-build.md)
    1. [Побудова проекту через командний рядок](/doc/sections/uk_UA/5-project-build/5-2-command-line-build.md)
    1. [Швидкі скрипти побудови](/doc/sections/uk_UA/5-project-build/5-38-quick-build-scripts.md)
    1. [Залежність від бібліотеки драйверів даних](/doc/sections/uk_UA/5-project-build/5-36-the-data-drivers-dependency.md)
    1. Вмикання тестів
        1. [Вмикання юніт-тестів](/doc/sections/uk_UA/5-project-build/testing/5-3-1-enabling-unit-testing.md)
        1. [Запобігання використання GTest з ОС](/doc/sections/uk_UA/5-project-build/testing/5-3-2-disabling-system-GTest-probe.md)
    1. [Побудова документації](/doc/sections/uk_UA/5-project-build/documentation/5-4-documentation-build.md)
    1. [Вмикання підтримки встановлення документації](/doc/sections/uk_UA/5-project-build/documentation/5-5-configuring-the-documentation-install-support.md)
    1. [Налаштування складових імені встановлюваної бібліотеки](/doc/sections/uk_UA/5-project-build/5-23-customizing-library-name-segments.md)
    1. Якість коду та санітайзери
        1. [Вмикання підтримки форматування коду](/doc/sections/uk_UA/5-project-build/code-quality/5-6-enabling-and-performing-code-formatting-target.md)
        1. [Вмикання підтримки цілі статичного аналізатора коду cppcheck](/doc/sections/uk_UA/5-project-build/code-quality/5-7-enabling-the-static-code-analyzer-target-with-cppcheck.md)
        1. [Вмикання підтримки статичного аналізатора коду clang-tidy](/doc/sections/uk_UA/5-project-build/code-quality/5-8-enabling-static-code-analyzer-with-clang-tidy.md)
    1. Контейнери та CI
        1. [Вмикання конвеєра Jenkins усередині Docker-контейнера](/doc/sections/uk_UA/5-project-build/containers-ci/5-17-enabling-Jenkins-pipeline-inside-Docker-container.md)
    1. Пакувальники
        1. [Вмикання підтримки генерування DEB-пакетів з cpack](/doc/sections/uk_UA/5-project-build/packagers/5-10-enabling-DEB-package-generation-with-cpack.md)
        1. [Вмикання підтримки генерування пакунків FreeBSD pkg з cpack](/doc/sections/uk_UA/5-project-build/packagers/5-20-enabling-FreeBSD-pkg-package-generation-with-cpack.md)
        1. [Вмикання підтримки генерування WIX MSI-пакетів з cpack](/doc/sections/uk_UA/5-project-build/packagers/5-21-enabling-WIX-MSI-package-generation-with-cpack.md)
        1. [Вмикання підтримки генерування RPM-пакунків з cpack](/doc/sections/uk_UA/5-project-build/packagers/5-22-enabling-RPM-package-generation-with-cpack.md)
    1. Бібліотеки
        1. [Вмикання читача розмірів зображень на OpenCV](/doc/sections/uk_UA/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md)
1. Запуск тестів
    1. [Запуск тестів за допомогою ctest](/doc/sections/uk_UA/6-running-the-tests/6-3-1-run-tests-by-the-ctest.md)
    1. [Ручний запуск тестів](/doc/sections/uk_UA/6-running-the-tests/6-3-2-manual-tests-run.md)
1. [Встановлення](/doc/sections/uk_UA/7-installing/7-installing.md)
1. [Використання бібліотеки у власному проекті](/doc/sections/uk_UA/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md)
