
{
    ModelRemoveCommand removeCommand(&rootModel);
    removeCommand.setAncestorRowNumbers({{event.rowAncestors}});
    removeCommand.setStartRow({{event.start}});
    removeCommand.setEndRow({{event.end}});
    removeCommand.doCommand();
}
