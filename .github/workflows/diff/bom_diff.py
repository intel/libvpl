#!/usr/bin/env python3
############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""Generate BOM diffrence report"""
import argparse
import os
import sys
import datetime
import contextlib
import filecmp


# pylint: disable=too-many-instance-attributes
# pylint: disable=consider-using-dict-items
class DiffInfo:
    """Diff information for a single tree node"""
    def log(self, msg):
        """Print a log message for this node"""
        print("  " * self.depth, end='')
        print(msg)

    # pylint: disable=too-many-arguments,too-many-positional-arguments
    def __init__(self, left, right, path='', name='', depth=0):
        self._cmp = None
        self._has_diff = False
        self._has_orphan = False
        self.depth = depth
        self.name = name
        self._root = {'left': left, 'right': right}
        self.rel_path = os.path.join(path, name)
        self._path = {
            'left': os.path.join(left, self.rel_path),
            'right': os.path.join(right, self.rel_path)
        }
        self.children = {}
        for side in self._path:
            self._extend(self._path[side])

    def flags(self, side):
        """Get the flag info for one side of the node"""
        if side not in self._path:
            raise IndexError()
        if self.exists(side):
            return oct(os.stat(self._path[side]).st_mode)[2:]
        return ''

    def path(self, side):
        """Get the filesystem path for one side of the node"""
        if side not in self._path:
            raise IndexError()
        return self._path[side]

    def file_size(self, side):
        """Get the size of one side of the node"""
        if side not in self._path:
            raise IndexError()
        if self.exists(side):
            return os.stat(self._path[side]).st_size
        return ''

    def hasorphan(self):
        """Check if node is an orphan or has orphan children"""
        self.cmp()
        return self._has_orphan

    def hasdiff(self):
        """Check if node is a diffs or has diffs children"""
        self.cmp()
        return self._has_diff

    def exists(self, side):
        """Check if one side of the node exists in the filesystem"""
        if side not in self._path:
            raise IndexError()
        return os.path.exists(self._path[side])

    def isfile(self, side):
        """Check if one side of the node is a file"""
        if side not in self._path:
            raise IndexError()
        return os.path.isfile(self._path[side])

    def isdir(self, side):
        """Check if one side of the node is a folder"""
        if side not in self._path:
            raise IndexError()
        return os.path.isdir(self._path[side])

    def invalidate(self):
        """Invalidate chached comparison info"""
        self._cmp = None
        for child in self.children:
            self.children[child].invalidate()

    # pylint: disable=too-many-branches, too-many-statements
    def cmp(self, check_flags=False):
        """Get comparison info for this node implies child nodes
        are checked as well.

        This information is cached, so multiple calls do
        not incure incresed cost"""
        if self._cmp:
            return self._cmp
        has_left = os.path.exists(self._path['left'])
        has_right = os.path.exists(self._path['right'])
        self._has_diff = False
        self._has_orphan = False

        if has_left and has_right:
            if self.isfile('left') and self.isfile('right'):
                if filecmp.cmp(self._path['left'], self._path['right'], False):
                    if check_flags:
                        if self.flags('left') == self.flags('right'):
                            self._cmp = 'match'
                        else:
                            self._has_diff = True
                            self._cmp = 'mismatch'
                    else:
                        self._cmp = 'match'
                else:
                    self._has_diff = True
                    self._cmp = 'mismatch'
            elif self.isdir('left') and self.isdir('right'):
                self._cmp = 'match'
                for child in self.children:
                    child_cmp = self.children[child].cmp()
                    if child_cmp != 'match':
                        self._has_diff = True
                        self._cmp = 'mismatch'
                    if self.children[child].hasorphan():
                        self._has_orphan = True
                    if self.children[child].hasdiff():
                        self._has_diff = True

                if check_flags and self._cmp == 'match':
                    if self.flags('left') == self.flags('right'):
                        self._cmp = 'match'
                    else:
                        self._has_diff = True
                        self._cmp = 'mismatch'
            else:
                self._cmp = 'mismatch'
                self._has_diff = True
                for child in self.children:
                    child_cmp = self.children[child].cmp()
                    if self.children[child].hasorphan():
                        self._has_orphan = True
                    if self.children[child].hasdiff():
                        self._has_diff = True
        elif has_left:
            self._has_diff = True
            self._has_orphan = True
            self._cmp = 'orphan-left'
        elif has_right:
            self._has_diff = True
            self._has_orphan = True
            self._cmp = 'orphan-right'
        else:
            self._cmp = 'missing'
        return self._cmp

    def _extend(self, child_path):
        """Add any items in the given path as children"""
        if os.path.isdir(child_path):
            for child in os.scandir(child_path):
                if child.name in self.children:
                    continue
                self.children[child.name] = DiffInfo(self._root['left'],
                                                     self._root['right'],
                                                     self.rel_path, child.name,
                                                     self.depth + 1)


@contextlib.contextmanager
def open_output(filename=None):
    """Open the main output stream, which may be stdout"""
    if filename is None or filename == '-':
        output = sys.stdout
        yield output
    else:
        with open(filename, 'w', encoding="utf-8") as output:
            yield output


# pylint: disable=too-many-branches, too-many-statements
def print_row(root, mode, indent=0):
    """Print one row of the diff report"""
    relation = root.cmp()
    result = ''

    if mode == 'All':
        pass
    elif mode == 'Diff' and root.hasdiff():
        pass
    elif mode == 'Orphan' and root.hasorphan():
        pass
    else:
        return ''

    result += '<tr class="SectionAll">'
    if relation == 'match':
        result += '<td class="DirItemSame">\n'
    elif relation == 'mismatch':
        result += '<td class="DirItemDiff">'
    elif relation == 'orphan-left':
        result += '<td class="DirItemOrphan">'
    elif relation == 'orphan-right':
        result += '<td>'
    elif relation == 'missing':
        result += '<td>'
    else:
        result += '<td>'
    if root.exists('left'):
        if root.isdir('left'):
            sym = '&#x1F4C2;'
        else:
            sym = '&#x1F5CE;'
        result += f'<span class="tree">{"&nbsp;&nbsp;" * indent}{sym}</span> {root.name}'
    result += '</td>'
    result += f'<td>{root.file_size("left")}</td>'
    result += f'<td>{root.flags("left")}</td>'

    if relation == 'match':
        sym = '='
        result += f'<td class="DirItemHeader AlignLeft">{sym}</td>'
    elif relation == 'mismatch':
        sym = '&#x2260;'
        result += f'<td class="DirItemHeader AlignLeft">{sym}</td>'
    elif relation == 'orphan-left':
        sym = ''
        result += f'<td class="DirItemHeader AlignLeft">{sym}</td>'
    elif relation == 'orphan-right':
        sym = ''
        result += f'<td class="DirItemHeader AlignLeft">{sym}</td>'
    elif relation == 'missing':
        sym = '&#x1F9A8;'
        result += f'<td class="DirItemHeader AlignLeft">{sym}</td>'
    else:
        sym = relation
        result += f'<td class="DirItemHeader AlignLeft">{sym}</td>'

    if relation == 'match':
        result += '<td class="DirItemSame">'
    elif relation == 'mismatch':
        result += '<td class="DirItemDiff">'
    elif relation == 'orphan-left':
        result += '<td>'
    elif relation == 'orphan-right':
        result += '<td class="DirItemOrphan">'
    elif relation == 'missing':
        result += '<td>'
    else:
        result += '<td>'
    if root.exists('right'):
        if root.isdir('right'):
            sym = '&#x1F4C2;'
        else:
            sym = '&#x1F5CE;'
        result += f'<span class="tree">{"&nbsp;&nbsp;" * indent}{sym}</span> {root.name}'
    result += '</td>'
    result += f'<td>{root.file_size("right")}</td>'
    result += f'<td>{root.flags("right")}</td>'
    result += '</tr>'
    return result


def print_tree(root, mode, indent=0):
    """Print root item and all children for the report"""
    result = ''
    if indent:
        result += print_row(root, mode, indent)
    for child in sorted(root.children):
        result += print_tree(root.children[child], mode, indent + 1)
    return result


def write_report(root, title, mode):
    """Print report

    Supported modes are:

    All: Include all nodes

    Diff: Include only nodes with diffrences

    Orphan: Include only orphan nodes
    """
    now = datetime.datetime.now()
    result = ''
    if mode == 'All':
        mode_name = 'All'
    elif mode == 'Diff':
        mode_name = 'Differences'
    elif mode == 'Orphan':
        mode_name = 'Orphans'
    else:
        mode_name = mode

    result += """<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<style>
.AlignLeft { text-align: left; }
.AlignCenter { text-align: center; }
.AlignRight { text-align: right; }
body { font-family: sans-serif; font-size: 11pt; }
td { vertical-align: top; padding-left: 4px; padding-right: 4px; }

tr.SectionGap td { font-size: 4px; border-left: none; border-top: none; border-bottom: 1px solid Black; border-right: 1px solid Black; }
tr.SectionAll td { border-left: none; border-top: none; border-bottom: 1px solid Black; border-right: 1px solid Black; }
tr.SectionBegin td { border-left: none; border-top: none; border-right: 1px solid Black; }
tr.SectionEnd td { border-left: none; border-top: none; border-bottom: 1px solid Black; border-right: 1px solid Black; }
tr.SectionMiddle td { border-left: none; border-top: none; border-right: 1px solid Black; }
tr.SubsectionAll td { border-left: none; border-top: none; border-bottom: 1px solid Gray; border-right: 1px solid Black; }
tr.SubsectionEnd td { border-left: none; border-top: none; border-bottom: 1px solid Gray; border-right: 1px solid Black; }
table.dc { border-top: 1px solid Black; border-left: 1px solid Black; width: 100%; font-family: sans-serif; font-size: 10pt; }
table.dc tr.SectionBegin td { border-bottom: 1px solid Silver; }
table.dc tr.SectionMiddle td { border-bottom: 1px solid Silver; }
td.DirItemHeader { color: #000000; background-color: #FFFFFF; background-color: #E7E7E7; padding-top: 8px; }
td.DirItemDiff { color: #FF0000; background-color: #FFFFFF; }
td.DirItemNewer { color: #FF0000; background-color: #FFFFFF; }
td.DirItemOlder { color: #808080; background-color: #FFFFFF; }
td.DirItemOrphan { color: #0000FF; background-color: #FFFFFF; }
td.DirItemSame { color: #000000; background-color: #FFFFFF; }
.DirSegInfo { color: #C0C0C0; }
.tree {
    font-family: monospace;
    font-size: 16pt;
}
</style>"""
    result += f"""<title>{title}</title>
</head>
<body>
{title}<br/>
Produced: {now.strftime("%Y-%m-%d %H:%M:%S")}<br/>
&nbsp; &nbsp;
<br/>
Mode:&nbsp; {mode_name} &nbsp;
<br/>
Left base folder:  {os.path.abspath(root.path('left'))}&nbsp;
<br/>
Right base folder:  {os.path.abspath(root.path('right'))}&nbsp;
<br/>
<table class="dc" cellspacing="0" cellpadding="0">
<tr class="SectionAll">
<td class="DirItemHeader">Name</td>
<td class="DirItemHeader">Size</td>
<td class="DirItemHeader">Flags</td>
<td class="DirItemHeader AlignLeft">&nbsp;</td>
<td class="DirItemHeader">Name</td>
<td class="DirItemHeader">Size</td>
<td class="DirItemHeader">Flags</td>
</tr>"""
    result += print_tree(root, mode, 0)
    result += """</tr>
</table>
<br/>
</body>
</html>"""
    return result


def main():
    """Main entrypoint"""
    parser = argparse.ArgumentParser(
        description=globals()['__doc__'],
        formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('left', action='store')
    parser.add_argument('right', action='store')
    parser.add_argument('--mode',
                        '-m',
                        default="All",
                        action='store',
                        choices=['All', 'Diff', 'Orphan'])
    parser.add_argument('--output', '-o', default=None, action='store')
    parser.add_argument('--title', '-t', default="BOM Diff", action='store')

    args = parser.parse_args()
    root = DiffInfo(args.left, args.right)
    root.cmp()
    report = write_report(root, args.title, args.mode)
    with open_output(args.output) as output:
        output.write(report)


if __name__ == "__main__":
    main()
