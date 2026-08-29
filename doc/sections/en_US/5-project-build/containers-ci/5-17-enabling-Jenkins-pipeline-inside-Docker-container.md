
## Enabling Jenkins pipeline inside Docker container

In order to enable the [Jenkins](https://www.jenkins.io/) pipeline execution inside the Docker container build and start the appropriate container with image by building the `jenkins-pipeline-docker-run` target:

```
# Inside the source root directory

# Configuring the project to support Docker with Jenkins targets
cmake -S . -B build -DCMAKE_PREFIX_PATH=<data drivers prefix> -DCMAKE_BUILD_TYPE=Debug -DENABLE_DOCKER=ON -DENABLE_JENKINS_DOCKER_PIPELINE=ON

# Building and starting Jenkins' pipeline Docker container
# or restarting it
cmake --build build --target jenkins-pipeline-docker-run
```

Locate and save the new generated password for the [Jenkins](https://www.jenkins.io/) Web GUI in the log output that may look like next:

```
# ...

*************************************************************
*************************************************************
*************************************************************

Jenkins initial setup is required. An admin user has been created and a password generated.
Please use the following password to proceed to installation:

aefdc6a4a2214c298c821236c739165c

This may also be found at: /var/jenkins_home/secrets/initialAdminPassword

*************************************************************
*************************************************************
*************************************************************

# ...
```

Go to `http://127.0.0.1:8080` (port may change) address with the help of prefered Web browser. Process to finilize the installation of the [Jenkins](https://www.jenkins.io/) system with the help of it's GUI.

After successfull installation and configuration of the [Jenkins](https://www.jenkins.io/) create appropriate pipeline for the project inside the [Jenkins](https://www.jenkins.io/) Web GUI panel.

All the [Jenkins](https://www.jenkins.io/) pipeline CMake build scripts are located at [cmake/enablers/dockerers/template-project-docker-Jenkins-pipeline-runner-target.cmake](/cmake/enablers/dockerers/template-project-docker-Jenkins-pipeline-runner-target.cmake) file. The pipeline script located at [misc/Jenkinsfile](/misc/Jenkinsfile) which should be specified in the pipeline source script at the [Jenkins](https://www.jenkins.io/) Web GUI pipelines configuration page. The [Jenkins](https://www.jenkins.io/) Web GUI Docker container port may be changed by specifying a new value for the `JENKINS_PIPELINES_PANEL_HTTP_PORT` CMake variable.

The [cmake/enablers/dockerers/template-project-docker-Jenkins-pipeline-runner-target.cmake](/cmake/enablers/dockerers/template-project-docker-Jenkins-pipeline-runner-target.cmake) CMake script of the [Jenkins](https://www.jenkins.io/) pipeline Docker container builder by default will restart previously built contianer if target build launched repeatedly.

### Starter scripts

The [scripts/docker](/scripts/docker) directory holds the shortcuts which perform both commands above, so the whole run takes a single command:

| Script | Starts the CMake target |
| --- | --- |
| [jenkins-run.sh](/scripts/docker/jenkins-run.sh) | `jenkins-pipeline-docker-run` |
| [jenkins-build.sh](/scripts/docker/jenkins-build.sh) | `jenkins-pipeline-docker-build` |

```
# inside the project root directory

scripts/docker/jenkins-run.sh
```

Each of them configures the `build/release` directory with both the `ENABLE_DOCKER` and the `ENABLE_JENKINS_DOCKER_PIPELINE` options set to `ON` and then builds the target of interest, so no `docker` command of its own is ever issued. The `jenkins-pipeline-docker-run` CMake target depends on the `jenkins-pipeline-docker-build` one, which means the [jenkins-run.sh](/scripts/docker/jenkins-run.sh) script builds the image as well and the [jenkins-build.sh](/scripts/docker/jenkins-build.sh) one is only needed to build the image without starting the interactive container.

The scripts accept the very same parameters as the [Quick build scripts](/doc/sections/en_US/5-project-build/5-38-quick-build-scripts.md) ones, since they are their callers:

```
# inside the project root directory

# changes the Jenkins Web GUI port of the container
scripts/docker/jenkins-run.sh -DJENKINS_PIPELINES_PANEL_HTTP_PORT=9090
```

Whether the already built image and the already created container are reused or rebuilt is decided by the CMake script at the configure step, so the `--no-reconfigure` parameter keeps the decision of the previous configure run.

### The pipeline dependency parameters

The library is built against the installed `ImagesAnnotatorDataDrivers` package (see the [The ImagesAnnotatorDataDrivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md) section), so the pipeline has a `Dependencies` stage which resolves it before anything is configured:

- `DATA_DRIVERS_PREFIX` - install prefix of an already built `ImagesAnnotatorDataDrivers`. When it is set the stage simply reuses it.
- `DATA_DRIVERS_REF` - the `ImagesAnnotatorDataDrivers` git reference (`main` by default) which the stage clones, builds and installs into the workspace local `deps-install` prefix when `DATA_DRIVERS_PREFIX` is left empty.

The resolved prefix is then passed to every `cmake` configure call of the pipeline as `-DCMAKE_PREFIX_PATH`.

The container image built from [misc/Dockerfiles/Dockerfile.Jenkins.pipeline.in](/misc/Dockerfiles/Dockerfile.Jenkins.pipeline.in) carries the compiler, `cmake`, `git`, `clang-format`, `clang-tidy`, `cppcheck`, `valgrind`, the GTest/GMock packages and the OpenCV development package; the data drivers dependency is built from source by the `Dependencies` stage.

### The pipeline stages

After the checkout and the dependency stage the pipeline runs:

- a `clang-format` configure and build, failing the run when a tracked file is left unformatted;
- a `Debug` configure and build with `ENABLE_UNIT_TESTS=ON`, `ENABLE_COMPONENT_TESTS=ON`, `ENABLE_SANITIZERS=ON`, `ENABLE_CPPCHECK=ON` and `MAX_LOG_LEVEL=5`;
- the `cppcheck` target, which is opt-in through the `RUN_CPPCHECK` boolean parameter;
- the unit tests in parallel, one stage per target: `UTEST_IADataImportersFacade`, `UTEST_LibFactory`, `UTEST_LibMain`, `UTEST_PlainTxtFolder2DBImporter`, `UTEST_Yolo4Folder2DBImporter` and `UTEST_PyTorchVisionFolder2DBImporter`;
- the component tests in parallel: `CTEST_DefaultLogger` and `CTEST_Importers` (the latter links the produced shared library and drives it through the installed public headers only);
- a `clang-tidy` configure and build;
- a `Release` configure, build and an install into a temporary prefix.

Each test stage publishes its own JUnit report from the `build/junit-out` directory.

The remaining stages verify the repository consistency itself: no git conflict markers in the tracked files, no `uk_UA` sections linked from the top level `README.md` and no `en_US` sections linked from [doc/README.uk_UA.md](/doc/README.uk_UA.md), every file under `doc/sections/<language>` linked from its README, every relative markdown link resolvable, every CMake module under `cmake/enablers` registered in the feature enabler files and every CMake declared test registered in the pipeline itself.

The package generation checks are the last stages and every one of them is opt-in, because each performs a complete configure and build of its own: `RUN_DEB_PACKAGER`, `RUN_RPM_PACKAGER`, `RUN_FREEBSD_PACKAGER` (FreeBSD hosts only) and `RUN_WIX_PACKAGER` (Microsoft Windows hosts only). Each enabled stage builds the project with the corresponding `ENABLE_*` option, runs `cpack` with the matching generator and fails when no package file was produced.
