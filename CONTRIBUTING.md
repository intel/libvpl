# Contributing

The Intel® Video Processing Library (Intel® VPL) project welcomes community
contributions. You can:

- Submit your changes directly as a [pull request](https://github.com/intel/libvpl/pulls)
- Log a bug or feature request with an [issue](https://github.com/intel/libvpl/issues)

## Pull Request Checklist

Before submitting your pull requests, please do the following:

- Make sure your Pull Request includes these three sections in the description

  - **Issue** - link for the ticket from Jira/HSD/ETC or state the problem
  - **Solution** - Overview of your approach to solving the problem
  - **Test** - how you tested the change

- Make sure you update CHANGELOG.md if the change is important.

- Make sure your changes are consistent with the
  [philosophy for contribution](#philosophy-for-contribution).

- Make sure the correct license is included at the top of new files.

- Make sure your commits follow the correct
  [commit message style](#commit-message-style).

- Make sure to test your code. At a minimum run `script/test`.

- Make sure to [sign your work](#sign-your-work).


## Philosophy for Contribution

Contributed code must be:

- *Tested*: Include tests when you contribute new features, as they help to a)
  prove that your code works correctly, and b) guard against future breaking
  changes to lower the maintenance cost. Bug fixes also generally require tests,
  because the presence of bugs usually indicates insufficient test coverage.

- *Documented*: Pull requests that change documented functionality should update
  the relevant documentation. New functionality should be documented.

- *Portable*: The project supports different operating systems CPU and GPU
  architectures, compilers, and run-times.

## Commit Message Style

1.  Separate subject from body with a blank line
2.  Limit the subject line to 50 characters
3.  Capitalize the subject line
4.  Do not end the subject line with a period
5.  Use the imperative mood in the subject line
6.  Wrap the body at 72 characters
7.  Use the body to explain what and why vs. how

Further reading:
[How to Write a Git Commit Message](https://chris.beams.io/posts/git-commit/)

## Sign Your Work

Please use the sign-off line at the end of the patch. Your signature certifies
that you wrote the patch or otherwise have the right to pass it on as an
open-source patch. The rules are pretty simple: if you can certify
the below (from [developercertificate.org](http://developercertificate.org/)):

```
Developer Certificate of Origin
Version 1.1

Copyright (C) 2004, 2006 The Linux Foundation and its contributors.
660 York Street, Suite 102,
San Francisco, CA 94110 USA

Everyone is permitted to copy and distribute verbatim copies of this
license document, but changing it is not allowed.

Developer's Certificate of Origin 1.1

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the open source license
    indicated in the file; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part
    by me, under the same open source license (unless I am
    permitted to submit under a different license), as indicated
    in the file; or

(c) The contribution was provided directly to me by some other
    person who certified (a), (b) or (c) and I have not modified
    it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    personal information I submit with it, including my sign-off) is
    maintained indefinitely and may be redistributed consistent with
    this project or the open source license(s) involved.
```

Then you just add a line to every git commit message:

    Signed-off-by: Kris Smith <kris.smith@email.com>

Use your real name (sorry, no pseudonyms or anonymous contributions.)

If you set your `user.name` and `user.email` git configs, you can sign your
commit automatically with `git commit -s`.
