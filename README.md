# oneAPI Video Processing Library

The oneAPI Video Processing Library (oneVPL) provides a single video processing
API for encode, decode, and video processing that works across a wide range of
accelerators.

This repository contains the API, dispatcher, and a CPU reference implementation
of the specification.

## Building

### Configure the build environment

See the [developer reference](doc/developer-reference/build.rst) for detailed
instructions for configuring the build environment


### Build dependencies and oneVPL

Ubuntu bash shell:
```
script/bootstrap
script/build
```

You can find the build output in `_build`.

Windows cmd prompt:
```
script\bootstrap
script\build
```

You can find the build output in `_build\Release`.


### Run the examples

Ubuntu bash shell:
```
export LD_LIBRARY_PATH=`pwd`/_build
_build/hello-decode test/content/cars_128x96.h265
_build/hello-endoce test/content/cars_128x96.i420 128 96
```

Windows cmd prompt:
```
_build\Release\hello-decode test\content\cars_128x96.h265
_build\Release\hello-encode test\content\cars_128x96.i420 128 96
```


### Other builds

See the [scripts README](script/README.md) for other build options.

## Contributing

Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file for details.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file
for details.
