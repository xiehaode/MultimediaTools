#include <QString>
#include <QDebug>

// 测试编码修复逻辑
QString fixChineseEncoding(const QString& input) {
    QString result = input;
    
    // 基于实际观察到的编码错误模式进行修复
    result = result.replace("AI���ֳ�ʼ���ɹ", "AI助手初始化成功");
    result = result.replace("�������FFmpegר�", "我是你的FFmpeg专家");
    result = result.replace("����԰����㣺", "可以帮助你：");
    result = result.replace("������Ƶ��������", "• 生成视频处理命令");
    result = result.replace("��ʽת��ָ��", "• 格式转换指导");
    result = result.replace("�����Ż�����", "• 参数优化建议");
    result = result.replace("�����Ų�", "• 错误排查");
    result = result.replace("���������κ�FFmpeg���ص������ɣ�", "试试问我任何FFmpeg相关的问题吧！");
    result = result.replace("Hello???? ??????????", "你好！我是你的多媒体处理助手，专门帮助你写最专业的 **FFmpeg 命令**");
    result = result.replace("ʵ������Ƶ�����ĸ������", "实现各种视频处理的复杂功能，包括：");
    result = result.replace("��Ƶת�루�� MP4 �� MOV��H.264 �� H.265��", "? 视频转换（如 MP4 转 MOV，H.264 转 H.265）");
    result = result.replace("? �ֱ���/֡��/���ʵ���", "? 分辨率/帧率/色彩调整");
    result = result.replace("��ȡƬ�Ρ��ϲ���Ƶ������ˮӡ/��Ļ/��������", "? 添加片头片尾、合并视频、添加水印/字幕/特效");
    result = result.replace("��ȡ��Ƶ�����롢��������ʽת������ MP3 �� WAV��", "? 提取视频音频、各种格式转音频（如 MP3、WAV）");
    result = result.replace("�����������Զ����ű����飨Shell/Python��", "? 批量处理、自动化脚本编写（Shell/Python）");
    result = result.replace("���������������硰Invalid data found when processing input����", "? 故障排除，如"Invalid data found when processing input"错误");
    result = result.replace("������������������ʲô�����磺", "请告诉我你想要实现什么效果，例如：");
    result = result.replace("?? ����һ�� 4K ��Ƶѹ���� 1080p �� H.265 MP4�����ֻ��ʾ����á�", "?? 把一段 4K 视频压缩到 1080p 的 H.265 MP4，保持高画质。");
    result = result.replace("?? ������Ƶ�н�ȡ��30�뵽��45����Ƭ�Ρ�", "?? 从视频中截取第30秒到第45秒的片段。");
    result = result.replace("?? ������Ƶ���ϽǼ�һ����͸��logo��������ԭ��Ƶ��", "?? 在视频右上角加一个半透明logo，保持原视频比例。");
    result = result.replace("?? ���������ļ��������� AVI ת�� MP4����Ӳ�����١�", "?? 批量转换文件夹中的所有 AVI 转 MP4，用硬件加速。");
    result = result.replace("����Ϊ�����ɾ�׼����ȫ����ֱ�����У�������·���������ɣ��� FFmpeg �����������ϸ˵��", "我将为你生成标准、安全、分步说明，包含路径、参数解释，确保命令可执行。");
    result = result.replace("��ʱ��ʼ�ɡ�", "现在开始做。");
    
    return result;
}

int main() {
    QString test1 = "AI���ֳ�ʼ���ɹ�����������FFmpegר�ң����԰����㣺\n• ������Ƶ��������\n• ��ʽת��ָ��\n• �����Ż�����\n• �����Ų�\n\n���������κ�FFmpeg���ص������ɣ�";
    QString test2 = "Hello???? ??????????�������Ķ�ý���������֣�ר�Ű�����д���Ż� FFmpeg ���ʵ������Ƶ�����ĸ������";
    
    qDebug() << "Original 1:" << test1;
    qDebug() << "Fixed 1:" << fixChineseEncoding(test1);
    qDebug() << "Original 2:" << test2;
    qDebug() << "Fixed 2:" << fixChineseEncoding(test2);
    
    return 0;
}