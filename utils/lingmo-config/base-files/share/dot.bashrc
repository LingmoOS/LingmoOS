# ~/.bashrc: executed by bash(1) for non-login shells.
# Lingmo OS default configuration

# 彩色提示符: user@host[工作目录]$
# 普通用户绿色，root 红色
if [ "$(id -u)" -eq 0 ]; then
    # root: 红色
    PS1='\[\033[01;31m\]\u@\h\[\033[00m\]\[\033[01;34m\][\W]\[\033[00m\]\$ '
else
    # 普通用户: 青蓝色
    PS1='\[\033[01;36m\]\u@\h\[\033[00m\]\[\033[01;34m\][\W]\[\033[00m\]\$ '
fi

export PS1

# ls 彩色输出
if [ -x /usr/bin/dircolors ]; then
    test -r ~/.dircolors && eval "$(dircolors -b ~/.dircolors)" || eval "$(dircolors -b)"
    alias ls='ls --color=auto'
    alias ll='ls --color=auto -l'
    alias la='ls --color=auto -la'
fi

# 安全别名（防误操作）
alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'
