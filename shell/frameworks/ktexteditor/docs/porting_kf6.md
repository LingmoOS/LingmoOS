# Porting to KF6KTextEditor

This document describes the changes in the api and what you need to be able to port to KF6 KTextEditor.

## Interfaces are gone

All KTextEditor::Document and KTextEditor::View extension interfaces for e.g., MovingInterface, MarkInterface, ConfigInterface etc were
removed and their API merged into respective view and document class. For porting, you can just remove the interface case and use the
document or view object directly, e.g:
```c++
// KF5
if (auto miface = qobject<KTextEditor::MovingInterface*>(m_doc)) {
    KTextEditor::MovingRange *range = miface->newMovingInterface(...);
}
```
in KF6 becomes:

```c++
    KTextEditor::MovingRange *range = m_doc->newMovingInterface(...);
```
