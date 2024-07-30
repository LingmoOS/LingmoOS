
{
    ModelLayoutChangeCommand layoutChangeCommand(&rootModel);
    layoutChangeCommand.setInitialTree({{event.interpretString}});
    QList<ModelLayoutChangeCommand::PersistentChange> changes;
    {% for change in event.changes %
    }
    {
        ModelLayoutChangeCommand::PersistentChange change;
        change.oldPath = {{change.oldPath}};
        change.newPath = {{change.newPath}};
        changes.append(change);
    }
    { % endfor % } layoutChangeCommand.setPersistentChanges(changes);
    layoutChangeCommand.doCommand();
}
