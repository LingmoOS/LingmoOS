find_package(Gettext REQUIRED)

# 设置翻译文件目录
set(LINGUAS zh_CN)
set(POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/lingmo-pkgbuild.pot)

# 编译每个语言的 .po 文件
foreach(LANG ${LINGUAS})
    # 设置输出目录
    set(PO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${LANG}.po")
    set(MO_FILE "${CMAKE_CURRENT_BINARY_DIR}/${LANG}/LC_MESSAGES/lingmo-pkgbuild.mo")
    
    # 创建输出目录
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LANG}/LC_MESSAGES")
    
    # 编译 .po 文件为 .mo 文件
    add_custom_command(
        OUTPUT ${MO_FILE}
        COMMAND msgfmt -o ${MO_FILE} ${PO_FILE}
        DEPENDS ${PO_FILE}
        COMMENT "Creating ${LANG} translation"
    )
    
    # 添加到构建目标
    add_custom_target(pofiles_${LANG} ALL DEPENDS ${MO_FILE})
    
    # 安装翻译文件
    install(
        FILES ${MO_FILE}
        DESTINATION share/locale/${LANG}/LC_MESSAGES
    )
endforeach() 