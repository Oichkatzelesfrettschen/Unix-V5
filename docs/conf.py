# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'UnixV5 Modernized'
copyright = '2025, AI Agent (Jules)' # Year will be updated by CI/CD or manually if needed
author = 'AI Agent (Jules)'

version = '0.1'
release = '0.1'

import os
import sys
# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# sys.path.insert(0, os.path.abspath('.'))
# sys.path.insert(0, os.path.abspath('../..')) # To find project root for breathe

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'sphinx.ext.todo',
    'sphinx.ext.coverage',
    'sphinx.ext.mathjax',
    'sphinx.ext.ifconfig',
    'sphinx.ext.viewcode',
    'sphinx.ext.githubpages',
    'sphinx.ext.napoleon',
    'breathe', # Added Breathe
    'sphinx_rtd_theme', # Added Read the Docs theme extension
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

language = 'en'

# -- Breathe Configuration ---------------------------------------------------
# This path assumes that Doxygen generates XML in <project_root>/build/doxygen_output/xml
# And this conf.py is in <project_root>/docs/
# So, the relative path from 'docs/' to 'build/doxygen_output/xml' is '../build/doxygen_output/xml'
# This assumes CMake is configured to build in a 'build' directory at the project root.
# The Doxyfile's OUTPUT_DIRECTORY should be consistent with this.
# (e.g. OUTPUT_DIRECTORY = ../build/doxygen_output when Doxyfile is in project root)
# The CMakeLists.txt sets DOXYGEN_OUTPUT_DIR to ${CMAKE_BINARY_DIR}/doxygen_output
# If CMake configures into 'build', then CMAKE_BINARY_DIR is 'project_root/build'.
# So breathe_projects path should be relative from docs/conf.py to project_root/build/doxygen_output/xml
breathe_projects = {
    "UnixV5_Modernized": "../build/doxygen_output/xml"
}
breathe_default_project = "UnixV5_Modernized"
breathe_default_members = ('members', 'undoc-members') # Show all members by default


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme' # Changed to Read the Docs theme
html_static_path = ['_static'] # Standard for Sphinx, created by quickstart

# Example of theme options (specific to sphinx_rtd_theme)
html_theme_options = {
    'analytics_id': '',  #  Provided by Google in your dashboard
    'analytics_anonymize_ip': False,
    'logo_only': False,
    'display_version': True,
    'prev_next_buttons_location': 'bottom',
    'style_external_links': False,
    'vcs_pageview_mode': '',
    # 'style_nav_header_background': 'white',
    # Toc options
    'collapse_navigation': True,
    'sticky_navigation': True,
    'navigation_depth': 4,
    'includehidden': True,
    'titles_only': False
}

# -- Options for intersphinx extension ---------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/intersphinx.html#configuration

intersphinx_mapping = {
    'python': ('https://docs.python.org/3', None),
}

# -- Options for todo extension ----------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/todo.html#configuration

todo_include_todos = True
