#include "qpainterdrawable.h"
#include "ui_qpainterdrawable.h"
#include <QPainter>
#include "mediaplayer.h"

QPainterDrawable::QPainterDrawable(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QPainterDrawable)
{
    ui->setupUi(this);
}

QPainterDrawable::~QPainterDrawable()
{
    delete ui;
    if(dst_video_frame_.data[0])
        free(dst_video_frame_.data[0]);
}

int QPainterDrawable::draw(const Frame *video_frame, bool, bool)
{
    QMutexLocker locker(&m_mutex);

    if(!img_scaler_) {
        int win_width = width();
        int win_height = height();
        video_width = video_frame->width;
        video_height = video_frame->height;
        img_scaler_ = new ImageScaler();
        double video_aspect_ratio = video_frame->width *1.0 / video_frame->height;
        double win_aspect_ratio = win_width*1.0 / win_height;
        if(win_aspect_ratio > video_aspect_ratio) {
            //此时应该是调整x的起始位置，以高度为基准
            img_height = win_height;
            if(img_height %2 != 0) {
                img_height -= 1;
            }

            img_width = img_height*video_aspect_ratio;
            if(img_width %2 != 0) {
                img_width -= 1;
            }
            y_ = 0;
            x_ = (win_width - img_width) / 2;
        } else {
            //此时应该是调整y的起始位置，以宽度为基准
            img_width = win_width;
            if(img_width %2 != 0) {
                img_width -= 1;
            }
            img_height = img_width / video_aspect_ratio;
            if(img_height %2 != 0) {
                img_height -= 1;
            }
            x_ = 0;
            y_ = (win_height - img_height) / 2;
        }
        img_scaler_->Init(video_width, video_height, video_frame->format,
                          img_width, img_height, AV_PIX_FMT_RGB24);
        memset(&dst_video_frame_, 0, sizeof(VideoFrame));
        dst_video_frame_.width = img_width;
        dst_video_frame_.height = img_height;
        dst_video_frame_.format = AV_PIX_FMT_RGB24;
        dst_video_frame_.data[0] = (uint8_t*)malloc(img_width * img_height * 3);
        dst_video_frame_.linesize[0] = img_width * 3; // 每行的字节数
    }
    img_scaler_->Scale3(video_frame, &dst_video_frame_);

    QImage imageTmp =  QImage((uint8_t *)dst_video_frame_.data[0],
            img_width, img_height, QImage::Format_RGB888);
    img = imageTmp.copy(0, 0, img_width, img_height);
    update();
    return 0;
}

QPainterDrawable::OnPlay(QString strFile)
{
    MediaPlayer::GetInstance()->StartPlay(strFile, NULL);
}
void QPainterDrawable::paintEvent(QPaintEvent *)
{
    QMutexLocker locker(&m_mutex);
    if (img.isNull()) {
        return;
    }
    QPainter painter(this);

    //    //    p.translate(X, Y);
    //    //    p.drawImage(QRect(0, 0, W, H), img);
    QRect rect = QRect(x_, y_, img.width(), img.height());
    painter.drawImage(rect, img);
}

bool QPainterDrawable::event(QEvent *e)
{
    /* Pass gesture and touch event to the parent */
    switch (e->type())
    {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::Gesture:
        return QCoreApplication::sendEvent(parent(), e);
    default:
        return QWidget::event(e);
    }
}



bool QPainterDrawable::Init()
{
    if (ConnectSignalSlots() == false)
    {
        return false;
    }

    //ui->label->setUpdatesEnabled(false);




    return true;
}

void QPainterDrawable::OnFrameDimensionsChanged(int nFrameWidth, int nFrameHeight)
{
    qDebug() << "QPainterDrawable::OnFrameDimensionsChanged" << nFrameWidth << nFrameHeight;
    m_nLastFrameWidth = nFrameWidth;
    m_nLastFrameHeight = nFrameHeight;

    ChangeShow();
}

void QPainterDrawable::ChangeShow()
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "QPainterDrawable::ChangeShow() "  << width() << ", " << height();
#if 0
    if (m_nLastFrameWidth == 0 && m_nLastFrameHeight == 0)
    {
        ui->label->setGeometry(0, 0, width(), height());
    }
    else
    {
        float aspect_ratio;
        int width, height, x, y;
        int scr_width = this->width();
        int scr_height = this->height();

        aspect_ratio = (float)m_nLastFrameWidth / (float)m_nLastFrameHeight;

        height = scr_height;
        width = lrint(height * aspect_ratio) & ~1;
        if (width > scr_width)
        {
            width = scr_width;
            height = lrint(width / aspect_ratio) & ~1;
        }
        x = (scr_width - width) / 2;
        y = (scr_height - height) / 2;


        ui->label->setGeometry(x, y, width, height);
    }
#endif
}

void QPainterDrawable::dragEnterEvent(QDragEnterEvent *event)
{

}

void QPainterDrawable::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    ChangeShow();
}

void QPainterDrawable::keyReleaseEvent(QKeyEvent *event)
{

}

// void QPainterDrawable::contextMenuEvent(QContextMenuEvent* event)
// {
//     //m_stMenu.exec(event->globalPos());
//     qDebug() << "QPainterDrawable::contextMenuEvent";
// }
void QPainterDrawable::mousePressEvent(QMouseEvent *event)
{

}

void QPainterDrawable::OnDisplayMsg(QString strMsg)
{
    qDebug() << "QPainterDrawable::OnDisplayMsg " << strMsg;
}


void QPainterDrawable::SlotStopFinished()
{
    update();
}


void QPainterDrawable::OnTimerShowCursorUpdate()
{
    //qDebug() << "QPainterDrawable::OnTimerShowCursorUpdate()";
    //setCursor(Qt::BlankCursor);
}

void QPainterDrawable::OnActionsTriggered(QAction *action)
{
    QString strAction = action->text();
    if (strAction == "全屏")
    {
        emit SigFullScreen();
    }
    else if (strAction == "停止")
    {
        emit SigStop();
    }
    else if (strAction == "暂停" || strAction == "播放")
    {
        emit SigPlayOrPause();
    }
}

bool QPainterDrawable::ConnectSignalSlots()
{
    QList<bool> listRet;
    bool bRet;

    bRet = connect(this, &QPainterDrawable::SigPlay, this, &QPainterDrawable::OnPlay);
    listRet.append(bRet);

    timerShowCursor.setInterval(2000);
    bRet = connect(&timerShowCursor, &QTimer::timeout, this, &QPainterDrawable::OnTimerShowCursorUpdate);
    listRet.append(bRet);

    for (bool bReturn : listRet)
    {
        if (bReturn == false)
        {
            return false;
        }
    }

    return true;
}

void QPainterDrawable::dropEvent(QDropEvent *event)
{

}
