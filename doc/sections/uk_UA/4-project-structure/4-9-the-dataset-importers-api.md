## API імпортерів наборів даних

Увесь інтерфейс бібліотеки оголошено заголовними файлами під [src/lib/facade/public](/src/lib/facade/public); вони встановлюються до піддиректорії `include/ImagesAnnotatorDataImporters-0.11` обраного префікса встановлення. Ніщо інше не залишає спільного обʼєкта: кожен клас реалізації лишається за абстрактними інтерфейсами, описаними тут.

Усі встановлювані оголошення живуть у просторі імен `ImagesAnnotatorDataImporters011`. Назва несе мажорний і мінорний номери версії бібліотеки (`0.11` дає суфікс `011`), тож дві версії бібліотеки можуть співіснувати в одній одиниці трансляції. Створіть для неї псевдонім один раз у власному коді:

```cpp
namespace iadi = ImagesAnnotatorDataImporters011;
```

Записи, які будують імпортери, тут не визначені. Вони походять із бібліотеки [ImagesAnnotatorDataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git), простір імен інтерфейсу якої - `ImagesAnnotatorDataDrivers011` (нижче з псевдонімом `iadd`). Див. підрозділ [Залежність від драйверів даних](/doc/sections/uk_UA/5-project-build/5-36-the-data-drivers-dependency.md) щодо збіркової частини цієї залежності.

### Встановлювані заголовні файли

| Заголовок | Оголошує |
| --- | --- |
| [ImportersAPI.h](/src/lib/facade/public/ImportersAPI.h) | макрос видимості `IADI_API`, яким позначено кожне встановлюване оголошення |
| [IImporter.h](/src/lib/facade/public/IImporter.h) | абстрактний інтерфейс імпортера `IImporter` |
| [IImageSizeFacility.h](/src/lib/facade/public/IImageSizeFacility.h) | інтерфейс `IImageSizeFacility`, який реалізує проект-споживач |
| [LibraryContext.h](/src/lib/facade/public/LibraryContext.h) | клас вхідних та вихідних даних `LibraryContext`, яким керуються обидві точки входу |
| [PlainTxtImportLibraryContext.h](/src/lib/facade/public/PlainTxtImportLibraryContext.h) | нащадок `LibraryContext` розкладки простого тексту |
| [Yolo4ImportLibraryContext.h](/src/lib/facade/public/Yolo4ImportLibraryContext.h) | нащадок `LibraryContext` розкладки YOLO v4 (darknet) |
| [PyTorchImportLibraryContext.h](/src/lib/facade/public/PyTorchImportLibraryContext.h) | нащадок `LibraryContext` розкладки PyTorch Vision |
| [ILib.h](/src/lib/facade/public/ILib.h) | абстрактний інтерфейс бібліотеки `ILib` із методом `perform_import` |
| [LibraryFacade.h](/src/lib/facade/public/LibraryFacade.h) | клас-фабрику `LibraryFacade`, точку входу бібліотеки |

Підключення `LibraryFacade.h` тягне за собою кожен інший заголовок зі списку.

### Розкладки наборів даних

```cpp
class PlainTxtImportLibraryContext : public LibraryContext;
class Yolo4ImportLibraryContext : public LibraryContext;
class PyTorchImportLibraryContext : public LibraryContext;
```

Три нащадки `LibraryContext` називають три розкладки наборів даних, які бібліотека вміє читати. Інстанціація одного з них - це і є вибір розкладки, а бібліотека відображає цей тип на імпортер, який її читає. Жоден із них нічого не додає до `LibraryContext`, бо все, що потрібно імпорту - директорія-джерело, база даних-призначення та засіб вимірювання зображень - зберігається у базовому класі. Що саме кожен із них очікує знайти на диску, описано в підрозділі [Розкладки наборів даних, які читаються](/doc/sections/uk_UA/4-project-structure/4-10-the-read-dataset-layouts.md).

Ці три є оберненням перших трьох розкладок, які записує споріднена бібліотека [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git), і пара замикає коло: проект, експортований у будь-яку з них та імпортований назад, дає ті самі анотації, з якими починав - настільки, наскільки сама розкладка здатна їх нести.

`Yolo4ImportLibraryContext` та `PyTorchImportLibraryContext` - це дві розкладки, які неможливо прочитати, не знаючи розміру кожного зображення: див. `IImageSizeFacility` нижче. `PlainTxtImportLibraryContext` не потребує вимірювання взагалі, бо та розкладка зберігає свої прямокутники у власних пікселях зображення, точно так, як їх тримає внутрішній формат проекту.

### LibraryContext

Єдиний клас даних бібліотеки, той самий, яким керуються обидві її точки входу: одноразовий `ILib::perform_import()` та `IImporter::import_db()` імпортера, зібраного вручну. Створіть його фабричним методом `LibraryFacade` потрібної розкладки - `create_plain_txt_library_context()`, `create_yolo4_library_context()` чи `create_pytorch_library_context()` - або інстанціюйте нащадка самостійно, як це робить споживач, шаблонізований за типом розкладки.

Дані, які він несе, є приватними і доступні лише через методи доступу. Кожен геттер віддає `const`-посилання на те, що зберігає контекст, кожен сеттер копіює передане значення всередину:

| Методи доступу | Напрямок | Значення |
| --- | --- | --- |
| `get_import_path()`, `set_import_path()` | вх. | директорія-джерело імпорту, `std::string`, обовʼязкова |
| `get_db()`, `set_db()` | вх.-вих. | база даних анотацій, до якої зливаються відновлені записи, `ImagesAnnotatorDataDrivers011::IAnnotationsDBPtr`, обовʼязкова |
| `get_image_sizer()`, `set_image_sizer()` | вх. | засіб вимірювання зображень, `IImageSizeFacilityPtr`. Обовʼязковий для розкладок, які без нього неможливо прочитати, якщо бібліотеку не зібрано з OpenCV |
| `get_importer()`, `set_importer()` | вих. | екземпляр імпортера, який виконав останній `perform_import`, `IImporterPtr` |
| `get_imported_records()`, `set_imported_records()` | вих. | скільки записів зображень останній імпорт відновив і передав до бази даних, `std::size_t` |

`LibraryContextPtr` - псевдонім `std::shared_ptr<LibraryContext>`.

База даних є `IAnnotationsDB`, а не лише читабельним `IImagesPathsDBProvider`, який бере бібліотека експортерів, бо імпорт має **додавати** записи: вони передаються через `IAnnotationsDB::add_images_db()`, який пропускає зображення, що база вже містить, і тримає результат відсортованим за шляхами зображень. Тому імпорт одного й того самого набору даних двічі додає його зображення один раз, а імпорт до бази даних проекту, що редагується, доповнює її, а не заміщує. Порожню базу отримайте з `iadd::LibraryFacade::create_annotations_db()`.

`get_imported_records()` рахує те, що імпорт відновив із набору даних, а не приріст бази даних - ці два числа різняться на записи, які злиття вище знайшло вже наявними.

### IImporter

```cpp
virtual bool import_db(LibraryContextPtr ictx) = 0;
```

Єдиний метод імпортера. Він читає набір даних, названий контекстом, у розкладці, яку цей імпортер реалізує, зливає відновлені записи до бази даних того самого контексту й повертає `true`, коли запуск загалом пройшов. Записи, які він не може обробити - зіпсований рядок, відсутній файл зображення, зображення, яке не вдалося виміряти - пропускаються та повідомляються через журнал бібліотеки, вони не завалюють запуск. `false` означає, що контекст неповний, шлях імпорту не містить директорії або директорія взагалі не містить набору даних цієї розкладки - відсутній чи нечитабельний дескриптор, навколо якого розкладку побудовано. База даних лишається недоторканою.

Розкладка, яку називає тип контексту, тут не розглядається: читається розкладка самого імпортера, тож навіть базовий клас `LibraryContext`, який не називає жодної розкладки, цілком придатний для керування `import_db()`. Реалізація не повинна утримувати контекст після виклику, бо контекст, що тримає цей імпортер, замкнув би цикл вказівників. `IImporterPtr` - псевдонім `std::shared_ptr<IImporter>`.

### IImageSizeFacility

Це єдиний інтерфейс, який проекту-споживачеві може довестися реалізувати самому:

```cpp
virtual bool read_image_size(const std::string& imagePath, int& width,
                             int& height) = 0;
virtual IImageSizeFacilityPtr clone() = 0;
```

Бібліотека не декодує жодного формату зображень власними силами, а анотація внутрішнього формату проекту - це прямокутник у пікселях зображення, над яким його намальовано. Тому набір даних, який зберігає свої рамки нормалізованими - тут це YOLO v4 - неможливо прочитати назад без розміру зображення, якому кожна рамка належить, а розкладку PyTorch Vision, у якій уся анотація і є обрізаним зображенням, - і поготів. Отже, імпорти, яким потрібне таке вимірювання, просять свого споживача виконати його тим графічним стеком, який той проект уже й так лінкує.

- `imagePath` - шлях файлової системи до зображення, яке слід виміряти. Інтерфейс навмисно приймає шлях, а не запис зображення: запис - це те, що імпорт саме будує, і поля його розміру заповнює цей виклик.
- `width` та `height` - вихідні параметри, які слід заповнити власним піксельним розміром того файлу.
- Поверніть `true`, щойно обидва прочитано. Повернення `false` для нечитабельного або непідтримуваного файлу не є помилкою саме по собі: імпорт занотує це одне зображення й піде далі.

Бібліотека, зібрана з OpenCV, постачає власну реалізацію, тож цей інтерфейс доводиться реалізовувати лише проектові, який хоче власного вимірювання, - або тому, що споживає бібліотеку, зібрану без OpenCV. Див. [Увімкнення читача розмірів зображень на OpenCV](/doc/sections/uk_UA/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md).

`clone()` має створювати копію, що не поділяє жодного змінного стану декодування з оригіналом. Імпортери, які постачаються сьогодні, викликають лише `read_image_size()`, але метод є частиною інтерфейсу й має бути реалізований.

### ILib

`ILib::perform_import(LibraryContextPtr ctx)` - одноразова точка входу бібліотеки. Вона будує імпортер розкладки, яку називає контекст, публікує його через `ctx->set_importer()` і виконує імпорт над тим самим контекстом, `get_imported_records()` якого потім несе кількість. Вона повертає `false`, коли контекст не називає жодної відомої розкладки або коли не вдався сам імпорт. Проектам, яким потрібне точніше керування, варто натомість збудувати імпортер напряму через `LibraryFacade::create_importer()` і викликати на ньому `import_db()` - з тим самим контекстом в обох руках.

### LibraryFacade

Клас лише зі статичних фабричних методів, і єдина точка входу, яка потрібна проекту-споживачеві:

| Метод | Повертає |
| --- | --- |
| `create_plain_txt_library_context()` | новий порожній `PlainTxtImportLibraryContextPtr` |
| `create_yolo4_library_context()` | новий порожній `Yolo4ImportLibraryContextPtr` |
| `create_pytorch_library_context()` | новий порожній `PyTorchImportLibraryContextPtr` |
| `create_default_lib()` | типову реалізацію `ILibPtr` |
| `create_library(LibraryContextPtr ctx)` | реалізацію `ILibPtr`, відповідну до наданого контексту |
| `create_importer(const LibraryContextPtr& ctx)` | новий `IImporterPtr` для розкладки контексту або `nullptr` для контексту, що не називає жодної відомої розкладки |
| `create_image_sizer()` | читач розмірів зображень, який бібліотека постачає сама, або `nullptr` у збірці без OpenCV |
| `library_version()` | рядок версії використовуваного бінарника бібліотеки |

### Повний приклад

Програма нижче читає назад навчальну директорію YOLO v4 і записує відновлені анотації як файл проекту ImagesAnnotator через бібліотеку драйверів даних. Див. розділ [Використання бібліотеки у власному проекті](/doc/sections/uk_UA/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md) щодо CMake-частини.

```cpp
#include <ImagesAnnotatorDataDrivers-0.11/LibraryFacade.h>
#include <ImagesAnnotatorDataImporters-0.11/LibraryFacade.h>

#include <iostream>
#include <memory>

namespace iadd = ImagesAnnotatorDataDrivers011;
namespace iadi = ImagesAnnotatorDataImporters011;

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " <dataset dir> <project.json>\n";
    return 1;
  }

  auto db = iadd::LibraryFacade::create_annotations_db();

  if (db == nullptr) {
    std::cerr << "fail to create the annotations database\n";
    return 1;
  }

  auto ctx = iadi::LibraryFacade::create_yolo4_library_context();

  ctx->set_import_path(argv[1]);
  ctx->set_db(db);

  auto importer = iadi::LibraryFacade::create_importer(ctx);

  if (importer == nullptr) {
    std::cerr << "no importer available for the requested layout\n";
    return 1;
  }

  if (!importer->import_db(ctx)) {
    std::cerr << "the import has failed\n";
    return 1;
  }

  if (!db->store_db(argv[2])) {
    std::cerr << "fail to store the project file " << argv[2] << '\n';
    return 1;
  }

  std::cout << "imported " << ctx->get_imported_records()
            << " image records by the importers library version "
            << iadi::LibraryFacade::library_version() << '\n';

  return 0;
}
```

Тут не передано жодного читача розмірів зображень, тож програмі вище потрібна бібліотека, зібрана з OpenCV. Інакше передайте власний читач до `ctx->set_image_sizer()` - наступний підрозділ показує як - або почніть із розкладки простого тексту, якій він не потрібен.

Зауважте, що імпортувати можна й **до** наявного файлу проекту: відкрийте його через `iadd::LibraryFacade::open_annotations_db()` замість створення порожньої бази, і імпорт долучить зображення, яких проект ще не містить.

### Реалізація читача розмірів зображень

Ескіз нижче підключає такий читач до графічних процедур, які проект-споживач уже має - підставте власні виклики декодування замість `my_imaging`:

```cpp
#include <ImagesAnnotatorDataImporters-0.11/IImageSizeFacility.h>

#include <memory>
#include <string>

namespace iadi = ImagesAnnotatorDataImporters011;

class MySizer : public iadi::IImageSizeFacility
{
 public:
  bool read_image_size(const std::string& imagePath, int& width,
                       int& height) override
  {
    auto image = my_imaging::load(imagePath);

    if (!image) {
      return false;
    }

    width = my_imaging::width_of(image);
    height = my_imaging::height_of(image);

    return width > 0 && height > 0;
  }

  IImageSizeFacilityPtr clone() override
  {
    return std::make_shared<MySizer>();
  }
};
```

Передайте екземпляр через контекст, і кожен імпортер його підхопить:

```cpp
auto ctx = iadi::LibraryFacade::create_yolo4_library_context();

ctx->set_image_sizer(std::make_shared<MySizer>());
```

Читач, установлений у такий спосіб, завжди перемагає той, який постачає бібліотека, зібрана з OpenCV, тож проект, що вже декодує зображення власним чином, робить це й далі.

### Позначка IADI_API

Кожен встановлюваний клас позначено макросом `IADI_API` з [ImportersAPI.h](/src/lib/facade/public/ImportersAPI.h), а бібліотека компілюється з прихованою видимістю символів, тож спільний обʼєкт залишає лише позначений інтерфейс. Це вимога коректності, а не оптимізація розміру: бібліотека драйверів даних, з якою ця лінкується, побудована з того самого шаблону проекту й несе власні визначення `default_logger::DefaultLogger` та `project_decls`. Якби експортувалися обидва набори, динамічний компонувальник міг би звʼязати виклики однієї бібліотеки з інакше розкладеними класами іншої.

Приховування не сягає фабрики. Інстанціація `std::make_shared` називає свій клас у власному спотвореному імені й лишається слабкою та експортованою, якою б не була видимість, тож простір імен реалізації тут - `iadi0impl`, а не `lib0impl`, який використовує шаблон проекту, а разом із ним і бібліотека драйверів даних.

Робочого споживача всього інтерфейсу збережено в дереві як компонентний тест `CTEST_Importers`, [src/importers/tests/component/Importers/CTEST_Importers.cpp](/src/importers/tests/component/Importers/CTEST_Importers.cpp): він лінкує справжню спільну бібліотеку й керує нею лише через публічні заголовки, точно так, як це робить проект нижче за течією.
