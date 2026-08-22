## Налаштування складових імені встановлюваної бібліотеки

Імʼя бібліотеки, яке бачать проекти-споживачі (бінарник бібліотеки, субдиректорія
`include/<name>/` для публічних заголовків, директорія CMake package і простір
імен імпортованої цілі що експортується через `install(EXPORT ...)`) виводиться
з трьох опціональних CMake-опцій. Вони дозволяють паралельним встановленням
різних версій бібліотеки співіснувати у системі - наприклад
`include/ImagesAnnotatorDataImporters-0.13.0-dev/` поруч з
`include/ImagesAnnotatorDataImporters-0.13/`.

| Опція | За замовчуванням | Ефект |
|---|---|---|
| `-DLIB_INCLUDE_MINOR_IN_NAME=OFF` | `ON` | Додає `.<minor>` до імені бібліотеки |
| `-DLIB_INCLUDE_MICRO_IN_NAME=ON` | `OFF` | Додає `.<micro>` (вмикає прапорець minor неявно) |
| `-DLIB_NAME_SUFFIX=-dev` | `""` | Додає довільний завершальний суфікс |

Складова minor включена за замовчуванням, оскільки простір імен публічного API
несе у собі пару "мажорна і мінорна версія" (`ImagesAnnotatorDataImporters013`),
а також тому, що [залежність від бібліотеки драйверів даних](/doc/sections/uk_UA/5-project-build/5-36-the-data-drivers-dependency.md)
іменує себе точно так само. Тому два мінорні випуски встановлюються цілком
паралельно - їхні бінарники, директорії заголовків і CMake-пакунки відрізняються.

Приклади імен бібліотеки для проекту `0.13.0`:

| Прапорці конфігурації | Імʼя бібліотеки | Згенерований бінарник |
|---|---|---|
| (немає) | `ImagesAnnotatorDataImporters-0.13` | `libImagesAnnotatorDataImporters-0.13.so` |
| `-DLIB_INCLUDE_MINOR_IN_NAME=OFF` | `ImagesAnnotatorDataImporters-0` | `libImagesAnnotatorDataImporters-0.so` |
| `-DLIB_INCLUDE_MICRO_IN_NAME=ON` | `ImagesAnnotatorDataImporters-0.13.0` | `libImagesAnnotatorDataImporters-0.13.0.so` |
| `-DLIB_NAME_SUFFIX=-dev` | `ImagesAnnotatorDataImporters-0.13-dev` | `libImagesAnnotatorDataImporters-0.13-dev.so` |
| `-DLIB_INCLUDE_MICRO_IN_NAME=ON -DLIB_NAME_SUFFIX=-dev` | `ImagesAnnotatorDataImporters-0.13.0-dev` | `libImagesAnnotatorDataImporters-0.13.0-dev.so` |

Приклад поєднаної конфігурації:

```
# всередині кореневої директорії проекту

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/шлях/до/префіксу/встановлення/драйверів/даних \
  -DLIB_INCLUDE_MICRO_IN_NAME=ON \
  -DLIB_NAME_SUFFIX=-dev
cmake --build build -j$(nproc)
cmake --install build --prefix /usr/local
```

Однакове імʼя послідовно використовується для кожного встановлюваного артефакту.
За замовчуванням це дає:

- `<prefix>/lib/libImagesAnnotatorDataImporters-0.13.so.0.13.0` разом із
  символьним посиланням soname `libImagesAnnotatorDataImporters-0.13.so.0`
  (`SOVERSION` залишається мажорною версією) і `.so`-посиланням для розробки,
- `<prefix>/include/ImagesAnnotatorDataImporters-0.13/*.h` - публічні заголовки з
  [src/lib/facade/public](/src/lib/facade/public), оголошені через
  `INSTALL_INTERFACE` цілі бібліотеки,
- `<libdir>/cmake/ImagesAnnotatorDataImporters-0.13/` зі згенерованими файлами
  `Config.cmake`, `ConfigVersion.cmake` і `Targets.cmake`, а також імпортовану
  ціль `ImagesAnnotatorDataImporters-0.13::ImagesAnnotatorDataImporters-0.13`.

Тому проект-споживач пише `find_package(ImagesAnnotatorDataImporters-0.13 0.13
REQUIRED)` для типового встановлення і має вживати змінене імʼя дослівно, якщо
складові були налаштовані інакше. Простір імен C++ у сирцевих файлах,
`ImagesAnnotatorDataImporters013`, цими опціями не змінюється.

Виведення імʼя реалізоване у
[cmake/template-project-misc-variables-declare.cmake](/cmake/template-project-misc-variables-declare.cmake)
і доступне як CMake cache-змінна `PROJECT_LIBRARY_NAME`, яку використовує решта
системи побудови, включно з іменем директорії HTML-виводу Doxygen. Вмикання
`LIB_INCLUDE_MICRO_IN_NAME` без `LIB_INCLUDE_MINOR_IN_NAME` спричиняє
попередження на стадії конфігурування, а прапорець minor вмикається неявно щоб
згенероване імʼя залишалось синтаксично коректним.

Імʼя [залежності від драйверів даних](/doc/sections/uk_UA/5-project-build/5-36-the-data-drivers-dependency.md)
не залежить від цих опцій: воно обирається окремою cache-змінною
`IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE`.
