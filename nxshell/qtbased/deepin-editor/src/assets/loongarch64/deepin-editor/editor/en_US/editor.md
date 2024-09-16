# Text Editor|deepin-editor|

## Overview

Text Editor is a simple text editing tool. You can use it to write a simple text document, or make it a code editing tool with its advanced features which support code syntax highlighting.

![overview](fig/overview.png)

## Guide

### Run Text Editor

1. Click the Launcher icon ![deepin_launcher](../common/deepin_launcher.svg) in the Dock to enter the Launcher interface. 
2. Locate Text Editor ![deepin_editor](../common/deepin_editor.svg)by  scrolling the mouse wheel or searching "text editor" in the Launcher interface and click it to run.
3. In Launcher, right-click Text Editor and you can:

  - Click **Send to desktop** to create a desktop shortcut.
  - Click **Send to dock** to fix it in the Dock.
  - Click **Add to startup** to add it to startup and it will automatically run when system boots.

### Exit Text Editor

- On the Text Editor interface, click ![close](../common/close_icon.svg) to exit.
- On the Text Editor interface, click ![icon_menu](../common/icon_menu.svg) > **Exit** to exit.
- Right-click ![deepin_editor](../common/deepin_editor.svg) in the Dock and select **Close All** to exit.

### View Shortcuts

On the Text Editor Interface, press **Ctrl + Shift + ?** to view all the shortcuts. You can also check shortcuts in Settings. Proficiency in shortcuts will greatly improve your efficiency.

![1|hotkey](fig/hotkey.png)

## Basic Operations

### Manage Tabs

- Create New Tabs/Windows
  - Click ![plus_icon](../common/+.svg) or ![icon_menu](../common/icon_menu.svg)> **New tab** on the title bar or press **Ctrl + T** to create a new tab.
  - Click ![icon_menu](../common/icon_menu.svg)> **New window** or press **Ctrl + N** to create a new window.

- Adjust Tabs/Windows
  - Drag the tab in the same window to reorder tabs.
  - Scroll the mouse wheel on the title of a tab to reorder tabs in the same window.
  - Move the tab out of the window to create a new window or move the tab from one window to another.

- Close Tabs /Windows

  The window will be closed as well when there is only one tab in the window.

  - Press **Ctrl + W** to close the file on the file interface.
  - Move the cursor to the file title, and click the ![close](../common/close_icon.svg) icon there or the mouse middle button to close the file.
  - Right-click the file title and select **Close tab** or **Close other tabs** or **More options** to close the tabs.

> ![notes](../common/notes.svg) Notes: If you make changes to the file but does not save it, you will be prompted by Text Editor to save before closing the file.

### Open Files

You can open one or multiple text files at one time in the following ways, and the selected file will open in the new tab:

- Drag the file directly to the Text Editor interface or the Text Editor icon.
- Right-click the file and select **Open with** > **Text Editor**. After selecting Text Editor as the default application for text files in Control Center, you can double-click the file to open it directly.
- On the Text Editor interface, click ![icon_menu](../common/icon_menu.svg)> **Open file**,  or use the shortcuts **Ctrl + O** to open the file in File Manager.

### Save Files

- Press **Ctrl + S** or click ![icon_menu](../common/icon_menu.svg)> **Save** to save the current document.
- Press **Ctrl + Shift + S** or  click ![icon_menu](../common/icon_menu.svg)> **Save as** to save the file as a new one.

### Print Files

To print a file in Text Editor, you should connect and set up the printer first.

1. Click ![icon_menu](../common/icon_menu.svg)> **Print**, or press **Ctrl + P** to open the print preview interface.
2. On the preview interface, preview the document，select a printer and set the printing page. 
3. Click the **Advanced** option on the preview interface to select parameters including paper size and layout.  
4. Click **Print** to start printing.

![preview](fig/preview.png)

![pagesetup](fig/advanced-preview.png)

## Edit Texts

### Move Cursor

In addition to the arrow keys and mouse clicks, you can also use the following shortcuts to move the cursor quickly:

| Function   |  Shortcuts |
| --------------- | ------------ |
| Save cursor position | Ctrl + Shift + > |
| Reset cursor position (Jump to cursor last saving position) | Ctrl + Shift + < |
| Forward one word (Jump forward over one word) | Ctrl + Right |
| Backward one word (Jump backward over one word) | Ctrl + Left |
| Move to end of line | End |
| Move to start of line | Home |
| Move to end of text | Ctrl + End |
| Move to start of text | Ctrl + Home |
| Move to line indentation | Ctrl + M |
| Forward over a pair (Jump forward after the right parenthesis) | Alt + P |
| Backward over a pair (Jump backward before the left parenthesis) | Alt + N |

### Change Cases

It helps you change the case of selected text. You can change it to lowercase, uppercase, or capitalize the initial.

1. Select the text.
2. Right-click and select **Change Case**.
3. Choose **Upper Case**, **Lower Case**, or **Capitalize**. The changes will take effect immediately.

You can also use **Alt** + **U / L / C** to quickly switch the cases.

![changecase](fig/d_changecase.png)

### Highlight Texts
Text Editor supports highlighting different kinds of texts.

Click the triangle symbol at the far right corner of the status bar at the bottom, and select the type of text you want to highlight. The corresponding contents in the text will be highlighted automatically.

![highlight](fig/highlight.png)

### Delete Texts

In addition to deleting characters one by one, you can quickly delete characters by using the following shortcuts:

| Function   |  Shortcuts |
| --------------- | ------------ |
| Delete to end of line | Ctrl + K |
| Delete current line | Ctrl + Shift + K |
| Delete one word backward | Alt + Shift + N |
| Delete one word forward | Alt + Shift + M |

### Undo

If you make an incorrect operation, press **Ctrl + Z** to undo it, or right-click to select **Undo**.

### Find Texts

1. Click ![menu](../common/icon_menu.svg) and select **Find**, or press **Ctrl** + **F** to open the "Find" box below.
2. Input the text you want to find. 
3. Click **Previous** /**Next** to find each match, or press the **Enter** key to find the next match.
4. Press the **Esc** key or click the close icon to close the dialog box.

> ![tips](../common/tips.svg)Tips: Select the text and conduct search. The text selected will be automatically displayed in the **Find** box.

### Replace Texts
1. Click ![menu](../common/icon_menu.svg), and select **Replace**, or press **Ctrl + H** to open the **Replace** box below.
2.  Input the text to be replaced and the new text.
3.  Click **Replace** to replace the matches one by one and click **Replace Rest** and **Replace All** to replace the rest or all the matches at once. Click **Skip** to skip the current matching text.
4.  Press **Esc** or click the close icon to close the dialog box.

### Go to Line
Use Go to Line to jump to the specific line directly.
Right-click and select **Go to Line**, or press **Ctrl + G** and then input the line number to go to that line.

### Edit Line

Use the shortcuts below to edit lines easily:

| Function  |  Shortcuts |
| ------------ | ------------ |
| New line above (Insert one line above) | Ctrl + Enter |
| New line below (Insert one line below) | Ctrl + Shift + Enter |
| Duplicate line | Ctrl + Shift + D |
| Swap line up (Swap the current line with above line) | Ctrl + Shift + Up |
| Swap line down (Swap the current line with below line)  | Ctrl + Shift + Down |
| Scroll up one line | Super + Shift + Up |
| Scroll down one line | Super + Shift + Down |
| Mark | Alt + H |
| Unmark | Alt + Shift + H |
| Copy line | Super + C |
| Cut line | Super + X |
| Merge lines | Ctrl + J |

### Enable/Disable Read-only Mode

1. Open the document with Text Editor and right-click to select **Turn on Read-only Mode**.

![readonly](fig/d_readonly.png)
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;

2.  Right-click to select **Turn off Read-only Mode** under the read-only mode.

![read-only](fig/d_turnoffreadonly.png)
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;

### Use Column Mode

Press and hold the **Alt** key on the keyboard, drag the mouse to select multiple lines, and edit the identical contents in multiple lines of codes efficiently by using the column mode function. 

![mode](fig/column mode.png)

### Color Mark

Select the text to be marked in Text Editor, right-click to select **Color Mark** and select the corresponding options in the drop-down list as needed.

![colormark](fig/d_colormark.png)
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;

- Select **Mark** to mark a certain line or a paragraph of texts. You can choose from 8 colors displayed through icons.
   - Place the cursor at any line and you can mark the current line.
   - Select a paragraph of texts and you can mark the selected texts.
- Select **Mark All** to mark all the contents in Text Editor, or mark all the matching contents of selected texts in Text Editor. You can choose from 8 colors displayed through icons.
   - Place the cursor at any line and you can mark all contents.
   - Select any content and you can mark the same content in the whole document. 
- Select **Clear Last Mark** to cancel the last mark operation.
- Select **Clear All Marks** to cancel all the current marks.


### Manage Bookmarks 

Check **Show bookmarks icon** in **Settings** and you can add a bookmark for any line on the Text Editor interface. The icon ![icon](../common/bookmark_normal_light.svg) shows up when you move the cursor to any line in the left column, and the icon ![bookmarkbig](../common/bookmarkbig_checked_light.svg) will appear after a bookmark is successfully added.

- Add bookmarks

   + Click ![icon](../common/bookmark_normal_light.svg) to add a bookmark for the line.
   + Right-click ![icon](../common/bookmark_normal_light.svg) and select **Add bookmark** to  add a bookmark for the line.
   + Place the cursor on any line and press **Ctrl + F2** to add a bookmark for the line.

-   Delete bookmarks  

   + Click the colored bookmark icon ![bookmarkbig](../common/bookmarkbig_checked_light.svg) to delete the bookmark of the line directly.
   + Right-click the colored bookmark icon ![bookmarkbig](../common/bookmarkbig_checked_light.svg) to select **Remove Bookmark** to delete the bookmark in the line.
   + Right-click a bookmark to select **Remove All Bookmarks** to delete all the bookmarks in the text.
  >![Notes](../common/notes.svg)Notes: The bookmark icon appears only when cursor is hovered over to the left of the line number in the left column; when it's moved beyond the left bookmark column, the bookmark icon will disappear. The bookmark icon will always be displayed after being added.


### Manage Comments 

You can add comments to any text with a code type suffix, such as cpp and java, among others.

- Add comments
  - Select the text needed and right-click to select **Add Comment**.
  - Select the text needed and press **Alt + A** to add comments.

- Cancel comments
     - Select the text with comments, and right-click to select **Remove Comment**.
- Select the text with comments, and press **Alt + Z** to cancel comments.


> ![notes](../common/notes.svg)Notes: This supports comments in different languages. Please refer to actual conditions for detailed information. For example, comment symbol for C, C# and Java is // and comment symbol for Python is #.

## Main Menu

On the main menu, you can Create New Windows, Create New Files, [Find Texts](#Find Texts), [Replace Texts](#Replace Texts), [Save Files](#Save Files), switch window themes, view help manual, and get more information about Text Editor.

### Settings

You can set the basic information, shortcuts and advanced information in settings.

#### Basic Settings

Click ![icon_menu](../common/icon_menu.svg)> **Settings** and you can perform the following in the Settings window:

- Select the Font and Font Size.
- Check or uncheck **Word wrap**.
- Check **Code folding flag** and ![next](../common/next.svg) or ![next](../common/next_down.svg) are displayed in the edit area. Right-click to select **Fold/Unfold Current Level** or **Fold/Unfold All** to perform the corresponding operations.
- Check **Show line numbers** to display the line number in the edit area.
- Check **Highlight current line** to highlight the current line.
- Check **Show bookmarks icon** to display the bookmarks in the text.
- Check **Show whitespaces and tabs** to display all the white spaces and tabs. You can also input or delete white spaces and tabs manually.

> ![tips](../common/tips.svg)Tips: You can also use **Ctrl + “+”/ “-”** to adjust the font size, and press **Ctrl + 0** to restore default font size.

#### Shortcuts Settings
1. Click ![icon_menu](../common/icon_menu.svg)> **Settings** to view the current shortcut in the **Shortcuts** option.
2. Select a keymap in **Shortcuts** and view the shortcuts.
3. You can click and customize new shortcuts.

> ![notes](../common/notes.svg)Notes: You can choose a proper keymap to fit your habits, including standard keymap, Emacs keymap, or customize it as you like.

#### Advanced Settings

1. Click ![icon_menu](../common/icon_menu.svg)> **Settings**.
2. In the **Advanced** option, you can set:
   - Window size: Normal, Maximum, or Full screen;
   - Tab width: The indentation width when pressing Tab.


### Theme

The window theme provides three theme types, namely Light Theme, Dark Theme and System Theme.

1. On the Text Editor interface, click ![icon_menu](../common/icon_menu.svg).
2.  Click **Theme** to select one theme.

### Help

1.  On the Text Editor interface, click ![menu](../common/icon_menu.svg).
2.  Select **Help** to view the manual of Text Editor.

### About

1. On the Text Editor interface, click ![menu](../common/icon_menu.svg).
2.  Select **About** to view the version and introduction of Text Editor.

### Exit

1. On the Text Editor interface, click ![menu](../common/icon_menu.svg).
2. Click **Exit** to exit Text Editor.  
