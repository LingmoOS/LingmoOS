# Print Manager|dde-printer|

## Overview

Print Manager is an easy-to-use CUPS-based tool that manages several printers at a time with a simple interface providing users with great convenience to add printers and install drivers quickly.

## Guide

You can run, close or create a shortcut for Print Manager as follows.

### Run Print Manager

1. Click ![deepin_launcher](../common/deepin_launcher.svg) on the Dock to enter the interface of launcher.
2. Locate ![dde_printer](../common/dde_printer.svg) by scrolling the mouse wheel or searching "Print Manager" in the Launcher interface and click it to run. 
3. Right-click![dde_printer](../common/dde_printer.svg) to:
   - Select **Send to desktop** to create a shortcut on the desktop.
   - Select  **Send to dock** to fix it onto the dock.
   - Select **Add to startup** to run it automatically when the computer is turned on.

### Exit Print Manager

- On the main interface, click  ![close_icon](../common/close.svg) to exit.
- Right-click ![dde_printer](../common/dde_printer.svg) on the Dock, select **Close all** to exit.
- On the main interface, click ![icon_menu](../common/icon_menu.svg)  and select **Exit** to exit.



## Add Printers
You can add printers via network or USB connection.

### Add printers via network

#### Find Printer

You can add printers via USB connection directly. Or, on the main interface, click![add_normal](../common/add_normal.svg) and you will have three options including **Discover Printer**, **Find Printer**, and **Enter URI**. 



**Discover Printer**

1. Click **Discover Printer** to load printer list automatically. Select the printer you want to add.

   ![0|2automaticsearch](fig/2automaticsearch.png)

2. Check the printer you want to add, the system will automatically find the proper driver.

   - Driver matched successfully: Click **Install Driver** to enter the installation interface.
   - Driver match failed: select **Select a driver** or click the website link to find, download and install the correct driver. Click **Next** to go to the driver selecting interface.

**Find Printer**

1. Click **Find Printer** , input user name or IP address (XX.XX.XX.XX) and click **Find** to search printers through different protocols. 

   ![0|3manualsearch](fig/3manualsearch.png)

2. Check the printer you want to add, the system will automatically find the proper driver.

   - Driver matched successfully: Click **Install Driver** to enter the installation interface.
   - Driver match failed: select **Select a driver** or click the website link to find, download and install the correct driver. Click **Next** to go to the driver selecting interface.

**Enter URI**

If no printers are found by  **Discover Printer** and  **Find Printer** , you can find and install printer driver by entering URI.  

1. Click **Enter URI** and input printer's URI.

   ![0|4URIsearch](fig/4URIsearch.png)

2. "Select a driver" is set by default. Or, you can click the website link to find, download and install the correct driver. 

3. Click **Next** to go to the driver selecting interface. 


#### Select drivers

1. Default driver: The system will choose recommended driver by default if there is a correct driver for the printer you select. 

2. Select a driver: you can choose "select a driver". Sources of drivers include: 


   * Local driver: select vendor and model from drop-down box to query local drivers.

     ![0|5localdriver](fig/5localdriver.png)

   * Local PPD files: drag and drop local PPD files here or click **Select a PPD file** to find one in local folder, for example, you can select a PPD file under the directory of /usr/share/ppd.  

   > ![notes](../common/notes.svg)Notes: Prerequisite of this operation is that user has installed driver locally. If not, you will get a hint "Driver install failed".

   ![0|6ppddocument](fig/6ppddocument.png)

   * Search for a driver: input particular vendor and model, and the system will search in background driver library. Results will be listed in the drop-down box. 

   ![0|7searchdriver](fig/7searchdriver.png)

#### Install Printer

After you add a printer and select the correct driver, click **Install Driver** to enter installing interface.  

- Successfully Installed 
  A window pops up saying the printer is successfully installed, in which you can click **Print Test Page** to verify if printing works normally, or, you can click **View Printer** to enter the interface of Print Manager. 

  ![0|8successfulinstallion](fig/8successfulinstall.png)

- Install Failed
  
  You can choose to reinstall if you get a hint saying "Install Failed". 

### Add printers via USB connection

When the printer is connected to the computer through USB, the printer will be automatically added to Print Manager in background, and you can operate according to the configuration results.

> ![notes](../common/notes.svg)Notes: Only drivers for printers that have been configured before could be found, therefore, automatic adding of printers could be successful. 

- Configuration success: a notification of successful configuration will pop up. Click it to jump to Print Manager interface to view the details of the printer.

![usb_connection](fig/usb_connection.png)

- Configuration failure: the notification of configuration failure will pop up. Click it to jump to Print Manager interface and add drivers manually. Refer to [Add printers via network](#Add printers via network) for operations. 

## Print Manager Interface

Once a printer is added successfully, when you click the printer in the interface of Print Manager, you can see buttons on the right, which are "Properties", "Print Queue", "Print Test Page", "Supplies" and "Troubleshoot". 

![0|9printermanagement](fig/9printermanagement.png)

### Properties 

1. Click **Properties** to view all settings of the printer. You can view the info about the printer's driver, URI, location, description, color mode and resolution and so on.

3. Settable items include: paper source, paper type, paper size, duplex, margins, orientation, page order and binding. It can be set according to your actual needs. For example, the paper size can be A4, A5, B5 or A3, and the printing direction can be vertical, horizontal or anti horizontal.

> ![notes](../common/notes.svg)Notes: **Properties** are related to the printer model and its driver. You can set according to your practical situation. 

![0|10printersettings1](fig/10printersettings1.png)

### Print Queue

1. Click **Print Queue**, you can view all print jobs and information, including "All", "Print queue", and "Completed". **Print Queue** is displayed by default. 

2. Select a job, right-click and you can select **Cancel**, **Delete**, **Pause**, **Resume**, **Print first**,  and **Reprint**. 


![0|11rightclicksetting](fig/11rightclicksetting.png)

### Print Test Page

Click **Print Test Page** to test if printing is completed successfully. 

   - Print successfully: if printing is completed successfully, you can continue with other tasks.

   - Print failed: if printing fails, you can reinstall a printer or start troubleshooting. 

### Supplies

In the main interface of Print Manager, click **Supplies** to view the ink/toner status. If it's insufficient, it needs to be replaced.

### Troubleshoot

If printing fails, you can click **Troubleshoot**, the checklist includes following items: 

![0|12troubleshooting ](fig/12troubleshooting.png)

   - Check if CUPS server is valid.

   - Check if driver is valid.

   - Check if printer settings are ok, i.e. if printer is started up, and, if tasks are accepted.

   - Check if printer connection is valid.


### Right-click options 

Select the printer and right-click, you will have options such as "Rename", "Shared", "Enabled", "Accept jobs", and "Set as default". 

> ![notes](../common/notes.svg)Notes: Before sharing a printer, must sharing function in "Main Menu > Settings" be opened. Otherwise, it would not take effect even though it's selected in the tight-click menu.

![0|9rightclicksetting](fig/9rightclicksetting.png)

## Delete Printer

On the main interface, click![-](../common/edit_delete.svg) to delete selected printer. 

![0|14delete](fig/14delete.png)


## Main Menu

In the main menu, you can set basic servers, switch themes, view help manual, and so on.

### Settings

1. Click ![icon_menu](../common/icon_menu.svg)on the interface.

2. Click **Settings**.

3. Basic server settings include: 

   - Publish shared printers connected to this system: Allow printing from the Internet. Check this option to share the added printers to users in the same LAN.
   - Allow remote administration
   - Save debugging information for troubleshooting 

   ![0|13rightclicksettingmainmenu](fig/13rightclicksettingmainmenu.png)

### Theme

The window theme includes Light Theme, Dark Theme and System Theme.

1. Click ![icon_menu](../common/icon_menu.svg)on the interface.
2. Click  **Theme** to select one.

### Help

Click Help to get the manual, which will help you further know and use Print Manager.

1. Click ![icon_menu](../common/icon_menu.svg)on the interface.
2. Click **Help** to view the manual.


### About

1. Click ![icon_menu](../common/icon_menu.svg)on the interface.
2. Click **About** to view version information and introduction about Print Manager.

### Exit

1. Click ![icon_menu](../common/icon_menu.svg)on the interface.
2. Click  **Exit**.

