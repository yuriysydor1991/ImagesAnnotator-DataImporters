## Залежність від бібліотеки ImagesAnnotatorDataDrivers

Імпортери не оголошують власних типів записів. Вони будують базу даних анотацій
- записи зображень, їхні прямокутники й імена анотацій - через споріднену
бібліотеку
[ImagesAnnotator-DataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git),
типи якої живуть у просторі імен `ImagesAnnotatorDataDrivers012`.
`IADataImportersContext` тримає отриманий з неї `IAnnotationsDBPtr`, до якого зливаються
відновлені записи, тому залежність є обовʼязковою: без неї проект не
конфігурується.

### Як CMake її знаходить

Модуль [cmake/enablers/template-project-data-drivers-enabler.cmake](/cmake/enablers/template-project-data-drivers-enabler.cmake)
знаходить пакунок за допомогою

```
find_package(${IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE} REQUIRED CONFIG)
```

Він виконується під час проходу активаторів до `src/`, тобто ще до того як
зʼявиться ціль бібліотеки, щоб виконувані файли модульних тестів, оголошені
всередині дерева сирців, могли лінкуватись з імпортованою ціллю напряму. Модуль
також складає імʼя імпортованої цілі у змінну
`IMAGES_ANNOTATOR_DATA_DRIVERS_TARGET`, яка за замовчуванням розгортається у
`ImagesAnnotatorDataDrivers-0.12::ImagesAnnotatorDataDrivers-0.12`.

### Cache-змінна IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE

Імʼя пакунка не зашите у код. Воно міститься у cache-змінній
`IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE`, значення якої за замовчуванням -
`ImagesAnnotatorDataDrivers-0.12`, тобто імʼя, яке несе типове встановлення
драйверів даних, адже той проект постачається з `LIB_INCLUDE_MINOR_IN_NAME=ON`
так само як і цей. Заміни його, якщо будуєшся проти встановлення, складові імені
якого були змінені, наприклад сконфігурованого з `LIB_INCLUDE_MINOR_IN_NAME=OFF`
або з `LIB_NAME_SUFFIX`:

```
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/шлях/до/префіксу/встановлення/драйверів/даних \
  -DIMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE=ImagesAnnotatorDataDrivers-0
```

Імʼя імпортованої цілі слідує за заміненим імʼям пакунка автоматично. А от рядки
`#include <ImagesAnnotatorDataDrivers-0.12/...>` у сирцях - ні: вони дослівно
вказують субдиректорію заголовків типового встановлення, тому встановлення з
іншим імʼям потребує, щоб його директорія заголовків була досяжна за тим самим
написанням.

### Чому версія не запитується

Виклик `find_package` навмисно не запитує версію. Випуск 0.12.0 бібліотеки
драйверів даних встановлює свій файл пакунка `Config.cmake` без супутнього
`<package>ConfigVersion.cmake`, а `find_package` читає версію лише з того
супутнього файлу, тож будь-який запит із версією не збігся б із цілком справним
встановленням. Мажорну і мінорну версії вже несе саме імʼя пакунка (`...-0.12`),
і саме це тримає несумісні випуски окремо.

### Самостійна побудова і встановлення драйверів даних

Коли пакунок дистрибутиву недоступний, побудуй залежність із її сирців і встанови
у префікс на свій вибір:

```
git clone https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git
cmake -S ImagesAnnotator-DataDrivers -B ImagesAnnotator-DataDrivers/build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/iadd-install
cmake --build ImagesAnnotator-DataDrivers/build -j$(nproc)
cmake --install ImagesAnnotator-DataDrivers/build
```

Далі вкажи цьому проекту отриманий префікс за допомогою `CMAKE_PREFIX_PATH`:

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/iadd-install
cmake --build build -j$(nproc)
```

`CMAKE_PREFIX_PATH` приймає список, розділений `;`, тож можна вказати кілька
префіксів одразу. Його можна взагалі опустити, якщо бібліотеку драйверів даних
було встановлено у префікс, який CMake переглядає за замовчуванням, наприклад
`/usr` чи `/usr/local`. Ту саму послідовність виконує конвеєр
[misc/Jenkinsfile](/misc/Jenkinsfile) у своїй стадії `Dependencies` перед кожним
конфігуруванням.

Якщо префікс хибний або відсутній, конфігурування зупиняється з

```
CMake Error: Could not find a package configuration file provided by
"ImagesAnnotatorDataDrivers-0.12"
```

### Чому залежність лінкується як PUBLIC

На відміну від усіх інших залежностей цього проекту, бібліотека драйверів даних
лінкується як `PUBLIC` модулем
[cmake/enablers/template-project-data-drivers-linker.cmake](/cmake/enablers/template-project-data-drivers-linker.cmake).
Встановлюваний заголовок `IADataImportersContext.h` з
[src/lib/facade/public](/src/lib/facade/public) згадує тип бази даних драйверів
даних, тому споживачу цієї бібліотеки потрібні і її директорії заголовків, і її
спільний обʼєкт - лінкування як `PRIVATE` приховало б їх і зламало б кожну
компіляцію нижче за течією.

З тієї ж причини згенерований файл конфігурації пакунка
[src/lib/cmake/ImportersLibraryConfig.cmake.in](/src/lib/cmake/ImportersLibraryConfig.cmake.in)
викликає `find_dependency()` для пакунка драйверів даних перед підключенням
експортованих цілей. Отже, проект нижче за течією теж має зробити префікс
драйверів даних видимим для власного конфігурування, точно як описано у
[Використання бібліотеки у власному проекті](/doc/sections/uk_UA/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md).
