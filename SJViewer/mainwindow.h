#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "stackmodel.h"

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QStack>
#include <QStringList>
#include <QStringListModel>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QVector<QString> allFiles;
    QGraphicsScene *imageScene;
    QGraphicsView *imageView;
    QGraphicsPixmapItem *currentImage;
    QStringList watchHistory;
    QStringListModel *listModel;

    QString lastDir(const QString &path);

    void tagRename(const QString &path, QString tagName="");
    void renameAll(const QString &path,const QString &formats, QString tagName="");
    void updateDirList(QVector<QString> &v, const QString &path, QString formats);

    void showImage(const QString &path);


private slots:
    void selectDirSlot();
    void renameSlot();

    void updateFilesList();
    void showRandomImage();
    void showPreviousImage();
    void keyBind(QKeyEvent *event);
signals:
    void keyPressEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H
