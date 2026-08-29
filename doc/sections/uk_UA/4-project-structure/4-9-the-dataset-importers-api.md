## API імпортерів наборів даних

Увесь інтерфейс бібліотеки оголошено заголовними файлами під [src/lib/facade/public](/src/lib/facade/public); вони встановлюються до піддиректорії `include/ImagesAnnotatorDataImporters-0.14` обраного префікса встановлення. Ніщо інше не залишає спільного обʼєкта: кожен клас реалізації лишається за абстрактними інтерфейсами, описаними тут.

Усі встановлювані оголошення живуть у просторі імен `ImagesAnnotatorDataImporters014`. Назва несе мажорний і мінорний номери версії бібліотеки (`0.14` дає суфікс `014`), тож дві версії бібліотеки можуть співіснувати в одній одиниці трансляції. Створіть для неї псевдонім один раз у власному коді:

```cpp
namespace iadi = ImagesAnnotatorDataImporters014;
```

Записи, які будують імпортери, тут не визначені. Вони походять із бібліотеки [ImagesAnnotatorDataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git), простір імен інтерфейсу якої - `ImagesAnnotatorDataDrivers012` (нижче з псевдонімом `iadd`). Див. підрозділ [Залежність від драйверів даних](/doc/sections/uk_UA/5-project-build/5-36-the-data-drivers-dependency.md) щодо збіркової частини цієї залежності.

### Встановлювані заголовні файли

| Заголовок | Оголошує |
| --- | --- |
| [ImportersAPI.h](/src/lib/facade/public/ImportersAPI.h) | макрос видимості `IADI_API`, яким позначено кожне встановлюване оголошення |
| [IImporter.h](/src/lib/facade/public/IImporter.h) | абстрактний інтерфейс імпортера `IImporter` |
| [IImageSizeFacility.h](/src/lib/facade/public/IImageSizeFacility.h) | інтерфейс `IImageSizeFacility`, який реалізує проект-споживач |
| [IADataImportersContext.h](/src/lib/facade/public/IADataImportersContext.h) | клас вхідних та вихідних даних `IADataImportersContext`, яким керуються обидві точки входу |
| [PlainTxtImportContext.h](/src/lib/facade/public/contexts/PlainTxtImportContext.h) | нащадок `IADataImportersContext` розкладки простого тексту |
| [Yolo4ImportContext.h](/src/lib/facade/public/contexts/Yolo4ImportContext.h) | нащадок `IADataImportersContext` розкладки YOLO v4 (darknet) |
| [UltralyticsDetectImportContext.h](/src/lib/facade/public/contexts/UltralyticsDetectImportContext.h) | нащадок `IADataImportersContext` розкладки виявлення Ultralytics YOLO |
| [UltralyticsObbImportContext.h](/src/lib/facade/public/contexts/UltralyticsObbImportContext.h) | нащадок `IADataImportersContext` розкладки орієнтованих обмежувальних рамок Ultralytics YOLO |
| [UltralyticsSegmentImportContext.h](/src/lib/facade/public/contexts/UltralyticsSegmentImportContext.h) | нащадок `IADataImportersContext` розкладки сегментації примірників Ultralytics YOLO |
| [CocoImportContext.h](/src/lib/facade/public/contexts/CocoImportContext.h) | нащадок `IADataImportersContext` розкладки виявлення обʼєктів COCO |
| [PascalVocImportContext.h](/src/lib/facade/public/contexts/PascalVocImportContext.h) | нащадок `IADataImportersContext` розкладки Pascal VOC |
| [CreateMLImportContext.h](/src/lib/facade/public/contexts/CreateMLImportContext.h) | нащадок `IADataImportersContext` розкладки виявлення обʼєктів Create ML |
| [PyTorchImportContext.h](/src/lib/facade/public/contexts/PyTorchImportContext.h) | нащадок `IADataImportersContext` розкладки PyTorch Vision |
| [IADataImportersLib.h](/src/lib/facade/public/IADataImportersLib.h) | абстрактний інтерфейс бібліотеки `IADataImportersLib` із методом `perform_import` |
| [IADataImportersFacade.h](/src/lib/facade/public/IADataImportersFacade.h) | клас-фабрику `IADataImportersFacade`, точку входу бібліотеки |

Підключення `IADataImportersFacade.h` тягне за собою кожен інший заголовок зі списку.

### Розкладки наборів даних

```cpp
class PlainTxtImportContext : public IADataImportersContext;
class Yolo4ImportContext : public IADataImportersContext;
class UltralyticsDetectImportContext : public IADataImportersContext;
class UltralyticsObbImportContext : public IADataImportersContext;
class UltralyticsSegmentImportContext : public IADataImportersContext;
class CocoImportContext : public IADataImportersContext;
class PascalVocImportContext : public IADataImportersContext;
class CreateMLImportContext : public IADataImportersContext;
class PyTorchImportContext : public IADataImportersContext;
```

Девʼять нащадків `IADataImportersContext` називають девʼять розкладок наборів даних, які бібліотека вміє читати. Інстанціація одного з них - це і є вибір розкладки, а бібліотека відображає цей тип на імпортер, який її читає. Жоден із них нічого не додає до `IADataImportersContext`, бо все, що потрібно імпорту - директорія-джерело, база даних-призначення та засіб вимірювання зображень - зберігається у базовому класі. Що саме кожен із них очікує знайти на диску, описано в підрозділі [Розкладки наборів даних, які читаються](/doc/sections/uk_UA/4-project-structure/4-10-the-read-dataset-layouts.md).

Ці девʼять є оберненням тих самих девʼяти розкладок, які записує споріднена бібліотека [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) - одна до одної, під тими самими назвами - і пара замикає коло: проект, експортований у будь-яку з них та імпортований назад, дає ті самі анотації, з якими починав - настільки, наскільки сама розкладка здатна їх нести.

Пʼять розкладок, які неможливо прочитати, не знаючи розміру кожного зображення (див. `IImageSizeFacility` нижче), - це `Yolo4ImportContext`, три `Ultralytics*ImportContext`, чия геометрія зберігається поділеною на той розмір, та `PyTorchImportContext`, чиєю анотацією є саме обрізане зображення. `CocoImportContext` та `PascalVocImportContext` читають розміри з власних дескрипторів, а `PlainTxtImportContext` і `CreateMLImportContext` не потребують вимірювання взагалі, бо обидві зберігають свої прямокутники у власних пікселях зображення, точно так, як їх тримає внутрішній формат проекту.

### IADataImportersContext

Єдиний клас даних бібліотеки, той самий, яким керуються обидві її точки входу: одноразовий `IADataImportersLib::perform_import()` та `IImporter::import_db()` імпортера, зібраного вручну. Створіть його фабричним методом `IADataImportersFacade` потрібної розкладки - одним із девʼяти `create_*_library_context()`, перелічених нижче - або інстанціюйте нащадка самостійно, як це робить споживач, шаблонізований за типом розкладки.

Дані, які він несе, є приватними і доступні лише через методи доступу. Кожен геттер віддає `const`-посилання на те, що зберігає контекст, кожен сеттер копіює передане значення всередину:

| Методи доступу | Напрямок | Значення |
| --- | --- | --- |
| `get_import_path()`, `set_import_path()` | вх. | директорія-джерело імпорту, `std::string`, обовʼязкова |
| `get_db()`, `set_db()` | вх.-вих. | база даних анотацій, до якої зливаються відновлені записи, `ImagesAnnotatorDataDrivers012::IAnnotationsDBPtr`, обовʼязкова |
| `get_image_sizer()`, `set_image_sizer()` | вх. | засіб вимірювання зображень, `IImageSizeFacilityPtr`. Обовʼязковий для розкладок, які без нього неможливо прочитати, якщо бібліотеку не зібрано з OpenCV |
| `get_importer()`, `set_importer()` | вих. | екземпляр імпортера, який виконав останній `perform_import`, `IImporterPtr` |
| `get_imported_records()`, `set_imported_records()` | вих. | скільки записів зображень останній імпорт відновив і передав до бази даних, `std::size_t` |

`IADataImportersContextPtr` - псевдонім `std::shared_ptr<IADataImportersContext>`.

База даних є `IAnnotationsDB`, а не лише читабельним `IImagesPathsDBProvider`, який бере бібліотека експортерів, бо імпорт має **додавати** записи: вони передаються через `IAnnotationsDB::add_images_db()`, який пропускає зображення, що база вже містить, і тримає результат відсортованим за шляхами зображень. Тому імпорт одного й того самого набору даних двічі додає його зображення один раз, а імпорт до бази даних проекту, що редагується, доповнює її, а не заміщує. Порожню базу отримайте з `iadd::IADataDriversFacade::create_annotations_db()`.

`get_imported_records()` рахує те, що імпорт відновив із набору даних, а не приріст бази даних - ці два числа різняться на записи, які злиття вище знайшло вже наявними.

### IImporter

```cpp
virtual bool import_db(IADataImportersContextPtr ictx) = 0;
```

Єдиний метод імпортера. Він читає набір даних, названий контекстом, у розкладці, яку цей імпортер реалізує, зливає відновлені записи до бази даних того самого контексту й повертає `true`, коли запуск загалом пройшов. Записи, які він не може обробити - зіпсований рядок, відсутній файл зображення, зображення, яке не вдалося виміряти - пропускаються та повідомляються через журнал бібліотеки, вони не завалюють запуск. `false` означає, що контекст неповний, шлях імпорту не містить директорії або директорія взагалі не містить набору даних цієї розкладки - відсутній чи нечитабельний дескриптор, навколо якого розкладку побудовано. База даних лишається недоторканою.

Розкладка, яку називає тип контексту, тут не розглядається: читається розкладка самого імпортера, тож навіть базовий клас `IADataImportersContext`, який не називає жодної розкладки, цілком придатний для керування `import_db()`. Реалізація не повинна утримувати контекст після виклику, бо контекст, що тримає цей імпортер, замкнув би цикл вказівників. `IImporterPtr` - псевдонім `std::shared_ptr<IImporter>`.

### IImageSizeFacility

Це єдиний інтерфейс, який проекту-споживачеві може довестися реалізувати самому:

```cpp
virtual bool read_image_size(const std::string& imagePath, int& width,
                             int& height) = 0;
virtual IImageSizeFacilityPtr clone() = 0;
```

Бібліотека не декодує жодного формату зображень власними силами, а анотація внутрішнього формату проекту - це прямокутник у пікселях зображення, над яким його намальовано. Тому набір даних, який зберігає свої рамки поділеними на той розмір - тут це YOLO v4 та три розкладки Ultralytics YOLO - неможливо прочитати назад без розміру зображення, якому кожна рамка належить, а розкладку PyTorch Vision, у якій уся анотація і є обрізаним зображенням, - і поготів. Отже, імпорти, яким потрібне таке вимірювання, просять свого споживача виконати його тим графічним стеком, який той проект уже й так лінкує.

Чотири розкладки, що лишилися, читаються й без нього. Дескриптори COCO та Pascal VOC несуть розмір кожного зображення, яке називають, а розкладки простого тексту та Create ML тримають свої прямокутники у власних пікселях зображення, тож читач лише заповнює поля `iwidth` та `iheight` створених записів - що варто надати однаково, бо саме ті два поля дозволяють анотатору малювати зображення у власному масштабі.

- `imagePath` - шлях файлової системи до зображення, яке слід виміряти. Інтерфейс навмисно приймає шлях, а не запис зображення: запис - це те, що імпорт саме будує, і поля його розміру заповнює цей виклик.
- `width` та `height` - вихідні параметри, які слід заповнити власним піксельним розміром того файлу.
- Поверніть `true`, щойно обидва прочитано. Повернення `false` для нечитабельного або непідтримуваного файлу не є помилкою саме по собі: імпорт занотує це одне зображення й піде далі.

Бібліотека, зібрана з OpenCV, постачає власну реалізацію, тож цей інтерфейс доводиться реалізовувати лише проектові, який хоче власного вимірювання, - або тому, що споживає бібліотеку, зібрану без OpenCV. Див. [Увімкнення читача розмірів зображень на OpenCV](/doc/sections/uk_UA/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md).

`clone()` має створювати копію, що не поділяє жодного змінного стану декодування з оригіналом. Імпортери, які постачаються сьогодні, викликають лише `read_image_size()`, але метод є частиною інтерфейсу й має бути реалізований.

### IADataImportersLib

`IADataImportersLib::perform_import(IADataImportersContextPtr ctx)` - одноразова точка входу бібліотеки. Вона будує імпортер розкладки, яку називає контекст, публікує його через `ctx->set_importer()` і виконує імпорт над тим самим контекстом, `get_imported_records()` якого потім несе кількість. Вона повертає `false`, коли контекст не називає жодної відомої розкладки або коли не вдався сам імпорт. Проектам, яким потрібне точніше керування, варто натомість збудувати імпортер напряму через `IADataImportersFacade::create_importer()` і викликати на ньому `import_db()` - з тим самим контекстом в обох руках.

### IADataImportersFacade

Клас лише зі статичних фабричних методів, і єдина точка входу, яка потрібна проекту-споживачеві:

| Метод | Повертає |
| --- | --- |
| `create_plain_txt_library_context()` | новий порожній `PlainTxtImportContextPtr` |
| `create_yolo4_library_context()` | новий порожній `Yolo4ImportContextPtr` |
| `create_ultralytics_detect_library_context()` | новий порожній `UltralyticsDetectImportContextPtr` |
| `create_ultralytics_obb_library_context()` | новий порожній `UltralyticsObbImportContextPtr` |
| `create_ultralytics_segment_library_context()` | новий порожній `UltralyticsSegmentImportContextPtr` |
| `create_coco_library_context()` | новий порожній `CocoImportContextPtr` |
| `create_pascal_voc_library_context()` | новий порожній `PascalVocImportContextPtr` |
| `create_createml_library_context()` | новий порожній `CreateMLImportContextPtr` |
| `create_pytorch_library_context()` | новий порожній `PyTorchImportContextPtr` |
| `create_default_lib()` | типову реалізацію `IADataImportersLibPtr` |
| `create_library(IADataImportersContextPtr ctx)` | реалізацію `IADataImportersLibPtr`, відповідну до наданого контексту |
| `create_importer(const IADataImportersContextPtr& ctx)` | новий `IImporterPtr` для розкладки контексту або `nullptr` для контексту, що не називає жодної відомої розкладки |
| `create_image_sizer()` | читач розмірів зображень, який бібліотека постачає сама, або `nullptr` у збірці без OpenCV |
| `library_version()` | рядок версії використовуваного бінарника бібліотеки |

### Повний приклад

Програма нижче читає назад навчальну директорію YOLO v4 і записує відновлені анотації як файл проекту ImagesAnnotator через бібліотеку драйверів даних. Див. розділ [Використання бібліотеки у власному проекті](/doc/sections/uk_UA/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md) щодо CMake-частини.

```cpp
#include <ImagesAnnotatorDataDrivers-0.12/IADataDriversFacade.h>
#include <ImagesAnnotatorDataImporters-0.14/IADataImportersFacade.h>

#include <iostream>
#include <memory>

namespace iadd = ImagesAnnotatorDataDrivers012;
namespace iadi = ImagesAnnotatorDataImporters014;

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " <dataset dir> <project.json>\n";
    return 1;
  }

  auto db = iadd::IADataDriversFacade::create_annotations_db();

  if (db == nullptr) {
    std::cerr << "fail to create the annotations database\n";
    return 1;
  }

  auto ctx = iadi::IADataImportersFacade::create_yolo4_library_context();

  ctx->set_import_path(argv[1]);
  ctx->set_db(db);

  auto importer = iadi::IADataImportersFacade::create_importer(ctx);

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
            << iadi::IADataImportersFacade::library_version() << '\n';

  return 0;
}
```

Тут не передано жодного читача розмірів зображень, тож програмі вище потрібна бібліотека, зібрана з OpenCV. Інакше передайте власний читач до `ctx->set_image_sizer()` - наступний підрозділ показує як - або почніть із розкладки простого тексту, якій він не потрібен.

Зауважте, що імпортувати можна й **до** наявного файлу проекту: відкрийте його через `iadd::IADataDriversFacade::open_annotations_db()` замість створення порожньої бази, і імпорт долучить зображення, яких проект ще не містить.

### Реалізація читача розмірів зображень

Ескіз нижче підключає такий читач до графічних процедур, які проект-споживач уже має - підставте власні виклики декодування замість `my_imaging`:

```cpp
#include <ImagesAnnotatorDataImporters-0.14/IImageSizeFacility.h>

#include <memory>
#include <string>

namespace iadi = ImagesAnnotatorDataImporters014;

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
auto ctx = iadi::IADataImportersFacade::create_yolo4_library_context();

ctx->set_image_sizer(std::make_shared<MySizer>());
```

Читач, установлений у такий спосіб, завжди перемагає той, який постачає бібліотека, зібрана з OpenCV, тож проект, що вже декодує зображення власним чином, робить це й далі.

### Позначка IADI_API

Кожен встановлюваний клас позначено макросом `IADI_API` з [ImportersAPI.h](/src/lib/facade/public/ImportersAPI.h), а бібліотека компілюється з прихованою видимістю символів, тож спільний обʼєкт залишає лише позначений інтерфейс. Це вимога коректності, а не оптимізація розміру: бібліотека драйверів даних, з якою ця лінкується, побудована з того самого шаблону проекту й несе власні визначення `default_logger::DefaultLogger` та `project_decls`. Якби експортувалися обидва набори, динамічний компонувальник міг би звʼязати виклики однієї бібліотеки з інакше розкладеними класами іншої.

Приховування не сягає фабрики. Інстанціація `std::make_shared` називає свій клас у власному спотвореному імені й лишається слабкою та експортованою, якою б не була видимість, тож простір імен реалізації тут - `iadi0impl`, а не `lib0impl`, який використовує шаблон проекту, а разом із ним і бібліотека драйверів даних.

Робочого споживача всього інтерфейсу збережено в дереві як компонентний тест `CTEST_Importers`, [src/importers/tests/component/Importers/CTEST_Importers.cpp](/src/importers/tests/component/Importers/CTEST_Importers.cpp): він лінкує справжню спільну бібліотеку й керує нею лише через публічні заголовки, точно так, як це робить проект нижче за течією.
