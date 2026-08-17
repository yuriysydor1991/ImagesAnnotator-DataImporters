## Обов'язкові інструменти для ОС на базі GNU/Лінукс

Для того щоб побудувати бібліотеку, необхідно встановити компілятор GCC C++ разом з системою побудови CMake і системою версіювання Git:

```
sudo apt install -y git g++ cmake
```

Проект потребує CMake версії `3.13` або новішої і компілятора з підтримкою C++ `17`.

OpenCV є необов'язковим. Встановлення його пакунка розробки

```
sudo apt install -y libopencv-dev
```

дає бібліотеці власний читач розмірів зображень, тож проекту-споживачу не потрібен жоден набір засобів роботи із зображеннями для запуску імпортів YOLO v4 та PyTorch Vision. Без нього все інше збирається так само, а ті імпорти і надалі просять читача у свого споживача. Переглянь підсекцію [Вмикання читача розмірів зображень на OpenCV](/doc/sections/uk_UA/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md).

Єдина обов'язкова залежність, бібліотека ImagesAnnotatorDataDrivers, є спорідненим проектом даного і будується зі своїх власних джерельних кодів за адресою [https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git). Спершу побудуй і встанови її, а тоді передай даному проекту її префікс встановлення:

```
# з кореневої директорії проекту

cmake -S . -B build -DCMAKE_PREFIX_PATH=<data drivers install prefix>
cmake --build build -j$(nproc)
```

Переглянь підсекцію [Залежність від бібліотеки драйверів даних](/doc/sections/uk_UA/5-project-build/5-36-the-data-drivers-dependency.md) для деталей, включно зі змінною `IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE`, яка перевизначає ім'я пакунка, що шукається.
