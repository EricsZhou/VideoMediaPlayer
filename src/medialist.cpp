#include <QContextMenuEvent>
#include <QFileDialog>
#include <QDebug>
#include "medialist.h"

#pragma execution_character_set("utf-8")

MediaList::MediaList(QWidget *parent)
    : QListWidget(parent),
      m_stMenu(this),
      m_stActAddFile(this),
      m_stActAddUrl(this),
      m_stActRemove(this),
      m_stActClearList(this)
{



}

MediaList::~MediaList()
{
}

bool MediaList::Init()
{
    m_stActAddFile.setText("添加文件");
    m_stMenu.addAction(&m_stActAddFile);

    m_stActAddUrl.setText("添加地址");
    m_stMenu.addAction(&m_stActAddUrl);


    m_stActRemove.setText("移除");
    m_stMenu.addAction(&m_stActRemove);

    m_stActClearList.setText("清空列表");
    m_stMenu.addAction(&m_stActClearList);


    connect(&m_stActAddFile, &QAction::triggered, this, &MediaList::AddFile);
    connect(&m_stActAddUrl, &QAction::triggered, this, &MediaList::AddUrl);
    connect(&m_stActRemove, &QAction::triggered, this, &MediaList::RemoveFile);
    connect(&m_stActClearList, &QAction::triggered, this, &QListWidget::clear);

    return true;
}

void MediaList::contextMenuEvent(QContextMenuEvent* event)
{
    m_stMenu.exec(event->globalPos());
}

void MediaList::AddFile()
{
    QStringList listFileName = QFileDialog::getOpenFileNames(this, "打开文件", QDir::homePath(),
            "视频文件(*.mkv *.rmvb *.mp4 *.avi *.flv *.wmv *.3gp)");
    for (QString strFileName : listFileName)
    {
        emit SigAddFile(strFileName);
    }
}

void MediaList::AddUrl()
{
    UrlDialog urlDialog(this);
    int nResult = urlDialog.exec();
    if(nResult == QDialog::Accepted) {
        //
        QString url = urlDialog.GetUrl();
        qDebug() << "Add url ok, url: " << url;
        if(!url.isEmpty()) {
            qDebug() << "SigAddFile url: " << url;
            emit SigAddFile(url);
        }
    }else {
        qDebug() << "Add url no";
    }
}

void MediaList::RemoveFile()
{
    takeItem(currentRow());
}

