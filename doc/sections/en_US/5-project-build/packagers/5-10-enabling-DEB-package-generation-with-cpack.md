## Enabling DEB package generation with cpack

In order to be able to generate the `deb` package file configure project to support the `cpack` command feature with a `ENABLE_DEB` variable enabled:

```
# inside the project root directory 

cmake -B build -S . -DCMAKE_PREFIX_PATH=<data drivers prefix> -DENABLE_DEB=ON
```

Next, build all available targets required for the `deb` package

```
# inside the project root directory

cmake --build build --target all
```

Finally, execute the `cpack` command inside the project build directory:

```
# inside the project build directory

cpack
```

The package file should be generated inside the project build root directory. For example, if project name wasn't changed and it's version is 0.13.0 so the package name may look like `ImagesAnnotatorDataImporters-0.13.0-Linux.deb`.

The `CPACK_DEBIAN_PACKAGE_SHLIBDEPS` option is set to `ON`, so `dpkg-shlibdeps` reads the run time dependencies of the produced `libImagesAnnotatorDataImporters-0.13.so` out of the binary itself - the `ImagesAnnotatorDataDrivers` shared object it links, and the OpenCV ones when the build found them, end up in the package `Depends` field without any distribution package name being spelled out in the CMake files.

In order to examine details of the `deb` package configuration visit the [cmake/enablers/packagers/template-project-deb-enabler.cmake](/cmake/enablers/packagers/template-project-deb-enabler.cmake) file.
