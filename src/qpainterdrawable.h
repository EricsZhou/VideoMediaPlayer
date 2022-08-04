#ifndef QPAINTERDRAWABLE_H
#define QPAINTERDRAWABLE_H

#include <QWidget>
#include <QMutex>
#include <QActionGroup>
#include <QTimer>
#include <QMenu>
#include "mediabase.h"
#include "imagescaler.h"

using namespace LQF;
namespace Ui {
class QPainterDrawable;
}
#include "framequeue.h"
class QPainterDrawable : public QWidget
{
    Q_OBJECT

public:
    explicit QPainterDrawable(QWidget *parent = 0);
    ~QPainterDrawable();
    int draw(const Frame *newVideoFrame, bool, bool);
    OnPlay(QString strFile);
    bool ConnectSignalSlots();
    bool Init();
signals:
    void SigOpenFile(QString strFileName);///< 增加视频文件
    void SigPlay(QString strFile); ///<播放


private:
    Ui::QPainterDrawable *ui;

    int x_ = 0; //  起始位置
    int y_ = 0;
    int video_width = 0;
    int video_height = 0;
    int img_width = 0;
    int img_height = 0;
    QImage img;
    VideoFrame dst_video_frame_;
    QMutex m_mutex;
    ImageScaler *img_scaler_ = NULL;

protected:
    bool event(QEvent *) override;
    void paintEvent(QPaintEvent *) override;
    /**
     * @brief	放下事件
     *
     * @param	event 事件指针
     * @note
     */
    void dropEvent(QDropEvent *event);
    /**
     * @brief	拖动事件
     *
     * @param	event 事件指针
     * @note
     */
    void dragEnterEvent(QDragEnterEvent *event);
    /**
     * @brief	窗口大小变化事件
     *
     * @param	event 事件指针
     * @note
     */
    void resizeEvent(QResizeEvent *event);

    /**
     * @brief	按键事件
     *
     * @param
     * @return
     * @note
     */
    void keyReleaseEvent(QKeyEvent *event);


    void mousePressEvent(QMouseEvent *event);
    //void contextMenuEvent(QContextMenuEvent* event);
public:
    void SlotStopFinished();

    /**
     * @brief	调整显示画面的宽高，使画面保持原比例
     *
     * @param	nFrameWidth 宽
     * @param	nFrameHeight 高
     * @return
     * @note
     */
    void OnFrameDimensionsChanged(int nFrameWidth, int nFrameHeight);
private:
    /**
     * @brief	显示信息
     *
     * @param	strMsg 信息内容
     * @note
     */
    void OnDisplayMsg(QString strMsg);


    void OnTimerShowCursorUpdate();

    void OnActionsTriggered(QAction *action);
private:


    void ChangeShow();
signals:
//    void SigOpenFile(QString strFileName);///< 增加视频文件
//    void SigPlay(QString strFile); ///<播放

    void SigFullScreen();//全屏播放
    void SigPlayOrPause();
    void SigStop();
    void SigShowMenu();

    void SigSeekForward();
    void SigSeekBack();
    void SigAddVolume();
    void SigSubVolume();
private:

    int m_nLastFrameWidth; ///< 记录视频宽高
    int m_nLastFrameHeight;
    bool is_display_size_change_ = false;

    QTimer timerShowCursor;

    QMenu m_stMenu;
};

#endif // QPAINTERDRAWABLE_H
