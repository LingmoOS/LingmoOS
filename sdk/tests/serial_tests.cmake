# Timings: tests running on the sibling cores slow these down threefold.
set_tests_properties(FrameCostTest.PageOfCardsRepaintsWithinAFrame
                     LayoutCostTest.SettingsResize
                     LayoutCostTest.SettingsValueChange
                     LayoutCostTest.SettingsScrollAndHover
                     LayoutCostTest.LongList
                     LayoutCostTest.CellLayoutSolve
                     LayoutCostTest.BoxLayoutMeasure
                     TextCostTest.SettingsPage
                     TextCostTest.LongTextArea
                     TextCostTest.TextFieldTyping
                     TextCostTest.CodeLines
                     PROPERTIES RUN_SERIAL TRUE LABELS timing)

# Wall-clock bounds that a loaded machine misses.
set_tests_properties(ImageViewTest.ShowsPicturesThatFinishTogetherTogether PROPERTIES LABELS timing)
