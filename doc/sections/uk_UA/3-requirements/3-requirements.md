# Вимоги

Дана секція містить список усіх пакунків та інструментів, які повинні бути встановленими у системі для того, щоб побудувати бібліотеку імпортерів.

Одна залежність є обов'язковою - без неї крок конфігурування CMake завершується помилкою:

- Бібліотека **ImagesAnnotatorDataDrivers**. Імпортери будують через неї записи зображень і заповнюють базу даних їхніх анотацій, тому вона знаходиться за допомогою `find_package(ImagesAnnotatorDataDrivers-0.11 REQUIRED CONFIG)` і лінкується як `PUBLIC`, оскільки заголовкові файли, які встановлюються, згадують її тип бази даних. Спершу побудуй і встанови її з [https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git) і вкажи даному проекту її префікс встановлення за допомогою `-DCMAKE_PREFIX_PATH=<prefix>`. Переглянь підсекцію [Залежність від бібліотеки драйверів даних](/doc/sections/uk_UA/5-project-build/5-36-the-data-drivers-dependency.md).

Усе перелічене нижче є або базовим інструментом побудови, або необов'язковим доповненням, потрібним лише для окремої задачі розробки.

1. [Обов'язкові інструменти для ОС на базі GNU/Лінукс](/doc/sections/uk_UA/3-requirements/3-1-required-tools-for-the-GNU-Linux-based-OS.md)
1. [Обов'язкові інструменти для ОС на базі MS Windows](/doc/sections/uk_UA/3-requirements/3-2-required-tools-for-the-MS-Windows-based-OS.md)
1. [Необов'язкові пакети для тестів](/doc/sections/uk_UA/3-requirements/3-3-optional-for-the-tests.md)
1. [Необов'язкові пакети для створення документації](/doc/sections/uk_UA/3-requirements/3-4-optional-for-the-documentation.md)
1. [Необов'язкові пакети для форматування коду](/doc/sections/uk_UA/3-requirements/3-5-optional-for-the-code-formatting.md)
1. [Необов'язкові пакети для статичного аналізатора коду cppcheck](/doc/sections/uk_UA/3-requirements/3-6-optional-for-the-code-analyzer-cppcheck.md)
1. [Необов'язкові пакети для статичного аналізатора коду clang-tidy](/doc/sections/uk_UA/3-requirements/3-7-optional-for-the-code-analyzer-with-clang-tidy.md)

Бібліотека ImagesAnnotatorDataDrivers - єдина обов'язкова стороння залежність цієї бібліотеки. OpenCV є єдиною необов'язковою: з ним бібліотека несе і власний читач розмірів зображень, а без нього конфігурація лише повідомляє про це - див. [Вмикання читача розмірів зображень на OpenCV](/doc/sections/uk_UA/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md). Усе інше з переліченого вище є необов'язковим інструментарієм.
