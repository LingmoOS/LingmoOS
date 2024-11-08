# LingmoUIFileMetadata
## 简介
LingmoUIFileMetaData提供了文件元数据解析与用户自定义数据写入功能，其中读取部分结构与部分代码参考自KFileMetaData。
## 功能说明
### 文件元数据解析
用于提取文件元数据信息，包含： 
```c++
enum Flag {
ExtractNothing = 0,
ExtractMetaData = 1,   //基本元数据
ExtractPlainText = 2,  //文本
ExtractImageData = 4,  //图片数据
ExtractThumbnail = 8   //缩略图
};
```
使用时需要继承ExtractionResult实现一个用于传递参数和存储解析结果的子类。
使用举例：
```c++
//解析文件文本内容
QString mimeType = LingmoUIFileMetadata::MimeUtils::strictMimeType(path, {}).name();
QList<LingmoUIFileMetadata::Extractor*> extractors = m_extractorManager.fetchExtractors(mimeType);
ExtractionResult result(path, mimeType, LingmoUIFileMetadata::ExtractionResult::Flag::ExtractPlainText);
for(auto extractor : extractors) {
    extractor->extract(&result);
    if(!result.text().isEmpty()) {
        qDebug() << result.text();
        break;
    }
}
    //获取缩略图
QString mimeType = LingmoUIFileMetadata::MimeUtils::strictMimeType(path, {}).name();
QList<LingmoUIFileMetadata::Extractor*> extractors = m_extractorManager.fetchExtractors(mimeType);
ExtractionResult result(path, mimeType, LingmoUIFileMetadata::ExtractionResult::Flag::ExtractThumbnail);
result.setThumbnailRequest(LingmoUIFileMetadata::ThumbnailRequest(QSize(512, 512), 1));
for(auto extractor : extractors) {
    extractor->extract(&result);
    if(result.thumbnail().isValid()) {
        qDebug() << result.thumbnail().image();
    break;
}
    //使用ocr
QString textContent = LingmoUIFileMetadata::OcrUtils::getTextInPicture(path);
qDebug() << textContent;
}
```
### 文件元数据写入
TODO
### 元数据解析插件
继承ExtractorPlugin类实现针对某种类型的文件元数据解析功能可实现功能扩展

##使用说明
```cmake
find_package(lingmo-file-metadata)
target_link_libraries(your-target PUBLIC lingmo-file-metadata)
```
