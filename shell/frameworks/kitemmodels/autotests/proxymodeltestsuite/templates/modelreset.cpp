
{
    ModelResetCommand resetCommand(&rootModel);
    resetCommand.setInitialTree({{event.interpretString}});
    resetCommand.doCommand();
}
