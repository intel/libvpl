# Contributing guidelines

## Pull Request Checklist

Before sending your pull requests, make sure you followed this list.

- Read [contributing guidelines](CONTRIBUTING.md).

- Check if changes are consistent with the
  [guidelines](#general-guidelines-and-philosophy-for-contribution).

- Check if changes are consistent with the
  [coding style](#commit-message-style).


## How to become a contributor and submit your own code

### Contribution guidelines and standards

Before sending your pull request for review, make sure your changes are
consistent with the guidelines and follow the coding style.

#### General guidelines and philosophy for contribution

Contributed code must be:

- *Tested*: Include tests when you contribute new features, as they help to a)
  prove that your code works correctly, and b) guard against future breaking
  changes to lower the maintenance cost. Bug fixes also generally require tests,
  because the presence of bugs usually indicates insufficient test coverage.

- *Documented*: Pull requests that change documented functionality should update
  the relevant documentation. New functionality should be documented.

- *Portable*: The project supports different operating systems CPU and GPU
  architectures, compilers, and run-times.


#### License

Include a license at the top of new files.

#### Commit message style

1.  Separate subject from body with a blank line
2.  Limit the subject line to 50 characters
3.  Capitalize the subject line
4.  Do not end the subject line with a period
5.  Use the imperative mood in the subject line
6.  Wrap the body at 72 characters
7.  Use the body to explain what and why vs. how

Further reading:
[How to Write a Git Commit Message](https://chris.beams.io/posts/git-commit/)

#### Automated style checking

Use `script/lint` to assist with style conformance checking. To install its
python prerequisites do:

```
pip3 install -r requirements-dev.txt
```

Version 11.0.0 of `clang-format` also needs to be in the path.

[Windows binary](https://commondatastorage.googleapis.com/chromium-clang-format/d4afd4eba27022f5f6d518133aebde57281677c9)
(sha1: d4afd4eba27022f5f6d518133aebde57281677c9)

[Linux binary](https://commondatastorage.googleapis.com/chromium-clang-format/1baf0089e895c989a311b6a38ed94d0e8be4c0a7)
(sha1: 1baf0089e895c989a311b6a38ed94d0e8be4c0a7)
