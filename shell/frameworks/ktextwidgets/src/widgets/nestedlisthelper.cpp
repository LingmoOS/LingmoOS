/*
    Nested list helper
    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "nestedlisthelper_p.h"

#include <QKeyEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextList>

#include "ktextedit.h"

NestedListHelper::NestedListHelper(QTextEdit *te)
    : textEdit(te)
{
}

NestedListHelper::~NestedListHelper()
{
}

bool NestedListHelper::handleKeyPressEvent(QKeyEvent *event)
{
    QTextCursor cursor = textEdit->textCursor();
    if (!cursor.currentList()) {
        return false;
    }

    if (event->key() == Qt::Key_Backspace && !cursor.hasSelection() && cursor.atBlockStart() && canDedent()) {
        changeIndent(-1);
        return true;
    }

    if (event->key() == Qt::Key_Return && !cursor.hasSelection() && cursor.block().text().isEmpty() && canDedent()) {
        changeIndent(-1);
        return true;
    }

    if (event->key() == Qt::Key_Tab && (cursor.atBlockStart() || cursor.hasSelection()) && canIndent()) {
        changeIndent(+1);
        return true;
    }

    return false;
}

bool NestedListHelper::canIndent() const
{
    const QTextCursor cursor = topOfSelection();
    const QTextBlock block = cursor.block();
    if (!block.isValid()) {
        return false;
    }
    if (!block.textList()) {
        return true;
    }
    const QTextBlock prevBlock = block.previous();
    if (!prevBlock.textList()) {
        return false;
    }
    return block.textList()->format().indent() <= prevBlock.textList()->format().indent();
}

bool NestedListHelper::canDedent() const
{
    const QTextCursor cursor = bottomOfSelection();
    const QTextBlock block = cursor.block();
    if (!block.isValid()) {
        return false;
    }
    if (!block.textList() || block.textList()->format().indent() <= 0) {
        return false;
    }
    const QTextBlock nextBlock = block.next();
    if (!nextBlock.textList()) {
        return true;
    }
    return block.textList()->format().indent() >= nextBlock.textList()->format().indent();
}

bool NestedListHelper::handleAfterDropEvent(QDropEvent *dropEvent)
{
    Q_UNUSED(dropEvent);
    QTextCursor cursor = topOfSelection();

    QTextBlock droppedBlock = cursor.block();
    int firstDroppedItemIndent = droppedBlock.textList()->format().indent();

    int minimumIndent = droppedBlock.previous().textList()->format().indent();

    if (firstDroppedItemIndent < minimumIndent) {
        cursor = QTextCursor(droppedBlock);
        QTextListFormat fmt = droppedBlock.textList()->format();
        fmt.setIndent(minimumIndent);
        QTextList *list = cursor.createList(fmt);

        int endOfDrop = bottomOfSelection().position();
        while (droppedBlock.next().position() < endOfDrop) {
            droppedBlock = droppedBlock.next();
            if (droppedBlock.textList()->format().indent() != firstDroppedItemIndent) {
                // new list?
            }
            list->add(droppedBlock);
        }
        //         list.add( droppedBlock );
    }

    return true;
}

void NestedListHelper::processList(QTextList *list)
{
    QTextBlock block = list->item(0);
    int thisListIndent = list->format().indent();

    QTextCursor cursor = QTextCursor(block);
    list = cursor.createList(list->format());
    bool processingSubList = false;
    while (block.next().textList() != nullptr) {
        block = block.next();

        QTextList *nextList = block.textList();
        int nextItemIndent = nextList->format().indent();
        if (nextItemIndent < thisListIndent) {
            return;
        } else if (nextItemIndent > thisListIndent) {
            if (processingSubList) {
                continue;
            }
            processingSubList = true;
            processList(nextList);
        } else {
            processingSubList = false;
            list->add(block);
        }
    }
    //     delete nextList;
    //     nextList = 0;
}

void NestedListHelper::reformatList(QTextBlock block)
{
    if (block.textList()) {
        int minimumIndent = block.textList()->format().indent();

        // Start at the top of the list
        while (block.previous().textList() != nullptr) {
            if (block.previous().textList()->format().indent() < minimumIndent) {
                break;
            }
            block = block.previous();
        }

        processList(block.textList());
    }
}

void NestedListHelper::reformatList()
{
    QTextCursor cursor = textEdit->textCursor();
    reformatList(cursor.block());
}

QTextCursor NestedListHelper::topOfSelection() const
{
    QTextCursor cursor = textEdit->textCursor();

    if (cursor.hasSelection()) {
        cursor.setPosition(qMin(cursor.position(), cursor.anchor()));
    }
    return cursor;
}

QTextCursor NestedListHelper::bottomOfSelection() const
{
    QTextCursor cursor = textEdit->textCursor();

    if (cursor.hasSelection()) {
        cursor.setPosition(qMax(cursor.position(), cursor.anchor()));
    }
    return cursor;
}

void NestedListHelper::changeIndent(int delta)
{
    QTextCursor cursor = textEdit->textCursor();
    cursor.beginEditBlock();

    const int top = qMin(cursor.position(), cursor.anchor());
    const int bottom = qMax(cursor.position(), cursor.anchor());

    // A reformatList should be called on the block inside selection
    // with the lowest indentation level
    int minIndentPosition;
    int minIndent = -1;

    // Changing indentation of all blocks between top and bottom
    cursor.setPosition(top);
    do {
        QTextList *list = cursor.currentList();
        // Setting up listFormat
        QTextListFormat listFmt;
        if (!list) {
            if (delta > 0) {
                // No list, we're increasing indentation -> create a new one
                listFmt.setStyle(QTextListFormat::ListDisc);
                listFmt.setIndent(delta);
            }
            // else do nothing
        } else {
            const int newIndent = list->format().indent() + delta;
            if (newIndent > 0) {
                listFmt = list->format();
                listFmt.setIndent(newIndent);
            } else {
                listFmt.setIndent(0);
            }
        }

        if (listFmt.indent() > 0) {
            // This block belongs to a list: here we create a new one
            // for each block, and then let reformatList() sort it out
            cursor.createList(listFmt);
            if (minIndent == -1 || minIndent > listFmt.indent()) {
                minIndent = listFmt.indent();
                minIndentPosition = cursor.block().position();
            }
        } else {
            // If the block belonged to a list, remove it from there
            if (list) {
                list->remove(cursor.block());
            }
            // The removal does not change the indentation, we need to do it explicitly
            QTextBlockFormat blkFmt;
            blkFmt.setIndent(0);
            cursor.mergeBlockFormat(blkFmt);
        }
        if (!cursor.block().next().isValid()) {
            break;
        }
        cursor.movePosition(QTextCursor::NextBlock);
    } while (cursor.position() < bottom);
    // Reformatting the whole list
    if (minIndent != -1) {
        cursor.setPosition(minIndentPosition);
        reformatList(cursor.block());
    }
    cursor.setPosition(top);
    reformatList(cursor.block());
    cursor.endEditBlock();
}

void NestedListHelper::handleOnBulletType(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();
    if (styleIndex != 0) {
        QTextListFormat::Style style = static_cast<QTextListFormat::Style>(styleIndex);
        QTextList *currentList = cursor.currentList();
        QTextListFormat listFmt;

        cursor.beginEditBlock();

        if (currentList) {
            listFmt = currentList->format();
            listFmt.setStyle(style);
            currentList->setFormat(listFmt);
        } else {
            listFmt.setStyle(style);
            cursor.createList(listFmt);
        }

        cursor.endEditBlock();
    } else {
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.setBlockFormat(bfmt);
    }

    reformatList();
}
