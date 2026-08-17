## Обов'язкові інструменти для ОС на базі MS Windows

Для того щоб побудувати бінарні файли бібліотеки, необхідно завантажити інсталятор інструментів розробника зі сторінки завантажень MSVC [https://visualstudio.microsoft.com/downloads/](https://visualstudio.microsoft.com/downloads/), знайти і увімкнути секцію встановлення інструментів для нативної розробки програм на C++.

Окремо кожен необхідний проект можна завантажити і встановити з окремих джерел (**можлива необхідність завантаження і встановлення інших інструментів**):
- Система версіювання Git за адресою [https://git-scm.com/downloads/win](https://git-scm.com/downloads/win)
- Система побудови CMake за адресою [https://cmake.org/download/](https://cmake.org/download/)
- Пакунки Windows 10 SDK за адресою [https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/)

Єдина обов'язкова залежність стосується і цієї системи. Спершу побудуй і встанови бібліотеку ImagesAnnotatorDataDrivers з [https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git) і передай її префікс встановлення за допомогою `-DCMAKE_PREFIX_PATH=<prefix>`, як описано у підсекції [Залежність від бібліотеки драйверів даних](/doc/sections/uk_UA/5-project-build/5-36-the-data-drivers-dependency.md).
