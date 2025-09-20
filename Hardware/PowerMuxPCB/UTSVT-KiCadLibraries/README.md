# Common Libraries
This repository contains the footprint and symbol libraries that are used among us, the Longhorn Racing Solar Vehicles Team. 
In order to work on the hardware for our system, please follow the below standard practices when designing a new board.

# Using this Repository
In order to use the files in this repository, you must:
1. Add this repository as a submodule to the PCB repository. It will then show up as a folder within the PCB repository.
2. Create the PCB KiCAD Project and open it in KiCAD.
3. Click on `Preferences` -> `Manage Symbol Libraries` -> `Project Specific Libraries` and add the `.lib` files (within the submodule directory) to the path.
    
    **NOTE**: Ensure all `.lib` files are named `utsvt-name`

# Board Repositories
* All board repositories should contain this repository as a submodule.
* All board repositories should be named Team-BoardName (ex. BPS-LeaderPCB) - all board names should end in PCB
* All board repositories should have zip file of gerbers for the board
* All board repositories should contain pdf with images of board schematic and board layout (with dimensions)
* All board repositories should have `BoardName-backups/*` directory in their `.gitignore` file to ignore all backups generated.
* All board repositories should have README that contains the following information
    * Boards Purpose
    * Mouser link BOM for Board
    * List of components that take up vertical/horizontal volume outside of regular board dimensions (for mechanical purposes)
    * List of LED's on board and what information they provide

# Pull Request Template
The file PULL_REQUEST_TEMPLATE.md should be copied into the main board repository directory. This file creates a checklist of 
things the hardware engineer must complete after opening a PR. After submitting a PR, you must ensure this checklist is completed. Only after
the checklist is completed will a reviewer review your board.

    **NOTE**: After making changes to your board, it is essential that you re-request a review from the reviewers. Otherwise they will not know when
to review your design again. YOU MUST BE PROACTIVE IN YOUR WORK!

When first creating your board repository, and every time the Pull Request Template is updated, you must run the following command
`cp common/PULL_REQUEST_TEMPLATE.md .` to copy the Pull Request Template into the board repository.

# Adding Components to library
1. Add component information to Standardized Mouser Components Excel Sheet (this is so other designs can take this component into consideration if the use is applicable).
    * Be sure to include component name, mouser cart link, and footprint.
2. Open the PCB that you are working on in KiCAD (under its repository)
3. Click on `Symbol Editor`, select a `UTSVT` library to add it to, and click on `Add a New Symbol`.
4. Download component symbol and footprint (or make one yourself using symbol/footprint editor in KiCAD)
    * You may not need to download footprints/symbols for some components. Some footprints/symbols will work for multiple components if they are same package/pinout.
5. If component is large (vertically or horizontally), download 3d model for component.
6. All symbols should have the following information filled out when added:
    * Name
    * Datasheet/Mouser link
    * Footprint Association
7. If adding footprint, click on `Footprint Editor`, select a `UTSVT` folder to add it to.
8. If component takes up vertical or horizontal space (buck converters, connectors, large capacitors, etc.), follow the below steps:
    1. Download footprint and store in the submodule through your PCB repository
    2. Open KiCAD through your project file in your PCB repository. Select `Preferences` -> `Configure File Paths` -> `+` -> and add the following path `$(KIPRJMOD)/common/utsvt3d/`.
    3. Now when adding the 3D Model to the footprint, just select the footprint through the footprint library, click on 3D Models, and add the name of the step file (through one of the folders in `utsvt3d`).
9. After filling out the above information, enter the submodule directory through your PCB repository.
10. There will be changes made to this submodule if everything was done correctly. Now you must commit your changes to a new branch and open a Pull Request.
11. After the Pull Request is merged, update the submodule in your PCB repository through `git submodule update`.

# Standard Constraints for Laying out Board
When manufacturing a PCB, there are constraints that are given to us by the manufacturer based on how precise their manufacturing equipment is. The following are [**JLCPCB Manufacturing Capabilities**](https://jlcpcb.com/capabilities/Capabilities).  
When creating the layout of your board, select
`File` -> `Board Setup` -> `Constraints` and set them to the following values:
* Minimum Clearance (2 layer): .127mm
* Minimum Track Width (2 layer): .127mm
* Minimum Clearance (4 layer): .09mm
* Minimum Track Width (4 layer): .09mm
* Minimum Connection Width: 0mm
* Minimum Annular Width: .13mm
* Minimum Via Diameter (2 layer): .5mm
* Minimum Via Diameter (4 layer): .25mm
* Copper to Hole Clearance: .254mm
* Copper to Edge Clearance: .3mm
* Minimum Through Hole: .3mm
* Hole to Hole Clearance: .5mm
* Minimum Item Clearance: .15mm
* Minimum Text Height: 1mm
* Minimum Text Thickness: .153mm

# Exporting a Board as a 3D Model
Before exporting a board go into the 3D viewer of KiCAD `View` -> `3D Viewer` while in the PCB editor to see what you will be exporting. 
* For mechanical integration, mechanical doesn't need to see every individual component.  If your board doesn't have any smd parts that take up vertical or horizontal space then while in the 3D viewer do `Preferences` -> `Toggle SMD 3D Models` the same applies for any tht component except it's `Preferences` -> `Toggle Through Hole 3D Models`.  
* Go to `File` -> `Export` -> `Step` and select the directory you want to export to and in the `Coordinantes` section make sure `Board Center Origin` is selected.
* Press `Export`.

# Working on a Board
* The first page of the KiCAD schematic should have a version control description of the board that is being worked on. It should list the changes between each version of the board and changes that should be made in future versions of the board.
* When working on a board, the branch name should be the version of the board.
* Versions are updated every time the layout of the board is changed after a fabrication.
* Everyone working on boards should download the Interactive HTML BOM plugin for KiCAD. After laying out the board, an interactive BOM should be generated so soldering the board is easily managed.