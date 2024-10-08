import subprocess
subprocess.call("doxygen", shell=True)

extensions = ['breathe']

breathe_projects = {'NovaPhysics' : 'xml'}
breathe_default_project = 'NovaPhysics'

# The master toctree document.
master_doc = "index"

# Suffix of source files.
source_suffix = ".rst"

# General information about the project.
project = "Nova Physics"
copyright = "2024, Kadir Aksoy"
author = "Kadir Aksoy"

html_static_path = ["_static"]
html_theme = "furo"
html_title = "Nova Physics"

html_theme_options = {
    "light_logo": "nova-light.png",
    "dark_logo": "nova-dark.png",

    "sidebar_hide_name": True,
}