
{
    % if event.interpretString %
}
{
    ModelInsertCommand insertCommand(&rootModel);
    insertCommand.setStartRow(0);
    insertCommand.interpret({{event.interpretString}});
    insertCommand.doCommand();
}
{
    % endif %
}
