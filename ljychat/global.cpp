#include "global.h"

QString gate_url_prefix="";



std::function<void(QWidget*)> repolish = [](QWidget* w){
    //剥离旧的样式
    w->style()->unpolish(w);
    //重新应用新样式
    w->style()->polish(w);
};


std::function<QString(QString)> xorString = [](QString input){
    //复制原始字符串，以便进行修改
    QString result = input;
    //获取字符串的长度
    int length=input.length();
    length = length % 255;
    for(int i=0; i<length;++i){
        //对每个字符进行异或操作
        //注意：这里假设字符都是ASCII，所以直接转换为QChar
        result[i]=QChar(static_cast<ushort>(input[i].unicode() ^ static_cast<ushort>(length)));
    }
    return result;
};
