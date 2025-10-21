# Welcome to the KiCad Sharepoint Documentation
Welcome to the LHRs KiCad Sharepoint! KiCad-Sharepoint contains the shared symbols, footprints, and design blocks used to develop our custom PCBs for our solar-powered vehicle. Additionally, this documentation contains helpful guides for setting up and using KiCad with our standardized design process. Any inaccurate or outright wrong information should be blamed on our Platform Lead, Lakshay Gupta. 
## Where to Start
Install and set up KiCad according to these [instructions](./KiCad-Setup.md/). Before starting a PCB project, make sure to review our expectations for design [standards](./Standards.md/) and [workflow](./Workflow.md/). If you have any questions, don't hesitate to ask your leads. Happy building :)

## How to use KiCAD Sharepoint
In order to use KiCAD Sharepoint into your project you need to do these three things:

- Add KiCAD Sharepoint as a submodule, instructions can be found here
- Add KiCAD Sharepoint symbols to your symbol table, instructions can be found [here](Workflow.md#Symbol-Table)
- Add KiCAD Sharepoint footprints to your footprint table, instructions can be found [here](./Workflow.md)

## Contributing to the docs

### Installing python dependencies
We use ``mkdocs`` to host our documentation, which requires several python packages.  
In your terminal and home directory of KiCAD-Sharepoint, run:
```sh
python3 -m venv .venv

# On Windows, run:
.venv\Scripts\activate

# On Linux or MacOS, run:
source .venv/bin/activate

pip install -r requirements.txt

```

### Viewing the docs
While in your python virtual environment (after running ```source .venv/bin/activate```), run:
```sh
mkdocs serve
```

From there, mkdocs will host a preview of the documentation on a local port, usually ```http://127.0.0.1:8000/```. The specific link can be found where it says "Serving on"

### Deploying the documentation
To deploy your changes to main KiCad-Sharepoint site, push your changes to the main branch (through a pull request), which will run a github workflow to deploy the docs to our site. For test viewing your changes the best way to do it is to build and serve the docs
```sh
mkdocs build
mkdocs serve
```