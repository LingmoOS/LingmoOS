#!/bin/bash

echo "已经暂存文件，恢复文件使用: git stash pop"
git stash pop

echo "准备切换分支到：dev/professional/bugfix ..."
git checkout dev/professional/bugfix

gitstatus=`git status`

current_branch_name=`echo $gitstatus | cut -d " " -f 2`

echo "当前分支名字:" $current_branch_name

echo "请输入新分支名字:"
read new_branch_name

echo "获取" $current_branch_name "分支最新..."
git pull 

echo "克隆新分支:" $new_branch_name
git checkout -b $new_branch_name

echo "关联远端分支..."
git branch --set-upstream-to=origin/$current_branch_name
git pull

echo "推送本地分支到远端分支..."
git push -u  origin $new_branch_name:$new_branch_name
git pull

git status
