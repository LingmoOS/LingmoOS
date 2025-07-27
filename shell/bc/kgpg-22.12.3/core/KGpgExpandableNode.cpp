/*
    SPDX-FileCopyrightText: 2008, 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "KGpgExpandableNode.h"

#include "kgpgsettings.h"
#include "core/convert.h"
#include "model/kgpgitemmodel.h"

KGpgExpandableNode::KGpgExpandableNode(KGpgExpandableNode *parent)
	: KGpgNode(parent)
{
	if (parent != nullptr)
		parent->children.append(this);
}

KGpgExpandableNode::~KGpgExpandableNode()
{
	for (int i = children.count() - 1; i >= 0; i--)
		delete children[i];
}

KGpgNode *
KGpgExpandableNode::getChild(const int index) const
{
	return children.at(index);
}

int
KGpgExpandableNode::getChildCount()
{
	if (children.isEmpty())
		readChildren();

	return children.count();
}

bool
KGpgExpandableNode::hasChildren() const
{
	return !children.isEmpty();
}

bool
KGpgExpandableNode::wasExpanded() const
{
	return !children.isEmpty();
}

const
KGpgNode::List &
KGpgExpandableNode::getChildren() const
{
	return children;
}

int
KGpgExpandableNode::getChildIndex(KGpgNode *node) const
{
	return children.indexOf(node);
}

void
KGpgExpandableNode::deleteChild(KGpgNode *child)
{
	children.removeAll(child);
}
