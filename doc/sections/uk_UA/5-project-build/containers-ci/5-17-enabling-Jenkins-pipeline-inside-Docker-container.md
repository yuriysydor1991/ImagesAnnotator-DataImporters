
## Enabling Jenkins pipeline inside Docker container

In order to enable the [Jenkins](https://www.jenkins.io/) pipeline execution inside the Docker container build and start the appropriate container with image by building the `jenkins-pipeline-docker-run` target:

```
# Inside the source root directory

# Configuring the project to support Docker with Jenkins targets
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_DOCKER=ON -DENABLE_JENKINS_DOCKER_PIPELINE=ON

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

The [cmake/enablers/dockerers/template-project-docker-Jenkins-pipeline-runner-target.cmake](/cmake/enablers/dockerers/template-project-docker-Jenkins-pipeline-runner-target.cmake) CMake script for the template project [Jenkins](https://www.jenkins.io/) pipeline Docker container builder by default will restart previously built contianer if target build launched repeatedly.

### Стартові скрипти

Директорія [scripts/docker](/scripts/docker) містить скорочення, які виконують обидві команди вище, отож увесь запуск виконується однією командою:

| Скрипт | Запускає ціль CMake |
| --- | --- |
| [jenkins-run.sh](/scripts/docker/jenkins-run.sh) | `jenkins-pipeline-docker-run` |
| [jenkins-build.sh](/scripts/docker/jenkins-build.sh) | `jenkins-pipeline-docker-build` |

```
# в середині кореневої директорії проекту

scripts/docker/jenkins-run.sh
```

Кожен з них конфігурує директорію `build/release` з параметрами `ENABLE_DOCKER` і `ENABLE_JENKINS_DOCKER_PIPELINE`, встановленими у `ON`, і потім будує потрібну ціль, отож жодної власної команди `docker` не виконується. Ціль CMake `jenkins-pipeline-docker-run` залежить від цілі `jenkins-pipeline-docker-build`, що означає, що скрипт [jenkins-run.sh](/scripts/docker/jenkins-run.sh) будує образ також, а скрипт [jenkins-build.sh](/scripts/docker/jenkins-build.sh) потрібен лише для побудови образу без запуску інтерактивного контейнера.

Скрипти приймають ті ж самі параметри, що й скрипти секції [Швидкі скрипти побудови](/doc/sections/uk_UA/5-project-build/5-38-quick-build-scripts.md), оскільки є їх викликачами:

```
# в середині кореневої директорії проекту

# змінює порт Web-панелі Jenkins контейнера
scripts/docker/jenkins-run.sh -DJENKINS_PIPELINES_PANEL_HTTP_PORT=9090
```

Чи буде повторно використано вже побудований образ і вже створений контейнер, вирішує скрипт CMake на кроці конфігурації, отож параметр `--no-reconfigure` зберігає рішення попереднього запуску конфігурації.
