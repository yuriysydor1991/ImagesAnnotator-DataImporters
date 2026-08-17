# Побудова проекту

1. [Побудова за допомогою IDE](/doc/sections/uk_UA/5-project-build/5-1-IDE-build.md)
1. [Побудова проекту-шаблону через командний рядок](/doc/sections/uk_UA/5-project-build/5-2-command-line-build.md)
1. [Швидкі скрипти побудови](/doc/sections/uk_UA/5-project-build/5-38-quick-build-scripts.md)
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
1. Пакувальники
    1. [Вмикання підтримки генерування DEB-пакетів з cpack](/doc/sections/uk_UA/5-project-build/packagers/5-10-enabling-DEB-package-generation-with-cpack.md)
    1. [Вмикання підтримки генерування пакунків FreeBSD pkg з cpack](/doc/sections/uk_UA/5-project-build/packagers/5-20-enabling-FreeBSD-pkg-package-generation-with-cpack.md)
    1. [Вмикання підтримки генерування WIX MSI-пакетів з cpack](/doc/sections/uk_UA/5-project-build/packagers/5-21-enabling-WIX-MSI-package-generation-with-cpack.md)
    1. [Вмикання підтримки генерування RPM-пакунків з cpack](/doc/sections/uk_UA/5-project-build/packagers/5-22-enabling-RPM-package-generation-with-cpack.md)
1. Бібліотеки
    1. [Вмикання читача розмірів зображень на OpenCV (необов'язково)](/doc/sections/uk_UA/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md)

Бібліотека ImagesAnnotatorDataDrivers - єдина обов'язкова стороння залежність
цієї бібліотеки. OpenCV є необов'язковим: без нього бібліотека просто не несе
власного читача розмірів зображень.
