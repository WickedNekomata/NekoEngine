# Hello Wwise Wamp - Python Sample
## Overview

This sample demonstrates how to connect to Wwise through WAAPI using Python with Autobahn and Asyncio.

## Requirements

### Native packages
For Python 2.7:
1. [Microsoft Visual C++ Compiler for Python 2.7](http://aka.ms/vcpython27)

### Python packages
Make sure you have pip installed (it should be provided with the Python installation and the command might not be available until you restart your computer).

Install Autobahn using the following command in the terminal:

``` pip install autobahn ```

If you are using Python 3.6, no other package is required.

If you are using Python 2.7, Asyncio is not available as a builtin module. Instead, install "Trollius" which is a port of Asyncio for Python 2.7:

``` pip install trollius ```

You may optionally install the module "futures" when using Python 2.7 to use a pool of threads instead of a synchronous executor (preventing blocking operations): ``` pip install futures ```.

The sample was tested using Autobahn version 17.8.1. This version is compatible with both Python 2.7 and 3.6.

## Execution

Run the following commands from the "hello-wwise-wamp" directory:

If you are using Python 2:

    python main_py2.py

If you are using Python 3:

    python main_py3.py

Select an object in Wwise's Project Explorer and see that its name is printed in the output.

## Troubleshooting

If you are unable to properly install Trollius using the previous steps, you can download a built version.

Download the file appropriate for your Python installation from https://www.lfd.uci.edu/~gohlke/pythonlibs/#trollius. (Note, Audiokinetic is not responsible for the content of external sites.)

Make sure pip is up to date:

``` pip install --upgrade pip ```

Install wheel:

``` pip install wheel ```

Install trollius:

``` pip install trollius-2.1-cp27-cp27m-win32.whl ```