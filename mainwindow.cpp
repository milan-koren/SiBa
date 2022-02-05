#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QDateTime>
#include <Qt>

/*!
 * *****************************************************************
 *                               SiBa
 * *****************************************************************
 * \file copier.cpp
 *
 * \title Simple Backup
 * \brief Incremental backup to archive
 *
 * \author M. Koren, milan.koren3@gmail.com
 * \version 1.3
 * \date February, 2022
 * Source: https:\\github.com/milan-koren/SiBa
 * Licence: EUPL v. 1.2
 * https://joinup.ec.europa.eu/collection/eupl
 *
 * Application creates incremental backup of a source directory to a target directory.
 *
 * To prevent accidental overwrite of source files and directories,
 * source and the destination directories are marked with special files.
 * Copy the source.siba file into the source directory and target.siba file to the target directory.
 * *****************************************************************
 */


/*!
 * \brief Connects to signals of the Copier object.
 * \param parent Parent widget.
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&_copier, &QThread::finished, this, &MainWindow::threadFinished);
    connect(&_copier, &Copier::signalError, this, &MainWindow::showError);
    connect(&_copier, &Copier::signalMessage, this, &MainWindow::showMessage);
    connect(&_copier, &Copier::signalStatus, this, &MainWindow::showStatus);
    connect(&_copier, &Copier::signalBackupFinished, this, &MainWindow::backupFinished);
}


/*!
 * \brief Default destructor, deletes ui object.
 */
MainWindow::~MainWindow()
{
    delete ui;
}


/*!
 * \brief Browse and setup a source directory.
 */
void MainWindow::on_btnBrowseSourceDir_clicked()
{
    QString sourceDir;
    sourceDir = QFileDialog::getExistingDirectory(nullptr, ("Select source directory"), QDir::currentPath());
    if (!sourceDir.isNull())
    {
        ui->tbSourceDir->setText(sourceDir);
    }
}


/*!
 * \brief Browse and setup a target directory.
 */
void MainWindow::on_btnBrowseTargetDir_clicked()
{
    QString targetDir;
    targetDir = QFileDialog::getExistingDirectory(nullptr, ("Select target directory"), QDir::currentPath());
    if (!targetDir.isNull())
    {
        ui->tbTargetDir->setText(targetDir);
    }
}


/*!
 * \brief Reads input parameters and runs backup.
 */
void MainWindow::on_btnRun_clicked()
{
    QString sourceDirectory;
    QString targetDirectory;
    bool validate;

    if (_copier.isRunning()) return;
    if (_copier.isInterruptionRequested()) return;

    enableControls(false);
    ui->btnCancel->setText("Cancel");

    ui->txtMessage->setText(nullptr);
    ui->txtError->setText(nullptr);
    _msgCount = 0;

    sourceDirectory = ui->tbSourceDir->text();
    targetDirectory = ui->tbTargetDir->text();

    validate = ui->chbValidateArchive->isChecked();

    _copier.Setup(sourceDirectory, targetDirectory, validate, _printDetails);

    _copier.start();
}


/*!
 * \brief Sens interuption request and displays message.
 */
void MainWindow::on_btnCancel_clicked()
{
    if (_copier.isRunning())
    {
        _copier.requestInterruption();
        showError("cancel requested");
    }
    else
    {
        close();
    }
}


/*!
 * \fn MainWindow::enableControls(bool enabled)
 * \brief enable controls in the main form
 * \param enabled: input controls are enabled
 */
void MainWindow::enableControls(bool enabled)
{
    ui->btnRun->setEnabled(enabled);
    ui->btnBrowseSourceDir->setEnabled(enabled);
    ui->btnBrowseTargetDir->setEnabled(enabled);
    ui->tbSourceDir->setEnabled(enabled);
    ui->tbTargetDir->setEnabled(enabled);
}


/*!
 * \fn MainWindow::showMessage(QString message)
 * \brief append message in the error text box
 * \param message: error message to display
 */
void MainWindow::showError(QString message)
{
    ui->txtError->append(message);
    QTextCursor textCursor = ui->txtError->textCursor();
    textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor,1);
    ui->txtError->setTextCursor(textCursor);
}


/*!
 * \fn MainWindow::showMessage(QString msg, bool print)
 * \brief append message in the message text box
 * \param message: message to display
 */
void MainWindow::showMessage(QString message)
{
    if (MESSAGELIMIT <= _msgCount) {
        ui->txtMessage->setText(message);
        _msgCount = 0;
    }
    else {
        ui->txtMessage->append(message);
        _msgCount++;
        QTextCursor textCursor = ui->txtMessage->textCursor();
        textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor,1);
        ui->txtMessage->setTextCursor(textCursor);
    }
    //QApplication::processEvents();
}


/*!
 * \brief Updates the statistics of backup process.
 * \param message Main mesage
 * \param removedFiles The number of removed files.
 * \param removedFilesSize The size of removed files in GB.
 * \param overwrittenFiles the Number of overwritten files.
 * \param overwrittenFilesSize The size of overwritten files in GB.
 * \param newFiles The number of new files.
 * \param newFilesSize The size of new files in GB.
 * \param directoriesCount The number of processed directories.
 * \param newDirectories The number of new directories.
 * \param removedDirectories The number of removed directories.
 */
void MainWindow::showStatus(QString message,
                            qint64 removedFiles, qint64 removedFilesSize,
                            qint64 overwrittenFiles, qint64 overwrittenFilesSize,
                            qint64 newFiles, qint64 newFilesSize,
                            qint64 directoriesCount, qint64 newDirectories, qint64 removedDirectories)
{
    showMessage(message);
    showStatistics(removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                   newFiles, newFilesSize,
                   directoriesCount, newDirectories, removedDirectories);
}


/*!
 * \fn MainWindow::getStatString(long fileCount, long double fileSize)
 * \brief format output string with files statistics
 * \param fileCount: number of files
 * \param fileSize: total size of files in bytes
 * \return formated string
 */
QString MainWindow::getStatString(qint64 fileCount, qint64 fileSize)
{
    double sizeGB;

    sizeGB = fileSize/(long double)(1024*1024*1024);
    if (sizeGB < 0.1l)
        return QString().setNum(fileCount);
    else
        return QString("%1 (%2#3.1fGB)").arg(fileCount).arg(sizeGB);
}


/*!
 * \fn MainWindow::showStatistics()
 * \brief formats and display statistics
 */
void MainWindow::showStatistics(qint64 removedFiles, qint64 removedFilesSize,
                                qint64 overwrittenFiles, qint64 overwrittenFilesSize,
                                qint64 newFiles, qint64 newFilesSize,
                                qint64 directoriesCount, qint64 newDirectories, qint64 removedDirectories)
{
    ui->lblDirectoriesCount->setText(QString().setNum(directoriesCount));
    ui->lblNewDirectories->setText(QString().setNum(newDirectories));
    ui->lblRemovedDirectories->setText(QString().setNum(removedDirectories));

    ui->lblNewFiles->setText(getStatString(newFiles, newFilesSize));
    ui->lblOverwrittenFiles->setText(getStatString(overwrittenFiles, overwrittenFilesSize));
    ui->lblRemovedFiles->setText(getStatString(removedFiles, removedFilesSize));
}


/*!
 * \brief Display final statistics of files backup.
 * \param removedFiles The number of removed files.
 * \param removedFilesSize The size of removed files in GB.
 * \param overwrittenFiles the Number of overwritten files.
 * \param overwrittenFilesSize The size of overwritten files in GB.
 * \param newFiles The number of new files.
 * \param newFilesSize The size of new files in GB.
 * \param directoriesCount The number of processed directories.
 * \param newDirectories The number of new directories.
 * \param removedDirectories The number of removed directories.
 */
void MainWindow::backupFinished(qint64 removedFiles, qint64 removedFilesSize,
                                qint64 overwrittenFiles, qint64 overwrittenFilesSize,
                                qint64 newFiles, qint64 newFilesSize,
                                qint64 directoriesCount, qint64 newDirectories, qint64 removedDirectories)
{
    showStatistics(removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                   newFiles, newFilesSize,
                   directoriesCount, newDirectories, removedDirectories);

    showMessage("");
    showMessage(QString("Directories: %1").arg(directoriesCount));
    showMessage(QString("New directories: %1").arg(newDirectories));
    showMessage(QString("Removed directories: %1").arg(removedDirectories));
    showMessage("");
    showMessage("New files: " + getStatString(newFiles, newFilesSize));
    showMessage("Overwritten files: " + getStatString(overwrittenFiles, overwrittenFilesSize));
    showMessage("Removed files: " + getStatString(removedFiles, removedFilesSize));
    showMessage("");

    if (_copier.isInterruptionRequested())
        showMessage("\n*** Cancelled by user ***");
    else
        showMessage("\n*** Finished ***");
}


/*!
 * \brief Enable controls.
 */
void MainWindow::threadFinished()
{
    enableControls(true);
    ui->btnCancel->setText("Close");
}
