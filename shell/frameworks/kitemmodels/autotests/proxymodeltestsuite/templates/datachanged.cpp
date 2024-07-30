
{
    ModelDataChangeCommand dataChangeCommand(&rootModel);
    dataChangeCommand.setAncestorRowNumbers({{event.rowAncestors}});
    dataChangeCommand.setStartRow({{event.start}});
    dataChangeCommand.setEndRow({{event.end}});
    dataChangeCommand.doCommand();
}
