#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "copier.h"

/*!
 * *****************************************************************
 *                               SiBa
 * *****************************************************************
 * \file mainwindow.h
 *
 * \author M. Koren, milan.koren3@gmail.com
 * Source: https:\\github.com/milan-koren/SiBa
 * Licence: EUPL v. 1.2
 * https://joinup.ec.europa.eu/collection/eupl
 * *****************************************************************
 */


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    const int MESSAGELIMIT = 1024;
    int _msgCount = 0;

    bool _printDetails = false;
    Copier _copier;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnBrowseSourceDir_clicked();
    void on_btnBrowseTargetDir_clicked();
    void on_btnRun_clicked();
    void on_btnCancel_clicked();

private:
    Ui::MainWindow *ui;

protected:
    void enableControls(bool enabled);

    QString getStatString(qint64 fileCount, qint64 fileSize);
    void showStatistics(qint64 removedFiles, qint64 removedFilesSize,
                        qint64 overwrittenFiles, qint64 overwrittenFilesSize,
                        qint64 newFiles, qint64 newFilesSize,
                        qint64 directoriesCount, qint64 newDirectories, qint64 removedDirectories);

public slots:
    void showError(QString message);
    void showMessage(QString message);
    void showStatus(QString message,
                    qint64 removedFiles, qint64 removedFilesSize,
                    qint64 overwrittenFiles, qint64 overwrittenFilesSize,
                    qint64 newFiles, qint64 newFilesSize,
                    qint64 directoriesCount, qint64 newDirectories, qint64 removedDirectories);

    void backupFinished(qint64 removedFiles, qint64 removedFilesSize,
                        qint64 overwrittenFiles, qint64 overwrittenFilesSize,
                        qint64 newFiles, qint64 newFilesSize,
                        qint64 directoriesCount, qint64 newDirectories, qint64 removedDirectories);

    void threadFinished();
};

#endif // MAINWINDOW_H
