#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDirIterator>
#include <QDir>
#include <QDebug>
#include <QString>
#include <QRegExp>
#include <QImage>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QAbstractListModel>
#include <QStringList>
#include <QStringListModel>

#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   imageScene = new QGraphicsScene();
   imageView = new QGraphicsView(imageScene);
   currentImage = new QGraphicsPixmapItem();

   timer = new QTimer();

   imageScene->addItem(currentImage);
   ui->ImageLayout->addWidget(imageView);
   ui->timeInterval->setText("1000");
   ui->updateBox->setCheckState(Qt::CheckState::Checked);
   imageView->setMinimumWidth(imageView->maximumWidth());
   imageView->setMinimumHeight(imageView->maximumHeight());

   currentImage->setPixmap(QPixmap::fromImage(QImage(":/EmptyImage.png")));

   imageView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
   imageView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
   imageView->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);

   listModel = new QStringListModel(watchHistory, this);
   ui->stackView->setModel(listModel);


   connect(ui->selectDirButton,SIGNAL(clicked()),this,SLOT(selectDirSlot()));
   connect(ui->renameButton,SIGNAL(clicked()),this,SLOT(renameSlot()));
   connect(ui->updateButton,SIGNAL(clicked()),this,SLOT(updateFilesList()));
   connect(ui->randomButton,SIGNAL(clicked()),this,SLOT(showRandomImage()));

   connect(this,SIGNAL(keyPressEvent(QKeyEvent *)),this,SLOT(keyBind(QKeyEvent *)));

   connect(timer,SIGNAL(timeout()),this,SLOT(showRandomImage()));
   connect(ui->slideshowButton,SIGNAL(clicked()),this,SLOT(timerTick()));
   connect(ui->stopButton,SIGNAL(clicked()),this,SLOT(stopTimer()));

   connect(ui->stackView, SIGNAL(clicked(QModelIndex)),this,SLOT(currentChanged(QModelIndex)));
   currItem = -1;

   srand(time(NULL));
}

MainWindow::~MainWindow()
{

    delete ui;
}

QString MainWindow::lastDir(const QString &path)
{
    QString res = "";
    for (int i=path.size()-1;i>=0;i--)
    {
        if (path[i]!='/'){
            if (i==0)
            {
                return "";
            }
            res += path[i];
        }
        else
            break;
    }

    std::reverse(res.begin(),res.end());
    return res;
}

void MainWindow::tagRename(const QString &path, QString tagName)
{
    if (tagName.isEmpty())
        tagName = lastDir(path);

    // Images
    // "*.jpg *.png *.jpeg *.gif *.svg *.bmp"
    renameAll(path,"*.jpg *.png *.jpeg *.gif *.svg *.bmp", tagName);
    // Videos
    // "*.webm *.flv *.ogg *.avi *.wmv *.mpg *.mpeg *.mp4"
    renameAll(path,"*.webm *.flv *.ogg *.avi *.wmv *.mpg *.mpeg *.mp4", tagName);
}

void MainWindow::renameAll(const QString &path, const QString &formats, QString tagName)
{
    if (tagName.isEmpty())
        tagName = lastDir(path);

    QStringList filterList;
    for (auto &s:formats.split(" "))
        filterList << s;

    QDirIterator it(path, filterList, QDir::Files);
    int cnt = 0;
    while (it.hasNext()){
        it.next();
        qDebug() << path + "/" + it.fileName() << " renamed: " << path + "/" + tagName + "_" +QString::number(cnt) + "." + it.fileInfo().suffix();
        QFile::rename(path + "/" + it.fileName(), path + "/" + tagName + "_" +QString::number(cnt) + "." + it.fileInfo().suffix());
        cnt++;
    }
}

void MainWindow::updateDirList(QVector<QString> &v, const QString &path, QString formats)
{
    QStringList filterList;
    for (auto &s:formats.split(" "))
        filterList << s;
    QDirIterator it(path, filterList, QDir::Files);
    while (it.hasNext()){
        it.next();
        qDebug() << "add: " << it.filePath();
        v.push_back(it.filePath());
    }
}

void MainWindow::showImage(const QString &path)
{
    auto w = imageView->width();
    auto h = imageView->height();
    currentImage->setPixmap(QPixmap::fromImage(QImage(path).scaled(w,h,Qt::KeepAspectRatio)));
    auto x = imageView->parentWidget()->width()-w/2;
    auto y = imageView->parentWidget()->height()-h/2;
    imageView->centerOn(x,y);

    listModel->setStringList(watchHistory);
    ui->stackView->update();
}

void MainWindow::selectDirSlot()
{
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Select directory"), "/home",
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    ui->selectedDir->setText(dirName);
    if (ui->updateBox->checkState() == Qt::CheckState::Checked)
        updateFilesList();
}

void MainWindow::renameSlot()
{
    auto dirName  = ui->selectedDir->text();
    auto tagText = ui->tagText->toPlainText();

    if (dirName.isEmpty())
        return;

    tagRename(dirName,tagText);
    QDirIterator it(dirName, QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()){
        it.next();
        qDebug() << "Changing: " << it.filePath();
        tagRename(it.filePath(), tagText);
    }
}

void MainWindow::updateFilesList()
{
    auto dirName = ui->selectedDir->text();
    allFiles = {};

    if (dirName.isEmpty())
        return;
    updateDirList(allFiles, dirName, "*.jpg *.png *.jpeg *.gif *.svg *.bmp");

    QDirIterator it(dirName,QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()){
        it.next();
        updateDirList(allFiles,it.filePath(),"*.jpg *.png *.jpeg *.gif *.svg *.bmp");
    }
}

void MainWindow::showRandomImage()
{
    if (allFiles.isEmpty())
        return;

    if (currItem <= 0){
        currItem = 0;
        auto imgId = rand()%allFiles.size();
        int tryCount =0;
        while (QImage(allFiles[imgId]).isNull() && tryCount < 10)
        {
            imgId = rand()%allFiles.size();
        }
        if (tryCount>=10)
            return;
        watchHistory.push_front(allFiles[imgId]);
    }
    else if(currItem>0)
        currItem--;
    else
        showImage(":/EmptyImage.png");

    showImage(watchHistory[currItem]);
    ui->stackView->setCurrentIndex(QModelIndex(listModel->index(currItem)));
}

void MainWindow::showPreviousImage()
{
    if (watchHistory.size()<=1)
        return;
    if (currItem+1 < watchHistory.size())
        currItem++;
    auto s = watchHistory[currItem];
    showImage(s);
    ui->stackView->setCurrentIndex(QModelIndex(listModel->index(currItem)));
}

void MainWindow::keyBind(QKeyEvent *event)
{
    if (event->key() == Qt::Key_D || event->key() == Qt::Key_Right)
        showRandomImage();
    else if (event->key() == Qt::Key_A || event->key() == Qt::Key_Left)
        showPreviousImage();
    else if (event->key() == Qt::Key_S)
        stopTimer();
    else if (event->key() == Qt::Key_W)
        timerTick();
}

void MainWindow::currentChanged(const QModelIndex &current)
{
    currItem = current.row();
    showImage(watchHistory[currItem]);
    ui->stackView->setCurrentIndex(QModelIndex(listModel->index(currItem)));
}

void MainWindow::stopTimer()
{
    timer->stop();
}

void MainWindow::timerTick()
{
    int t;
    if (ui->timeInterval->text().isEmpty())
        t = 1000;
    else
        t = ui->timeInterval->text().toInt();

    timer->start(t);

}
