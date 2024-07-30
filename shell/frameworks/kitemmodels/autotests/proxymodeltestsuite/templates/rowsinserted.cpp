
{
    ModelInsertCommand insertCommand(&rootModel);
    insertCommand.setAncestorRowNumbers({{event.rowAncestors}});
    insertCommand.setStartRow({{event.start}});
    {
        % if event.interpretString %
    }
    insertCommand.interpret({{event.interpretString}});
    { % else % } insertCommand.setEnd({{event.end}});
    { % endif % } insertCommand.doCommand();
}
