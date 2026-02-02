#ifndef MGLWIDGET_H
#define MGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QColor>
#include <QMatrix4x4>

class QOpenGLShaderProgram;
class QOpenGLTexture;

#include "player.h"

class mGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit mGLWidget(QWidget *parent = nullptr);
    ~mGLWidget() override;

    void setPlayer(player* p) { m_player = p; }
    void setClearColor(const QColor &color);
    void setTextureImage(const QString &imagePath);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

signals:
    void clicked(); 

public slots:
    void onFrameReady(int w, int h, int type, int bpp);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mouseReleaseEvent(QMouseEvent *event) override; 

private:
    void make2DObject();

    player* m_player = nullptr;
    QColor clearColor;
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vbo;
    QOpenGLTexture *texture;
};

#endif // MGLWIDGET_H
