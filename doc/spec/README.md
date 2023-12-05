# Intel® Video Processing Library Specification

# How to build locally
The following instructions were tested on Ubuntu 22.04.

1. Go to the root level of this repository

2. Build Docker image with build environment:

```console
python3 doc/spec/build-spec.py dockerbuild
```

3. Startup Docker container

```console
python3 doc/spec/build-spec.py dockerrun
```

4. Inside the container, this command will build HTML version of the specification:

```console
python3 doc/spec/build-spec.py html doc/spec
```

Generated spec will be in this folder: doc/spec/build/html

5. Alternatively, you can build PDF version by using this command:

```console
python3 doc/spec/build-spec.py latexpdf doc/spec
```

6. To run the spell checker

```console
python3 doc/spec/build-spec.py --verbose spelling doc/spec
```

Words to ignore in spell checking may be added to the files:

``doc/spec/spelling_wordlist.txt``
