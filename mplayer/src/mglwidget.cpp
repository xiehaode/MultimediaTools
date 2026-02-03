#include "mglwidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <QImage>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include "mdevice.h"

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

void mGLWidget::onFrameReady(int w, int h, int type, int bpp)
{
    if (!m_player) return;
    SharedFrameBuffer* shared_buf = m_player->getSharedBuffer();
    
    std::lock_guard<std::mutex> lock(shared_buf->mtx);
    if (shared_buf->state != BUFFER_FILLED) return;

    makeCurrent();
    if (texture && (texture->width() != w || texture->height() != h)) {
        texture->destroy();
        delete texture;
        texture = nullptr;
    }

    if (!texture) {
        texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        texture->create();
        texture->setMinificationFilter(QOpenGLTexture::Linear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    }


    // 零拷贝更新纹理
    QImage glImage(shared_buf->buf.data(), w, h, QImage::Format_RGBA8888);
    texture->setData(glImage);
    
    shared_buf->state = BUFFER_IDLE;
    doneCurrent();
    update();
}

mGLWidget::mGLWidget(QWidget *parent)

    : QOpenGLWidget(parent)
    , clearColor(Qt::white)
    , program(nullptr)
    , texture(nullptr)
{


}

mGLWidget::~mGLWidget()
{
    makeCurrent();
    vbo.destroy();
    delete texture;
    delete program;
    doneCurrent();
}

QSize mGLWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize mGLWidget::sizeHint() const
{
    return QSize(1920, 1080);
}

void mGLWidget::setClearColor(const QColor &color)
{
    clearColor = color;
    update();
}

void mGLWidget::setTextureImage(const QString &imagePath)
{
    makeCurrent();

    if (texture) {
        delete texture;
        texture = nullptr;
    }
    qDebug()<<"current dir :"<<QDir::currentPath();

    QImage image(imagePath);
    if (image.isNull()) {
        qWarning() << "[纹理错误] 图片加载失败，路径：" << imagePath;
        qWarning() << "提示；本地图片放在程序运行目录,资源文件需加入qrc";
        doneCurrent();
        update();
        return;
    }

    texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    // 关键：Y轴翻转（适配OpenGL纹理原点）+ 转换为RGBA8888（OpenGL兼容格式）
    //QImage glImage = image.mirror(false, true).convertToFormat(QImage::Format_RGBA8888);
    QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
    texture->setData(glImage); // 向GPU传入像素数据（缺一不可）

    // 纹理参数配置
    texture->setMinificationFilter(QOpenGLTexture::Linear);    // 缩小时平滑
    texture->setMagnificationFilter(QOpenGLTexture::Linear);  // 放大时平滑
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);        // 边缘夹紧，避免重复
    //texture->setFormat(QOpenGLTexture::RGBA8_UNorm);          // 与QImage格式匹配
    qInfo() << "[纹理成功] 图片加载完成，尺寸：" << glImage.size();

    doneCurrent(); // 释放上下文
    update();      // 触发重绘，显示新纹理
}

void mGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    make2DObject(); // 创建顶点数据（适配任意窗口尺寸）

    // 顶点着色器（无修改，保持2D正交投影）
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
        "attribute highp vec4 vertex;\n"
        "attribute mediump vec4 texCoord;\n"
        "varying mediump vec4 texc;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    texc = texCoord;\n"
        "}\n";
    if (!vshader->compileSourceCode(vsrc)) {
        qFatal("[着色器错误] 顶点着色器编译失败：%s", vshader->log().toUtf8().data());
    }

    // 片段着色器（无修改，纹理采样逻辑通用）
    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
        "uniform sampler2D texture;\n"
        "varying mediump vec4 texc;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, texc.st);\n"
        "}\n";
    if (!fshader->compileSourceCode(fsrc)) {
        qFatal("[着色器错误] 片段着色器编译失败：%s", fshader->log().toUtf8().data());
    }

    // 链接着色器程序，增加链接失败校验
    program = new QOpenGLShaderProgram;
    program->addShader(vshader);
    program->addShader(fshader);
    program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
    if (!program->link()) {
        qFatal("[着色器错误] 程序链接失败：%s", program->log().toUtf8().data());
    }

    program->bind();
    program->setUniformValue("texture", 0); // 绑定纹理单元0
}

void mGLWidget::paintGL()
{
    // 清除颜色缓冲区，恢复白色背景（方便观察纹理，避免黑屏混淆）
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!texture || !program) {
        qWarning() << "绘制跳过：纹理或着色器未初始化";
        return;
    }

    // 标准正交投影：匹配[-1,1]归一化顶点坐标，确保绘制在视口中央
    QMatrix4x4 m;
    m.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    vbo.bind();
    program->bind();
    program->setUniformValue("matrix", m);

    // 启用并设置顶点/纹理属性，确保步长和偏移完全匹配VBO数据
    program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    // 顶点属性：起始0，3个分量（x,y,z），步长5*GLfloat（3顶点+2纹理）
    program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    // 纹理属性：起始3*GLfloat，2个分量（s,t），步长与顶点一致
    program->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    // 绑定纹理单元0，确保与着色器中uniform sampler2D texture的绑定一致
    texture->bind(0);
    // 绘制4个顶点（GL_TRIANGLE_FAN：左上→右上→右下→左下，完美构成矩形）
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // 解绑资源，避免上下文污染
    texture->release();
    program->release();
    vbo.release();
}

void mGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height); // 视口匹配窗口尺寸
    // 窗口缩放时重新创建顶点数据，适配新尺寸
    make2DObject();
    update();
}

void mGLWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    emit clicked();
}

void mGLWidget::make2DObject()
{
    // 顶点数据使用归一化坐标，适配任意窗口尺寸
    // 不再依赖初始化时的width()/height()，而是通过正交投影矩阵映射到实际窗口
    // 顶点坐标：[-1,1]归一化范围，纹理坐标：[0,1]标准范围
    // 顶点顺序：左上、右上、右下、左下（GL_TRIANGLE_FAN绘制规则）
    static const GLfloat vertData[] = {
        // 顶点坐标(x,y,z)  纹理坐标(s,t)
        -1.0f,  1.0f, 0.0f,   0.0f, 0.0f, // 左上
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, // 右上
         1.0f, -1.0f, 0.0f,   1.0f, 1.0f, // 右下
        -1.0f, -1.0f, 0.0f,   0.0f, 1.0f  // 左下
    };

    // 重新创建VBO并分配数据（窗口缩放时会重新调用，覆盖旧数据）
    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(vertData, sizeof(vertData));
}
