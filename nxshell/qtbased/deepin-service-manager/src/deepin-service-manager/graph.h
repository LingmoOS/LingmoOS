// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GRAPH_H
#define GRAPH_H

#include <QDebug>
#include <QList>

// 邻接表
template<class T>
class Graph
{
private: // 内部类
    // 邻接表中表对应的链表的顶点
    class ENode
    {
        int ivex;        // 该边所指向的顶点的位置
        ENode *nextEdge; // 指向下一条弧的指针
        friend class Graph;
    };

    // 邻接表中表的顶点
    class VNode
    {
        T data;           // 顶点信息
        ENode *firstEdge; // 指向第一条依附该顶点的弧
        friend class Graph;
    };

private:                 // 私有成员
    int m_vexNum;        // 图的顶点的数目
    int m_edgNum;        // 图的边的数目
    QList<VNode> m_vexs; // 图的顶点数组

public:
    // 创建邻接表对应的图(自己输入)
    Graph();
    // 创建邻接表对应的图(用已提供的数据)
    Graph(const QList<T> &vexs, const QList<QPair<T, T>> &edges);
    ~Graph();

    // 拓扑排序
    int topologicalSort(QList<T> &out);

private:
    // 返回ch的位置
    int getPosition(T ch);
    // 将node节点链接到list的最后
    void linkLast(ENode *list, ENode *node);
};

template<class T>
Graph<T>::Graph()
{
}

template<class T>
Graph<T>::Graph(const QList<T> &vexs, const QList<QPair<T, T>> &edges)
{
    T c1, c2;
    int i, p1, p2;
    ENode *node1;

    // 初始化"顶点数"和"边数"
    m_vexNum = vexs.count();
    m_edgNum = edges.count();
    // 初始化"邻接表"的顶点
    for (auto &&vex : vexs) {
        VNode node;
        node.data = vex;
        node.firstEdge = nullptr;
        m_vexs.append(node);
    }

    // 初始化"邻接表"的边
    for (i = 0; i < m_edgNum; i++) {
        // 读取边的起始顶点和结束顶点
        c1 = edges[i].first;
        c2 = edges[i].second;

        p1 = getPosition(c1);
        p2 = getPosition(c2);
        // 初始化node1
        node1 = new ENode();
        node1->ivex = p2;
        // 将node1链接到"p1所在链表的末尾"
        if (m_vexs[p1].firstEdge == nullptr)
            m_vexs[p1].firstEdge = node1;
        else
            linkLast(m_vexs[p1].firstEdge, node1);
    }
}

template<class T>
Graph<T>::~Graph()
{
    ENode *node;

    for (int i = 0; i < m_edgNum; i++) {
        node = m_vexs[i].firstEdge;
        if (node != nullptr) {
            delete node;
            node = nullptr;
        }
    }
}

/*
 * 将node节点链接到list的最后
 */
template<class T>
void Graph<T>::linkLast(ENode *list, ENode *node)
{
    ENode *p = list;

    while (p->nextEdge)
        p = p->nextEdge;
    p->nextEdge = node;
}

/*
 * 返回指定数据的位置
 */
template<class T>
int Graph<T>::getPosition(T ch)
{
    int i;
    for (i = 0; i < m_vexNum; i++)
        if (m_vexs[i].data == ch)
            return i;
    return -1;
}

/*
 * 拓扑排序
 *
 * 返回值：
 *     -1 -- 失败(由于内存不足等原因导致)
 *      0 -- 成功排序，并输入结果
 *      1 -- 失败(该有向图是有环的)
 */
template<class T>
int Graph<T>::topologicalSort(QList<T> &out)
{
    int i, j;
    int index = 0;
    int head = 0;        // 辅助队列的头
    int rear = 0;        // 辅助队列的尾
    int queue[m_vexNum]; // 辅组队列
    int ins[m_vexNum];   // 入度数组
    T tops[m_vexNum];    // 拓扑排序结果数组，记录每个节点的排序后的序号。
    ENode *node;

    memset(ins, 0, m_vexNum * sizeof(int));
    memset(queue, 0, m_vexNum * sizeof(int));
    memset(tops, 0, m_vexNum * sizeof(T));

    // 统计每个顶点的入度数
    for (i = 0; i < m_vexNum; i++) {
        node = m_vexs[i].firstEdge;
        while (node != NULL) {
            ins[node->ivex]++;
            node = node->nextEdge;
        }
    }

    // 将所有入度为0的顶点入队列
    for (i = 0; i < m_vexNum; i++)
        if (ins[i] == 0)
            queue[rear++] = i; // 入队列

    while (head != rear) // 队列非空
    {
        j = queue[head++];              // 出队列。j是顶点的序号
        tops[index++] = m_vexs[j].data; // 将该顶点添加到tops中，tops是排序结果
        node = m_vexs[j].firstEdge;     // 获取以该顶点为起点的出边队列

        // 将与"node"关联的节点的入度减1；
        // 若减1之后，该节点的入度为0；则将该节点添加到队列中。
        while (node != NULL) {
            // 将节点(序号为node->ivex)的入度减1。
            ins[node->ivex]--;
            // 若节点的入度为0，则将其"入队列"
            if (ins[node->ivex] == 0)
                queue[rear++] = node->ivex; // 入队列

            node = node->nextEdge;
        }
    }

    if (index != m_vexNum) {
        qWarning() << "Graph has a cycle";
        return 1;
    }

    // 打印拓扑排序结果
    for (i = 0; i < m_vexNum; i++)
        out.append(tops[i]);

    return 0;
}

/* int main() */
/* { */
/*     QList<char> vexs = {'A', 'B', 'C', 'D', 'E', 'F', 'G'}; */
/*     QList<QPair<char, char>> edges = {{'A', 'G'}, */
/*                                       {'B', 'A'}, */
/*                                       {'B', 'D'}, */
/*                                       {'C', 'F'}, */
/*                                       {'C', 'G'}, */
/*                                       {'D', 'E'}, */
/*                                       {'D', 'F'}}; */
/*     Graph<char> *pG; */
/*  */
/*     // 自定义"图"(输入矩阵队列) */
/*     // pG = new ListDG(); */
/*     // 采用已有的"图" */
/*     pG = new Graph<char>(vexs, edges); */
/*  */
/*     pG->print();  // 打印图 */
/*     // pG->DFS();     // 深度优先遍历 */
/*     // pG->BFS();     // 广度优先遍历 */
/*     QList<char> result; */
/*     pG->topologicalSort(result);  // 拓扑排序 */
/*  */
/*     return 0; */
/* } */

#endif // GRAPH_H
