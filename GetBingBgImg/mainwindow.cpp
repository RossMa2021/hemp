#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <windows.h>
#include <QImage>
#include <qimagewriter.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    manager = new QNetworkAccessManager(this);

    url = GetBingBgImgAddr();
    qDebug() << "GetBingBgImgAddr " << url;

    QFileInfo info(url.path());
    fileName = info.fileName();
    fileName = fileName.mid(0,fileName.indexOf("_"));

    if (fileName.isEmpty()) fileName = "index.html";
    file = new QFile(fileName+".jpg");
    if(!file->open(QIODevice::WriteOnly))
    {
        delete file;
        file = 0;
        return;
    }
    startRequest(url);
    ui->progressBar->setValue(0);
    ui->progressBar->show();

}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::GetBingBgImgAddr()
{
  const QString URL = "https://www4.bing.com";
  QUrl url(URL);
  QEventLoop loop;
  qDebug() << "Reading code form " << URL;

  reply = manager->get(QNetworkRequest(url));
  //请求结束并下载完成后，退出子事件循环
  QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  //开启子事件循环
  loop.exec();

  //将读到的信息写入文件
  QString code = reply->readAll();

  //处理获得jpg地址
  QString findBeginCode  = "<img id=\"bgImg\" src=\"";
  QString findEndCode  = "1920x1080.jpg\" style=\"display:none\"";
  qint64 ibegin = code.indexOf(findBeginCode) + findBeginCode.length();
  qint64 iend = code.indexOf(findEndCode) + 13;
  QString jpgAddress = "https://cn.bing.com" + code.mid(ibegin,iend - ibegin);
  qDebug() <<"jpg address is "<<jpgAddress;

  return jpgAddress;
}

void MainWindow::startRequest(QUrl url)
{
    qDebug() <<"startRequest";
    reply = manager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::readyRead, this, &MainWindow::httpReadyRead);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &MainWindow::updateDataReadProgress);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::httpFinished);
}

void MainWindow::httpReadyRead()
{
    qDebug() <<"httpReadyRead";
    if (file) file->write(reply->readAll());
}

void MainWindow::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    qDebug() <<"updateDataReadProgress";
    ui->progressBar->setMaximum(totalBytes);
    ui->progressBar->setValue(bytesRead);
}

void MainWindow::httpFinished()
{
    qDebug() <<"httpFinished";
    //ui->progressBar->hide();
    if(file) {
        file->close();
        delete file;
        file = 0;
    }
    reply->deleteLater();
    reply = 0;

    transJpgToBmp();

    setDeskBg();
}

void MainWindow::setDeskBg(){
    QSettings wallPaper("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    preBg = wallPaper.value("Wallpaper").toString();
    qDebug() <<"wallPaper"<<wallPaper.value("Wallpaper").toString();
    QString path("c:\\BingWallPaper\\"+fileName+".bmp"); //把注册表的桌面图片路径改为指定路径.
    wallPaper.setValue("Wallpaper", path);
    QByteArray byte = path.toLocal8Bit(); //调用windows api.
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, byte.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
}

void MainWindow::transJpgToBmp(){
    QString dir_str = "C://BingWallPaper//";

    // 检查目录是否存在，若不存在则新建
    QDir dir;
    if (!dir.exists(dir_str))
    {
        bool res = dir.mkpath(dir_str);
        qDebug() << "新建目录是否成功" << res;
    }

    //转存jpg->bmp
    QImageWriter imageWriter;
    imageWriter.setFormat("bmp");
    imageWriter.setFileName(dir_str+fileName+".bmp");
    if( imageWriter.canWrite() )
    {
       imageWriter.write( QImage(fileName+".jpg", "jpg") );
    }


    QFile file3(fileName+".bmp");
    file3.open(QIODevice::ReadWrite);
    if (file3.exists())
    {
        file3.close();
        qDebug() <<"exists remove";
        file3.remove();
    }
    QFile file1(fileName+".jpg");
    if (file1.exists())
    {
        file1.close();
        qDebug() <<"exists remove";
        file1.remove();
    }
}

void MainWindow::on_pushButton_clicked()
{
    QSettings wallPaper("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    wallPaper.setValue("Wallpaper", preBg);
    QByteArray byte = preBg.toLocal8Bit(); //调用windows api.
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, byte.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
}
