# Test Tools

This folder contains tools used by tests.

## Python Tools

Tools in the Python folder are designed to be called either form the command
line, or as libraries. The entire test/tools/python folder can be added to
PYTHONPATH to expose these tools in the environment.

the vars.* scripts in test/env will add test/tools/python to PYTHONPATH
as part of their environment setup.

ex:

Note: in these examples "foo" is used as the test name. The actual tools
are stored in test/tools/python with each tool stored in a folder named
the same as the tool.

```batch
:: Windows
test/env/vars.bat
py -3 -m foo --help
```

```bash
# Bash
source test/env/vars.sh
python3 -m foo --help
```

```python
# Python
import foo
foo.foo()
```


