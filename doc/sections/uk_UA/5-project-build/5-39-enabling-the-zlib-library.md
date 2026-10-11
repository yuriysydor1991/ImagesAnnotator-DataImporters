## Вмикання бібліотеки zlib

[zlib](https://zlib.net/) є обов'язковою залежністю цієї бібліотеки: файли
dxxwjz [darknetxx](https://github.com/yuriysydor1991/darknetxx), які читає
бібліотека, є стиснутими gzip JSON-документами, і `ZlibController` розпаковує їх частина за частиною.
`ZlibController` з [src/zlib](/src/zlib) - це компонент контролера zlib з
[cpp-app-template](https://github.com/yuriysydor1991/cpp-app-template) (його
гілки `appZlib`) в адаптації, яку зробив з нього darknetxx для своїх файлів
dxxwjz: стиснення в пам'яті і у gzip-файли та потоки gzip-файлів, які читають
і пишуть файли у сотні мегабайтів частина за частиною. Тому CMake-опція
`ENABLE_ZLIB` типово має значення `ON`, і для звичайної побудови нічого
вмикати не потрібно:

```
# в середині кореневої директорії проекту

cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix
```

Значення `ENABLE_ZLIB=OFF` зупиняє конфігурування з помилкою, оскільки без zlib
бібліотека не будується.

Інтеграція zlib розділена на два CMake-модулі:

- [cmake/enablers/template-project-zlib-enabler.cmake](/cmake/enablers/template-project-zlib-enabler.cmake)
  лише робить залежність доступною. Він виконується до додавання `src/`, тож
  виконувані файли модульних тестів з дерева джерельних кодів уже можуть самі
  лінкувати `ZLIB::ZLIB`.
- [cmake/enablers/template-project-zlib-linker.cmake](/cmake/enablers/template-project-zlib-linker.cmake)
  виконується після `src/` і лінкує zlib з ціллю бібліотеки, яка існує лише
  тоді. Лінкування `PRIVATE`: жоден публічний заголовок, який встановлюється,
  не відкриває типів zlib, тож залежність не потрапляє в експортований
  інтерфейс бібліотеки.

Спершу перевіряється наявність системної zlib за допомогою `find_package`. Коли
її не знайдено, джерельні коди завантажуються за допомогою `FetchContent` з
репозиторію і теґу зі змінних кешу `TEMPLATE_APP_ZLIB_GIT` і
`TEMPLATE_APP_ZLIB_GIT_TAG` (типово теґ `v1.3.1`), і статична бібліотека
`zlibstatic`, яку вони будують, лінкується під тим самим іменем `ZLIB::ZLIB`:

```
# в середині кореневої директорії проекту

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DTEMPLATE_APP_ZLIB_GIT_TAG=v1.3.1
```

Встановлення системного пакунка розробки (наприклад, `zlib1g-dev` у
дистрибутивах на базі Debian) - швидший шлях, який залишає крок конфігурування
без доступу до мережі.
