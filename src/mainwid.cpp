﻿//主窗口
#include <QFile>
#include <QPainter>
#include <QtMath>
#include <QDebug>
#include <QAbstractItemView>
#include <QMimeData>
#include <QSizeGrip>
#include <QWindow>
#include <QDesktopWidget>
#include <QScreen>
#include <QRect>
#include <QFileDialog>

#include "mainwid.h"
#include "ui_mainwid.h"
#include "globalhelper.h"
#include "MediaPlayer.h"

const int FULLSCREEN_MOUSE_DETECT_TIME = 500;

MainWid::MainWid(QMainWindow *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWid),
    m_nShadowWidth(0),
    m_stMenu(this),
    m_stPlaylist(this),
    m_stTitle(this),
    m_bMoveDrag(false),
    m_stActExit(this),
    m_stActAbout(this),
    m_stActOpen(this),
    m_stActFullscreen(this)
{
    ui->setupUi(this);
    //无边框、无系统菜单、 任务栏点击最小化
    setWindowFlags(Qt::FramelessWindowHint /*| Qt::WindowSystemMenuHint*/ | Qt::WindowMinimizeButtonHint);
    //设置任务栏图标
    this->setWindowIcon(QIcon("://res/player.png"));
    //加载样式
    QString qss = GlobalHelper::GetQssStr("://res/qss/mainwid.css");
    setStyleSheet(qss);

    // 追踪鼠标 用于播放时隐藏鼠标
    this->setMouseTracking(true);

    //ui->ShowWid->setMouseTracking(true);

    //保证窗口不被绘制上的部分透明
    //setAttribute(Qt::WA_TranslucentBackground);

    //接受放下事件
    //setAcceptDrops(true);
    //可以清晰地看到放下过程中的图标指示
    //setDropIndicatorShown(true);

    //    setAcceptDrops(true);
    //    setDragDropMode(QAbstractItemView::DragDrop);
    //    setDragEnabled(true);
    //    setDropIndicatorShown(true);

    //窗口大小调节
    //    QSizeGrip   *pSizeGrip = new QSizeGrip(this);
    //    pSizeGrip->setMinimumSize(10, 10);
    //    pSizeGrip->setMaximumSize(10, 10);
    //    ui->verticalLayout->addWidget(pSizeGrip, 0, Qt::AlignBottom | Qt::AlignRight);

    m_bPlaying = false;

    m_bFullScreenPlay = false;

    m_stCtrlBarAnimationTimer.setInterval(2000);
    m_stFullscreenMouseDetectTimer.setInterval(FULLSCREEN_MOUSE_DETECT_TIME);
    MediaPlayer::GetInstance()->AddVideoRefreshCallback(std::bind(&MainWid::OutputVideo, this,
                                                               std::placeholders::_1));
    
}

MainWid::~MainWid()
{
    delete ui;
}

bool MainWid::Init()
{
    QWidget *em = new QWidget(this);
    ui->PlaylistWid->setTitleBarWidget(em);
    ui->PlaylistWid->setWidget(&m_stPlaylist);
    ui->PlaylistWid->setFixedWidth(150);

    QWidget *emTitle = new QWidget(this);
    ui->TitleWid->setTitleBarWidget(emTitle);
    ui->TitleWid->setWidget(&m_stTitle);
    

    //     FramelessHelper *pHelper = new FramelessHelper(this); //无边框管理
    //     pHelper->activateOn(this);  //激活当前窗体
    //     pHelper->setTitleHeight(ui->TitleWid->height());  //设置窗体的标题栏高度
    //     pHelper->setWidgetMovable(true);  //设置窗体可移动
    //     pHelper->setWidgetResizable(true);  //设置窗体可缩放
    //     pHelper->setRubberBandOnMove(true);  //设置橡皮筋效果-可移动
    //     pHelper->setRubberBandOnResize(true);  //设置橡皮筋效果-可缩放

    //连接自定义信号与槽
    if (ConnectSignalSlots() == false)
    {
        return false;
    }

    if (ui->CtrlBarWid->Init() == false ||
            ui->ShowWid->Init() == false ||
            m_stPlaylist.Init() == false ||
            m_stTitle.Init() == false)
    {
        return false;
    }


    m_stCtrlbarAnimationShow = new QPropertyAnimation(ui->CtrlBarWid, "geometry");
    m_stCtrlbarAnimationHide = new QPropertyAnimation(ui->CtrlBarWid, "geometry");

    if (m_stAboutWidget.Init() == false)
    {
        return false;
    }

    m_stActFullscreen.setText("全屏");
    m_stActFullscreen.setCheckable(true);
    m_stMenu.addAction(&m_stActFullscreen);


    m_stActOpen.setText("打开");
    m_stMenu.addAction(&m_stActOpen);

    m_stActAbout.setText("关于");
    m_stMenu.addAction(&m_stActAbout);
    
    m_stActExit.setText("退出");
    m_stMenu.addAction(&m_stActExit);



    return true;
}

void MainWid::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
}


void MainWid::enterEvent(QEvent *event)
{
    Q_UNUSED(event);

}

void MainWid::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

}

bool MainWid::ConnectSignalSlots()
{
    //连接信号与槽
    connect(&m_stTitle, &Title::SigCloseBtnClicked, this, &MainWid::OnCloseBtnClicked);
    connect(&m_stTitle, &Title::SigMaxBtnClicked, this, &MainWid::OnMaxBtnClicked);
    connect(&m_stTitle, &Title::SigMinBtnClicked, this, &MainWid::OnMinBtnClicked);
    connect(&m_stTitle, &Title::SigDoubleClicked, this, &MainWid::OnMaxBtnClicked);
//    connect(&m_stTitle, &Title::SigFullScreenBtnClicked, this, &MainWid::OnFullScreenPlay);
    connect(&m_stTitle, &Title::SigOpenFile, &m_stPlaylist, &Playlist::OnAddFileAndPlay);
    connect(&m_stTitle, &Title::SigShowMenu, this, &MainWid::SlotShowMenu);
    

    connect(&m_stPlaylist, &Playlist::SigPlay, ui->ShowWid, &QPainterDrawable::SigPlay);

//    connect(ui->ShowWid, &Show::SigOpenFile, &m_stPlaylist, &Playlist::OnAddFileAndPlay);
//    connect(ui->ShowWid, &Show::SigFullScreen, this, &MainWid::OnFullScreenPlay);
//    connect(ui->ShowWid, &Show::SigPlayOrPause, MediaPlayer::GetInstance(), &MediaPlayer::OnPause);
//    connect(ui->ShowWid, &Show::SigStop, MediaPlayer::GetInstance(), &MediaPlayer::OnStop);
//    connect(ui->ShowWid, &Show::SigShowMenu, this, &MainWid::SlotShowMenu);
//    connect(ui->ShowWid, &Show::SigSeekForward, MediaPlayer::GetInstance(), &MediaPlayer::OnSeekForward);
//    connect(ui->ShowWid, &Show::SigSeekBack, MediaPlayer::GetInstance(), &MediaPlayer::OnSeekBack);
//    connect(ui->ShowWid, &Show::SigAddVolume, MediaPlayer::GetInstance(), &MediaPlayer::OnAddVolume);
//    connect(ui->ShowWid, &Show::SigSubVolume, MediaPlayer::GetInstance(), &MediaPlayer::OnSubVolume);

    connect(ui->CtrlBarWid, &CtrlBar::SigSpeed, MediaPlayer::GetInstance(), &MediaPlayer::SlotSpeed);
    connect(ui->CtrlBarWid, &CtrlBar::SigShowOrHidePlaylist, this, &MainWid::SlotShowOrHidePlaylist);
    connect(ui->CtrlBarWid, &CtrlBar::SigPlaySeek, MediaPlayer::GetInstance(), &MediaPlayer::OnPlaySeek);
    connect(ui->CtrlBarWid, &CtrlBar::SigPlayVolume, MediaPlayer::GetInstance(), &MediaPlayer::OnPlayVolume);
    connect(ui->CtrlBarWid, &CtrlBar::SigPlayOrPause, MediaPlayer::GetInstance(), &MediaPlayer::OnPause);
    connect(ui->CtrlBarWid, &CtrlBar::SigStop, MediaPlayer::GetInstance(), &MediaPlayer::OnStop);
    connect(ui->CtrlBarWid, &CtrlBar::SigBackwardPlay, &m_stPlaylist, &Playlist::OnBackwardPlay);
    connect(ui->CtrlBarWid, &CtrlBar::SigForwardPlay, &m_stPlaylist, &Playlist::OnForwardPlay);
    connect(ui->CtrlBarWid, &CtrlBar::SigShowMenu, this, &MainWid::SlotShowMenu);
    connect(ui->CtrlBarWid, &CtrlBar::SigShowSetting, this, &MainWid::SlotShowSetting);

//    connect(this, &MainWid::SigShowMax, &m_stTitle, &Title::OnChangeMaxBtnStyle);
//    connect(this, &MainWid::SigSeekForward, MediaPlayer::GetInstance(), &MediaPlayer::OnSeekForward);
//    connect(this, &MainWid::SigSeekBack, MediaPlayer::GetInstance(), &MediaPlayer::OnSeekBack);
//    connect(this, &MainWid::SigAddVolume, MediaPlayer::GetInstance(), &MediaPlayer::OnAddVolume);
//    connect(this, &MainWid::SigSubVolume, MediaPlayer::GetInstance(), &MediaPlayer::OnSubVolume);
//    connect(this, &MainWid::SigOpenFile, &m_stPlaylist, &Playlist::OnAddFileAndPlay);
    
    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigSpeed, ui->CtrlBarWid, &CtrlBar::SlotSpeed);
    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigVideoTotalSeconds, ui->CtrlBarWid, &CtrlBar::SlotVideoTotalSeconds);
    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigVideoPlaySeconds, ui->CtrlBarWid, &CtrlBar::SlotVideoPlaySeconds);
    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigVideoVolume, ui->CtrlBarWid, &CtrlBar::SlotVideoVolume);
    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigPauseStat, ui->CtrlBarWid, &CtrlBar::SlotPauseStatus, Qt::QueuedConnection);
    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigStopFinished, ui->CtrlBarWid, &CtrlBar::SlotStopFinished, Qt::QueuedConnection);
//    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigStopFinished, ui->ShowWid, &Show::SlotStopFinished, Qt::QueuedConnection);
//    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigFrameDimensionsChanged, ui->ShowWid, &Show::OnFrameDimensionsChanged, Qt::QueuedConnection);
    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigStopFinished, &m_stTitle, &Title::SlotStopFinished, Qt::DirectConnection);
    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigStartPlay, &m_stTitle, &Title::OnPlay, Qt::DirectConnection);
    // 触发的是播放逻辑
    connect(MediaPlayer::GetInstance(), &MediaPlayer::SigRequestPlay, &m_stPlaylist, &Playlist::OnRequestPlayCurrentFile);

    connect(&m_stCtrlBarAnimationTimer, &QTimer::timeout, this, &MainWid::OnCtrlBarAnimationTimeOut);

    connect(&m_stFullscreenMouseDetectTimer, &QTimer::timeout, this, &MainWid::OnFullscreenMouseDetectTimeOut);


    connect(&m_stActAbout, &QAction::triggered, this, &MainWid::SlotShowAbout);
    connect(&m_stActFullscreen, &QAction::triggered, this, &MainWid::OnFullScreenPlay);
    connect(&m_stActExit, &QAction::triggered, this, &MainWid::OnCloseBtnClicked);
    connect(&m_stActOpen, &QAction::triggered, this, &MainWid::OpenFile);
    


    return true;
}


void MainWid::keyReleaseEvent(QKeyEvent *event)
{
    // 	    // 是否按下Ctrl键      特殊按键
    //     if(event->modifiers() == Qt::ControlModifier){
    //         // 是否按下M键    普通按键  类似
    //         if(event->key() == Qt::Key_M)
    //             ···
    //     }
    qDebug() << "MainWid::keyPressEvent:" << event->key();
    switch (event->key())
    {
//    case Qt::Key_Return://全屏
//        OnFullScreenPlay();
//        break;
//    case Qt::Key_Left://后退5s
//        emit SigSeekBack();
//        break;
//    case Qt::Key_Right://前进5s
//        qDebug() << "前进5s";
//        emit SigSeekForward();
//        break;
//    case Qt::Key_Up://增加10音量
//        emit SigAddVolume();
//        break;
//    case Qt::Key_Down://减少10音量
//        emit SigSubVolume();
//        break;
//    case Qt::Key_Space://减少10音量
//        emit SigPlayOrPause();
//        break;
        
    default:
        break;
    }
}


void MainWid::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (ui->TitleWid->geometry().contains(event->pos()))
        {
            m_bMoveDrag = true;
            m_DragPosition = event->globalPos() - this->pos();
        }
    }

    QWidget::mousePressEvent(event);
}

void MainWid::mouseReleaseEvent(QMouseEvent *event)
{
    m_bMoveDrag = false;

    QWidget::mouseReleaseEvent(event);
}

void MainWid::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bMoveDrag)
    {
        move(event->globalPos() - m_DragPosition);
    }

    QWidget::mouseMoveEvent(event);
}

void MainWid::contextMenuEvent(QContextMenuEvent* event)
{
    m_stMenu.exec(event->globalPos());
}

int MainWid::OutputVideo(const Frame *video_frame)
{
    return ui->ShowWid->draw(video_frame, false, false);
}

void MainWid::OnPlay(QString strFile)
{
    MediaPlayer::GetInstance()->StartPlay(strFile, NULL);
}

void MainWid::OnFullScreenPlay()
{
    if (m_bFullScreenPlay == false)
    {
        m_bFullScreenPlay = true;
        m_stActFullscreen.setChecked(true);
        //脱离父窗口后才能设置
        ui->ShowWid->setWindowFlags(Qt::Window);
        //多屏情况下，在当前屏幕全屏
        QScreen *pStCurScreen = qApp->screens().at(qApp->desktop()->screenNumber(this));
        ui->ShowWid->windowHandle()->setScreen(pStCurScreen);
        
        ui->ShowWid->showFullScreen();


        QRect stScreenRect = pStCurScreen->geometry();
        int nCtrlBarHeight = ui->CtrlBarWid->height();
        int nX = ui->ShowWid->x();
        m_stCtrlBarAnimationShow = QRect(nX, stScreenRect.height() - nCtrlBarHeight, stScreenRect.width(), nCtrlBarHeight);
        m_stCtrlBarAnimationHide = QRect(nX, stScreenRect.height(), stScreenRect.width(), nCtrlBarHeight);

        m_stCtrlbarAnimationShow->setStartValue(m_stCtrlBarAnimationHide);
        m_stCtrlbarAnimationShow->setEndValue(m_stCtrlBarAnimationShow);
        m_stCtrlbarAnimationShow->setDuration(1000);

        m_stCtrlbarAnimationHide->setStartValue(m_stCtrlBarAnimationShow);
        m_stCtrlbarAnimationHide->setEndValue(m_stCtrlBarAnimationHide);
        m_stCtrlbarAnimationHide->setDuration(1000);
        
        ui->CtrlBarWid->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
        ui->CtrlBarWid->windowHandle()->setScreen(pStCurScreen);
        ui->CtrlBarWid->raise();
        ui->CtrlBarWid->setWindowOpacity(0.5);
        ui->CtrlBarWid->showNormal();
        ui->CtrlBarWid->windowHandle()->setScreen(pStCurScreen);
        
        m_stCtrlbarAnimationShow->start();
        m_bFullscreenCtrlBarShow = true;
        m_stFullscreenMouseDetectTimer.start();

        this->setFocus();
    }
    else
    {
        m_bFullScreenPlay = false;
        m_stActFullscreen.setChecked(false);

        m_stCtrlbarAnimationShow->stop(); //快速切换时，动画还没结束导致控制面板消失
        m_stCtrlbarAnimationHide->stop();
        ui->CtrlBarWid->setWindowOpacity(1);
        ui->CtrlBarWid->setWindowFlags(Qt::SubWindow);
        
        ui->ShowWid->setWindowFlags(Qt::SubWindow);

        ui->CtrlBarWid->showNormal();
        ui->ShowWid->showNormal();

        m_stFullscreenMouseDetectTimer.stop();
        this->setFocus();
    }
}

void MainWid::OnCtrlBarAnimationTimeOut()
{
    QApplication::setOverrideCursor(Qt::BlankCursor);
}

void MainWid::OnFullscreenMouseDetectTimeOut()
{
    //     qDebug() << m_stCtrlBarAnimationShow;
    //     qDebug() << cursor().pos();
    //     qDebug() << ui->CtrlBarWid->geometry();
    if (m_bFullScreenPlay)
    {
        if (m_stCtrlBarAnimationShow.contains(cursor().pos()))
        {
            //判断鼠标是否在控制面板上面
            if (ui->CtrlBarWid->geometry().contains(cursor().pos()))
            {
                //继续显示
                m_bFullscreenCtrlBarShow = true;
            }
            else
            {
                //需要显示
                ui->CtrlBarWid->raise();
                
                m_stCtrlbarAnimationShow->start();
                m_stCtrlbarAnimationHide->stop();
                stCtrlBarHideTimer.stop();
            }
        }
        else
        {
            if (m_bFullscreenCtrlBarShow)
            {
                //需要隐藏
                m_bFullscreenCtrlBarShow = false;
                stCtrlBarHideTimer.singleShot(2000, this, &MainWid::OnCtrlBarHideTimeOut);
            }

        }

    }
}

void MainWid::OnCtrlBarHideTimeOut()
{
    m_stCtrlbarAnimationHide->start();
}

void MainWid::SlotShowMenu()
{
    m_stMenu.exec(cursor().pos());
}

void MainWid::SlotShowAbout()
{
    m_stAboutWidget.move(cursor().pos().x() - m_stAboutWidget.width()/2, cursor().pos().y() - m_stAboutWidget.height()/2);
    m_stAboutWidget.show();
}

void MainWid::OpenFile()
{
//    QString strFileName = QFileDialog::getOpenFileName(this, "打开文件", QDir::homePath(),
//                                                       "视频文件(*.mkv *.rmvb *.mp4 *.avi *.flv *.wmv *.3gp rtmp*)");


//    QString strFileName = QFileDialog::getOpenFileName(this, "打开文件", QDir::homePath(),NULL);
//    emit SigOpenFile(strFileName);

    QUrl url = QFileDialog::getOpenFileUrl(this,QStringLiteral("选择路径"),QDir::homePath(),
                                                nullptr,
                                                 nullptr,QFileDialog::DontUseCustomDirectoryIcons);
//    emit SigOpenFile(url.toString());
}

void MainWid::SlotShowSetting()
{
    m_stSettingWid.show();
}

void MainWid::OnCloseBtnClicked()
{
    this->close();
}

void MainWid::OnMinBtnClicked()
{
    this->showMinimized();
}

void MainWid::OnMaxBtnClicked()
{
//    if (isMaximized())
//    {
//        showNormal();
//        emit SigShowMax(false);
//    }
//    else
//    {
//        showMaximized();
//        emit SigShowMax(true);
//    }
}

void MainWid::SlotShowOrHidePlaylist()
{
    if (ui->PlaylistWid->isHidden())
    {
        ui->PlaylistWid->show();
    }
    else
    {
        ui->PlaylistWid->hide();
    }
    this->repaint();        // 重绘主界面
}