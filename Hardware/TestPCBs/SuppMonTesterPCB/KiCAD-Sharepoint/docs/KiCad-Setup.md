## KiCad Installation
For the 2025-2026 design cycle, LHRs Electrical members will use KiCad 9. Download the latest stable version [here](https://www.kicad.org/download/). 
## Required Plugins
There are a few plugins we use that aid in day-to-day development and will make your life a lot easier. You can access the Plugin and Content Manager (PCM) from the KiCad home screen as shown below.

![Plugin and Content Manager](img/PCM.png)
### Interactive HTML BOM (IBOM)
This allows you to generate a convenient Bill of Materials (BOM) listing with the ability to visually correlate and easily search for components and their placements on the PCB. It's useful when hand-soldering boards, since you can quickly find the location of each component you're placing. Install from the PCM as shown below. Make sure to click "Apply Pending Changes."

![IBOM Installation](img/IBOM-Install.png)

Once installed, you can launch the IBOM tool from pcbnew by clicking this icon in the top bar.

![IBOM Icon](img/IBOM-Icon.png)

Then modify any relevant settings and generate the IBOM. By default, it will be saved in your project directory as `ibom.html`.

![IBOM Generation](img/IBOM-Generate.png)
### KiCAD JLCPCB Tools
This plugin is very useful for working with our PCBA fab, [JLCPCB](https://jlcpcb.com/). It allows for one-click generation of gerbers, BOM, and CPL (component placement list) files, along with assigning LCSC (JLC supplier) parts for assembly.

Install this plugin by following the directions on [Github](https://github.com/Bouni/kicad-jlcpcb-tools#installation).

Once installed, you can launch the JLCPCB tool from pcbnew by clicking this icon in the top bar.

![JLC Icon](img/JLC-Icon.png)

To assign LCSC part numbers, simply select a part from the list, click "Select alike parts" in the right sidebar and then "Assign LCSC number." Then you can search for the LCSC number (or use [this website](https://yaqwsx.github.io/jlcparts/#/) for easy parametric search).

Now to generate production files simply click the generate button. The files will be located in the `jlcpcb/production_files` folder within your project directory.

![JLC Generation](img/JLC-Generate.png)

