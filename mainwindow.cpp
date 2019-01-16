#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QDateTime>
#include <Qt>


/*!
 * \module SiBa
 * \title Simple Backup
 * \brief Incremental backup to archive
 * \author Milan Koren, milan.koren3@gmail.com
 * \version 1.1
 * \date January, 2019
 *
 * Application creates incremental backup of a source directory into a target directory.
 *
 * To prevent accidental overwrite of source files and directories,
 * the source and the destination directories are marked with special files.
 * Put source.siba file into the source directory and target.siba file into the target directory.
 *
 */


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_btnBrowseSourceDir_clicked()
{
    QString sourceDir;
    sourceDir = QFileDialog::getExistingDirectory(nullptr, ("Select source directory"), QDir::currentPath());
    if (!sourceDir.isNull())
    {
        ui->tbSourceDir->setText(sourceDir);
    }
}

void MainWindow::on_btnBrowseTargetDir_clicked()
{
    QString targetDir;
    targetDir = QFileDialog::getExistingDirectory(nullptr, ("Select target directory"), QDir::currentPath());
    if (!targetDir.isNull())
    {
        ui->tbTargetDir->setText(targetDir);
    }
}

void MainWindow::on_btnRun_clicked()
{
    QString sourceDirectory;
    QString targetDirectory;
    bool validate;

    if (_running) return;

    _running = true;
    _cancelling = false;

    enableControls(false);

    ui->txtMessage->setText(nullptr);
    ui->txtError->setText(nullptr);
    _msgCount = 0;

    sourceDirectory = ui->tbSourceDir->text();
    targetDirectory = ui->tbTargetDir->text();

    _removedFiles = 0;
    _removedFilesSize = 0;

    _overwrittenFiles = 0;
    _overwrittenFilesSize = 0;

    _newFiles = 0;
    _newFilesSize = 0;

    _directoriesCount = 0;
    _newDirectories = 0;
    _removedDirectories = 0;

    validate = ui->chbValidateArchive->isChecked();
    showStatistics();
    copyDirectories(sourceDirectory, targetDirectory, validate, false);
    showStatistics();

    showMessage("");
    showMessage(&QString().sprintf("New directories: %i", _newDirectories));
    showMessage(&QString().sprintf("Removed directories: %i", _removedDirectories));
    showMessage("");
    showMessage(&QString().sprintf("New files: %i", _newFiles));
    showMessage(&QString().sprintf("Overwritten files: %i", _overwrittenFiles));
    showMessage(&QString().sprintf("Removed files: %i", _removedFiles));
    if (!_cancelling)
        showMessage("\n*** Finished ***");
    else
        showMessage("\n*** Cancelled by user ***");

    enableControls(true);
    _running = false;
}

void MainWindow::on_btnCancel_clicked()
{
    _cancelling = true;
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

    ui->btnCancel->setEnabled(!enabled);
}


/*!
 * \fn MainWindow::showError(const char *msg)
 * \brief append error message in the error text box
 * \param msg: message to display
 */
void MainWindow::showError(const char *msg)
{
    showError(new QString(msg));
}


/*!
 * \fn MainWindow::showError(QString *msg)
 * \brief append error message in the error text box
 * \param msg: message to display
 */
void MainWindow::showError(QString *msg)
{
    showStatistics();

    ui->txtError->append(*msg);
    QTextCursor textCursor = ui->txtError->textCursor();
    textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor,1);
    ui->txtError->setTextCursor(textCursor);

    QApplication::processEvents();
}


/*!
 * \fn MainWindow::showMessage(const char *msg, bool print)
 * \brief append message in the message text box
 * \param msg: message to display
 * \param print: display message in the text box
 */
void MainWindow::showMessage(const char *msg, bool print)
{
    showMessage(new QString(msg), print);
}


/*!
 * \fn MainWindow::showMessage(QString *msg, bool print)
 * \brief append message in the message text box
 * \param msg: message to display
 * \param print: display message in the text box
 */
void MainWindow::showMessage(QString *msg, bool print)
{
    if (print) {
        if (STATISTICSLIMIT <= _msgCount)
            showStatistics();

        if (MESSAGELIMIT <= _msgCount) {
            ui->txtMessage->setText(*msg);
            _msgCount = 0;
        }
        else {
            ui->txtMessage->append(*msg);
            _msgCount++;
            QTextCursor textCursor = ui->txtMessage->textCursor();
            textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor,1);
            ui->txtMessage->setTextCursor(textCursor);
        }
    }

    QApplication::processEvents();
}


/*!
 * \fn MainWindow::getStatString(long fileCount, long double fileSize)
 * \brief format output string with files statistics
 * \param fileCount: number of files
 * \param fileSize: total size of files in bytes
 * \return formated string
 */
QString MainWindow::getStatString(long fileCount, long double fileSize)
{
    long double sizeGB;

    sizeGB = fileSize/(long double)(1024*1024*1024);
    if (sizeGB < 0.1)
        return QString().setNum(fileCount);
    else
        return QString().sprintf("%i (%#3.1LfGB)", fileCount, sizeGB);
}


/*!
 * \fn MainWindow::showStatistics()
 * \brief formats and display statistics
 */
void MainWindow::showStatistics() {
    ui->lblDirectoriesCount->setText(QString().setNum(_directoriesCount));
    ui->lblNewDirectories->setText(QString().setNum(_newDirectories));
    ui->lblRemovedDirectories->setText(QString().setNum(_removedDirectories));

    ui->lblNewFiles->setText(getStatString(_newFiles, _newFilesSize));
    ui->lblOverwrittenFiles->setText(getStatString(_overwrittenFiles, _overwrittenFilesSize));
    ui->lblRemovedFiles->setText(getStatString(_removedFiles, _removedFilesSize));
}


/*!
 * \fn MainWindow::copyDirectories(QString sourceDirectory, QString targetDirectory, bool validate, bool printMessage)
 * \brief main synchronization procedure
 * \param String sourceDirectory: full path to the source directory
 * \param String targetDirectory: full path to the target directory
 * \param validate: validate source and target directories is requiered
 * \param printMessage: print detailed messages
 * \return true if archiving was successful
 */
bool MainWindow::copyDirectories(QString sourceDirectory, QString targetDirectory, bool validate, bool printMessage)
{
    if (_cancelling) return false;

    if (!QFile::exists(sourceDirectory))
    {
        showError("Source directory does not exist");
        return false;
    }

    if (!QFile::exists(targetDirectory))
    {
        showError("Target directory does not exist");
        return false;
    }

    if (validate)
    {
        QString sourceFN = sourceDirectory + "/" + SOURCEDIRID;
        QString targetFN = targetDirectory + "/" + TARGETDIRID;

        showMessage(&sourceFN);
        if (!QFile::exists(sourceFN))
        {
            showError("Invalid source directory");
            return false;
        }

        showMessage(&targetFN);
        if (!QFile::exists(targetFN))
        {
            showError("Invalid target directory");
            return false;
        }

        showMessage("");
    }

    _directoriesCount++;
    showMessage(&sourceDirectory);
    synchronizeFiles(sourceDirectory, targetDirectory, printMessage);
    synchronizeDirectories(sourceDirectory, targetDirectory, printMessage);

    return true;
}


/*!
 * \fn bool MainWindow::synchronizeFiles(QString sourceDirectory, QString targetDirectory, bool printMessage)
 * \brief copy and remove files in main directory
 * \param String sourceDirectory: full path to source directory
 * \param String targetDirectory: full path to target directory
 * \param printMessage: print detailed message
 * \return true if archiving was successful
 */
bool MainWindow::synchronizeFiles(QString sourceDirectory, QString targetDirectory, bool printMessage)
{
    QDir *sourceDir, *targetDir;
    QFileInfoList sourceFileList;
    QFileInfoList targetFileList;
    QString sourceFN, targetFN;
    QFileInfo *tfInfo;

    sourceDir = new QDir(sourceDirectory);
    sourceFileList = sourceDir->entryInfoList(QDir::Filter::Hidden | QDir::Filter::Files);

    targetDir = new QDir(targetDirectory);
    targetFileList = targetDir->entryInfoList(QDir::Filter::Hidden | QDir::Filter::Files);

    // remove files
    foreach (QFileInfo targetFileInfo, targetFileList) {
        if (targetFileInfo.isFile()) {
            targetFN = targetFileInfo.fileName();
            if (targetFN != SOURCEDIRID && targetFN != TARGETDIRID) {
                sourceFN = sourceDirectory + "/" + targetFN;
                if (!QFile::exists(sourceFN)) {
                    showMessage(new QString("remove file " + targetFileInfo.filePath()), printMessage);
                    QFile(targetFileInfo.filePath()).setPermissions(QFile::ReadOther | QFile::WriteOther);
                    QFile::remove(targetFileInfo.filePath());
                    if (QFile::exists(targetFileInfo.filePath())) {
                        showError(new QString("Cannot remove file " + targetFileInfo.filePath()));
                    }
                    _removedFilesSize += targetFileInfo.size();
                    _removedFiles++;
                }
            }
        }
    }

    // copy or overwrite files
    foreach (QFileInfo sourceFileInfo, sourceFileList) {
        if (sourceFileInfo.isFile())
        {
            sourceFN = sourceFileInfo.fileName();
            if (sourceFN != SOURCEDIRID && sourceFN != TARGETDIRID) {
                targetFN = targetDirectory + "/" + sourceFN;
                if (QFile::exists(targetFN)) {
                    tfInfo = new QFileInfo(targetFN);
                    if (tfInfo->lastModified() < sourceFileInfo.lastModified()) {
                        showMessage(new QString("overwrite " + targetFN), printMessage);
                        QFile(targetFN).setPermissions(QFile::ReadOther | QFile::WriteOther);
                        QFile::remove(targetFN);
                        QFile::copy(sourceFileInfo.filePath(), targetFN);
                        _overwrittenFilesSize += sourceFileInfo.size();
                        _overwrittenFiles++;
                    }
                }
                else {
                    showMessage(new QString("copy " + sourceFileInfo.fileName()), printMessage);
                    QFile::copy(sourceFileInfo.filePath(), targetFN);
                    _newFilesSize += sourceFileInfo.size();
                    _newFiles++;
                }
            }
        }
    }

    return true;
}


/*!
 * \fn bool MainWindow::synchronizeDirectories(QString sourceDirectory, QString targetDirectory, bool printMessage)
 * \brief recursively copy and remove subdirectories
 * \param sourceDirectory: full path to source directory
 * \param targetDirectory: full path to target directory
 * \param printMessage: print detailed message
 * \return true if archiving was successful
 */
bool MainWindow::synchronizeDirectories(QString sourceDirectory, QString targetDirectory, bool printMessage)
{
    QDir *sourceDir, *targetDir, *subDir;
    QFileInfoList sourceFileList;
    QFileInfoList targetFileList;
    QString sourceFN, targetFN;

    sourceDir = new QDir(sourceDirectory);
    sourceFileList = sourceDir->entryInfoList(QDir::Filter::Hidden | QDir::Filter::AllDirs);

    targetDir = new QDir(targetDirectory);
    targetFileList = targetDir->entryInfoList(QDir::Filter::Hidden | QDir::Filter::AllDirs);

    // remove directories
    foreach (QFileInfo targetFileInfo, targetFileList) {
        if (!targetFileInfo.isFile()) {
            targetFN = targetFileInfo.fileName();
            if (targetFN != "." && targetFN != "..") {
                sourceFN = sourceDirectory + "/" + targetFN;
                if (!QFile::exists(sourceFN)) {
                    showMessage(new QString("remove directory " + targetFileInfo.filePath()), true);
                    QFile(targetFileInfo.filePath()).setPermissions(QFile::ReadOther | QFile::WriteOther);
                    subDir = new QDir(targetFileInfo.filePath());
                    subDir->removeRecursively();
                    _removedDirectories++;
                }
            }
        }
    }

    // copy directories
    foreach (QFileInfo sourceFileInfo, sourceFileList) {
        if (!sourceFileInfo.isFile())
        {
            sourceFN = sourceFileInfo.fileName();
            if (sourceFN != "." && sourceFN != "..") {
                targetFN = targetDirectory + "/" + sourceFN;
                if (QFile::exists(targetFN)) {
                    copyDirectories(sourceFileInfo.filePath(), targetFN, false, printMessage);
                }
                else {
                    showMessage(new QString("new directory " + sourceFileInfo.fileName()), printMessage);
                    targetDir->mkdir(sourceFN);
                    copyDirectories(sourceFileInfo.filePath(), targetFN, false, printMessage);
                    _newDirectories++;
                }
            }
        }
    }

    return true;
}
