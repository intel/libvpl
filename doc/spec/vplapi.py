# SPDX-FileCopyrightText: 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT

__version__ = '0.0.1'

import re

from docutils import nodes
from docutils.parsers.rst import Directive

from sphinx.locale import _
from sphinx.util.docutils import SphinxDirective

from sphinx.util import logging

logger = logging.getLogger(__name__)

def type_of_node(n):
    return type(n).__name__

# <definition_list_item>                         --> def_node
#     <term>                                     --> def_node[0]
#         <strong>Since</strong>                 --> def_node[0][0].astext()
#     </term>
#     <definition>                               --> def_node[1]
#         <paragraph>her 1.0 her. </paragraph>   --> def_node[1][0].astext()
#     </definition>
# </definition_list_item>

def check_definition_list_item(def_node):
    if type_of_node(def_node[0]) == 'term' and type_of_node(def_node[1]) == 'definition':
        if(def_node[0][0].astext() == 'Since'):
            potential_ver_str = def_node[1][0].astext()
            potential_m = re.search(r'since API version (\d+\.\d+)\.',potential_ver_str)
            if potential_m:
                return potential_m.group(1)
    
    return None

def process_function_nodes(app, doctree, fromdocname):
    if fromdocname.find("VPL_") >= 0:
        for section in doctree.traverse(nodes.section, descend=True):
            for n in section:
                type_of_section = type_of_node(n)
                if type_of_section == 'title':
                    title_text = n[0]
                    if(title_text.startswith("MFX")): # this is function description section
                        # lets search for the `Since` term
                        isFunction = False
                        hasVersion = False
                        version = None
                        functionName = title_text
                        for k in n.traverse(condition=None, include_self=True, descend=True, siblings=True):
                            node_type = type_of_node(k)
                            if node_type == 'desc':
                                if k.hasattr('desctype'):
                                    if k.get('desctype') == 'function':
                                        isFunction = True
                                    else:
                                        continue
                            if node_type == 'definition_list_item':
                                tmp = check_definition_list_item(k)
                                if tmp:
                                    version = tmp
                                    hasVersion = True
                        if isFunction and not hasVersion:
                            logger.warning("Function {} doesn't have min API version defined".format(functionName))

def setup(app):
    app.connect('doctree-resolved', process_function_nodes)
    return {
        'version': __version__,
        'parallel_read_safe': True,
        'parallel_write_safe': True
    }
