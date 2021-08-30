
# Higiia v2.0 - Client Project Sandbox 

## _A domainless Content-Based Medical Image Retrieval tool for CT and RX_

Higiia is a Content-Based Medical Image Retrieval (CBMIR) tool designed for handling any domain of CT and RX images. This version was coded in C++ with Qt SDK (5.15.1 - LGPL-license) and it is fully compatible with Web Assembly (emscripten coupled to Qt 5.15.1 with em++ compiler v2.0.22).

It supports JPEG, PNG, BMP, and (most important) KRL and DICOM medical images by using self-contained 3rd-party libraries (see below). The query mechanism is provided by a counter-part that supports an extended SQL language and can be configured as an external server (see [The Siren Project][siren]), while external client-server applications can be used for managing medical image storage (see [QWebSocket File Transfer Project][ws]. As a result, the Higiia v2.0 implementation follows a simplified Model–View–Controller software pattern, in which a set of pre-defined tables and classes are employed for accessing and querying medical image datasets.

To set up your first CBMIR application supported by Higiia v2.0, the following steps are required:

1. Configure the Siren Server and run the DDL creation of its internal tables.
> The Siren server can be downloaded from [its GitHub entry][siren] and compiled with Qt 5.15.1 basic modules (+ network and SQL). Please note Siren is **neither** compatible with em++ (Web Assembly) **nor** loaded with its internal tables by default. Please, mind running the Siren DDL script once after the first instantiation of the server.

2. Run the Higiia DDL script on the server-side.
> You can use a simple telnet connection (i.e., telnet <siren-server-ip> <siren-server-port>) to submit the DDL script that contains standard SQL commands for the creation of the Higiia v2.0 Model tables.

3. Structure your medical image dataset as an Higiia table and instantiate it on the server-side.
> Please, refer to the [Higiia Modelling Guide][model] to structure your medical dataset as an Higiia table. The modeling process will generate an extra table <U_dataset> in which queries will be placed.

4. Load your **data** in the new dataset table, and the **queries** (potentially undiagnosed cases) in the Higiia automatically generated table.
> Data are inserted with extended INSERT INTO commands. See this [example for a dataset of lung images][ws]. Notice, feature extraction can be carried out manually or with the support of your favorite programming language and framework, e.g., sklearn, pandas, etc.

5. Run Higiia by providing the server address and port
> You can straightforwardly run the higiia.html file alongside .wasm and .js loaders or compile the source code with Qt to generate a binary version of Higiia. Alternatively, we can download the binary file of [Higiia v2.0 for Windows][ws].

6. ✨Query (undiagnosed) images by content✨

## A handful of new features

- Run either desktop or web version of Higiia v2.0 alike
- Support for DICOM images
- Query images by content with either similarity or diversified similarity criteria
- Support for zooming and windowing operations
- Keep relevant and discard non-relevant results
- Support for relevance-feedback cycles
- Explore the entire search space from the query image viewpoint
- Distance-based labeling of query images

## Background technologies and 3rd-party libraries

Higiia client tool relies on a series of background technologies and 3rd-party libraries, namely:

1. Background technologies 
    - [Qt SDK][qt] - Qt Cross-Platform and SDK C++ development environment.
    - [Emscripten][ems] - The Open Source Emscripten compiler toolchain (from C++) to WebAssembly. Supported (in some way) by Qt for WebAssembly.

2.  3rd-party libraries (self-contained)
    - [Hermes][hermes] - Our own C++ library of distance functions (and utils).
    - [Artemis][artemis] - Our own wrapper library for images (and utils).
    - [Dicomlib][dicomlib] - The classical self-contained Trevor Morgan dicomlib library from Google Code Archive (We were able to compile it with brand new Emscripten 😀 - Classic codes die hard).
    - [Boost][boost] - A few Boost .h files (without the need for compiling the entire library) - wasm-compliant.
    - [Eigen][eigen] - A few Eigen .h files (without the need for compiling the entire library) - wasm-compliant.

## Limitations

Known limitations include:

- Absence of SSL authentication due to Web Assembly and QSsl conflicts (See [here](https://bugreports.qt.io/browse/QTBUG-76350)).

## Installation

Install the background technologies and download the source code of 3rd-party libraries.
Then, adjust the include paths of `higiia.pro` file to point to the installation folders of the 3rd-party libraries.
After that, you can use `build.sh` to compile the client version.
Notice, you can configure the building type of Higiia in the `build.sh` file itself by selecting the `qmake` binary of your QT setup (e.g., wasm\_32 (default) or gcc\_64).

```sh
#!/bin/sh
~/Qt/5.15.1/wasm_32/bin/qmake   #for Web-Assembly building
#~/Qt/5.15.1/gcc_64/bin/qmake   #for desktop-binary building
make
```

Then, run the following command.

```sh
cd higiia
./build.sh && make clean
```

Higiia v2.0 binaries require Qt DLLs to run on Windows machines. The web version of Higiia requires the browser to support .wasm files loaded through .js. 

## Directory structure

The directory tree is structured as follows.

| Dir | Description |
| ------ | ------ |
| root  | Main functionalities and util |
| icons | Contains the .png icons of the application |
| controller | Contains the controller classes for the application |
| GUI | Contains the form and source code of the application |
| frontend | Contains a pre-compiled .wasm version of Higiia  |
| 3rd-party  | Includes the entry points for 3rd-party libraries |


**WARNING: Higiia is NOT clinical software. It is built for education and demonstration purposes ONLY!**

[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format it nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

   [siren]: <github.com/marcosivni/siren2.0>
   [qt]: <https://www.qt.io/download>
   [ws]: <github.com/marcosivni/siren_web_socket>
   [ems]: <https://emscripten.org/docs/introducing_emscripten/index.html>
   [hermes]: <github.com/marcosivni/hermes>
   [artemis]: <github.com/marcosivni/artemis>
   [dicomlib]: <github.com/marcosivni/dicomlib>
   [boost]: <https://www.boost.org/>
   [eigen]: <https://eigen.tuxfamily.org/>
   [model]: <https://github.com/marcosivni/higiia/blob/main/model/Model.md>
   

![GitHub all releases](https://img.shields.io/github/downloads/marcosivni/higiia/total?style=flat-square)
