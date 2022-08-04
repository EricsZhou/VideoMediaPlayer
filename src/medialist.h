#pragma once

#include <QListWidget>
#include <QMenu>
#include <QAction>
#include "urldialog.h"

class MediaList : public QListWidget
{
    Q_OBJECT

public:
    MediaList(QWidget *parent = 0);
    ~MediaList();
    bool Init();
protected:
    void contextMenuEvent(QContextMenuEvent* event);
private:
    void AddFile(); //添加文件
    void AddUrl();  // 添加网络地址
    void RemoveFile();
signals:
    void SigAddFile(QString strFileName);   //添加文件信号


private:
    QMenu m_stMenu;

    QAction m_stActAddFile;     //添加文件
    QAction m_stActAddUrl;      // 添加网络URL
    QAction m_stActRemove;      //移除文件
    QAction m_stActClearList;   //清空列表
};
