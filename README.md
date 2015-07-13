#Score-P Dataheap Event Plugin

##Compilation and Installation

###Prerequisites

To compile this plugin, you need:

* C++11 compiler

* Score-P

* The Dataheap library and header files for `libdataheap`. The Dataheap libraries and `libdataheap`
    are available at <https://fusionforge.zih.tu-dresden.de/projects/dataheap>.

###Building

1. Invoke CMake

        cmake -DLIBDATAHEAP_LIBRARIES=/path/to/libdataheap.a -DLIBDATAHEAP_INCLUDE_DIRS=/path/to/dataheap/include

2. Invoke make

        make
        
> *Note:*

> If you have `scorep-config` in your `PATH`, it should be found by CMake.

###Installation

1. Set the installation prefix

        cmake -DCMAKE_INSTALL_PREFIX=/path/to/install .

2. Invoke make

        make install

> *Note:*

> Make sure to add the subfolder `lib` to your `LD_LIBRARY_PATH`.

##Usage

To add a dataheap metric to your trace, you have to add `dataheap_plugin` to the environment
variable `SCOREP_METRIC_PLUGINS`.

In order for the plugin to connect to the right server, you have to define the dataheap server with
the environment variable `SCOREP_METRIC_DATAHEAP_SERVER`.

You have to add the list of the metric channel you are interested in to the environment variable
`SCOREP_METRIC_DATAHEAP_PLUGIN`.

For example:

    export SCOREP_METRIC_PLUGINS="dataheap_plugin"
    export SCOREP_METRIC_DATAHEAP_PLUGIN="rover/watts"
    export SCOREP_METRIC_DATAHEAP_SERVER="dataheap.my.institute.edu:22222"

> *Note:*

> The plugin replaces the first instance of `localhost` in any metric name with the hostname of the
> executing machine.

###Environment variables

* `DATAHEAP_METRIC_SERVER`

    Defines hostname and port of your dataheap installation.

* `SCOREP_METRIC_DATAHEAP_PLUGIN`

    Defines a counter on your dataheap server.

###If anything fails:

1. Check whether the plugin library can be loaded from the `LD_LIBRARY_PATH`.

2. Check whether your dataheap server is working properly and you can connect to it.

3. Write a mail to the author.

##Authors

* Mario Bielert (mario.bielert at tu-dresden dot de)
