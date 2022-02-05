#include <QFile>
#include <QFileInfoList>
#include <QDir>
#include <QDateTime>

#include "copier.h"

/*!
 * *****************************************************************
 *                               SiBa
 * *****************************************************************
 * \file copier.cpp
 *
 * \brief Copier class implemenation.
 *
 * \author M. Koren, milan.koren3@gmail.com
 * Source: https:\\github.com/milan-koren/SiBa
 * Licence: EUPL v. 1.2
 * https://joinup.ec.europa.eu/collection/eupl
 * *****************************************************************
 */


Copier::Copier(QObject *parent) : QThread(parent)
{
}

Copier::~Copier()
{
    if (isRunning()) terminate();
}


void Copier::Setup(QString sourceDirectory, QString targetDirectory, bool validate, bool showDetails)
{
    _sourceDirectory = sourceDirectory;
    _targetDirectory = targetDirectory;
    _validate = validate;
    _showDetails = showDetails;
    lastMessageSeconds = QDateTime::currentSecsSinceEpoch();
}


void Copier::run()
{
    qint64 removedFiles = 0;
    qint64 removedFilesSize = 0;

    qint64 overwrittenFiles = 0;
    qint64 overwrittenFilesSize = 0;

    qint64 newFiles = 0;
    qint64 newFilesSize = 0;

    qint64 directoriesCount = 0;
    qint64 newDirectories = 0;
    qint64 removedDirectories = 0;


    if (!QFile::exists(_sourceDirectory)) {
        emit signalError("Source directory does not exist");
        return;
    }

    if (!QFile::exists(_targetDirectory)) {
        emit signalError("Target directory does not exist");
        return;
    }

    if (_validate) {
        QString sourceFN = _sourceDirectory + "/" + SOURCEDIRID;
        QString targetFN = _targetDirectory + "/" + TARGETDIRID;

        emit signalMessage(sourceFN);
        if (!QFile::exists(sourceFN)) {
            emit signalError("Invalid source directory");
            return;
        }

        emit signalMessage(targetFN);
        if (!QFile::exists(targetFN)) {
            emit signalError("Invalid target directory");
            return;
        }

        emit signalMessage("Directories validated");
    }

    copyDirectories(_sourceDirectory, _targetDirectory, _showDetails,
                    removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                    newFiles, newFilesSize,
                    directoriesCount, newDirectories, removedDirectories);

    emit signalBackupFinished(removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                              newFiles, newFilesSize,
                              directoriesCount, newDirectories, removedDirectories);
}



bool Copier::showStatus(QString message, bool printMessage,
                        qint64 &removedFiles, qint64 &removedFilesSize,
                        qint64 &overwrittenFiles, qint64 &overwrittenFilesSize,
                        qint64 &newFiles, qint64 &newFilesSize,
                        qint64 &directoriesCount, qint64 &newDirectories, qint64 &removedDirectories)
{
    qint64 secs = QDateTime::currentSecsSinceEpoch();

    if (printMessage && (MESSAGELIMITSECONDS <= (secs - lastMessageSeconds))) {
        lastMessageSeconds = secs;
        emit signalStatus(message, removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                          newFiles, newFilesSize, directoriesCount, newDirectories, removedDirectories);
        if (isInterruptionRequested()) return false;
    }
    return true;
}



bool Copier::copyDirectories(QString sourceDirectory, QString targetDirectory, bool showDetails,
                             qint64 &removedFiles, qint64 &removedFilesSize,
                             qint64 &overwrittenFiles, qint64 &overwrittenFilesSize,
                             qint64 &newFiles, qint64 &newFilesSize,
                             qint64 &directoriesCount, qint64 &newDirectories, qint64 &removedDirectories)
{
    if (isInterruptionRequested()) return false;

    directoriesCount++;
    showStatus(sourceDirectory, true,
               removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
               newFiles, newFilesSize, directoriesCount, newDirectories, removedDirectories);

    if (!synchronizeFiles(sourceDirectory, targetDirectory, showDetails,
                          removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                          newFiles, newFilesSize, directoriesCount, newDirectories, removedDirectories))
        return false;

    return synchronizeDirectories(sourceDirectory, targetDirectory, showDetails,
                                  removedFiles, removedFilesSize,
                                  overwrittenFiles, overwrittenFilesSize, newFiles, newFilesSize,
                                  directoriesCount, newDirectories, removedDirectories);
}




/*!
 * \fn bool MainWindow::synchronizeFiles(QString sourceDirectory, QString targetDirectory, bool showDetails)
 * \brief copy and remove files in main directory
 * \param String sourceDirectory: full path to source directory
 * \param String targetDirectory: full path to target directory
 * \param showDetails: print detailed message
 * \return true if archiving was successful
 */
bool Copier::synchronizeFiles(QString sourceDirectory, QString targetDirectory, bool showDetails,
                              qint64 &removedFiles, qint64 &removedFilesSize,
                              qint64 &overwrittenFiles, qint64 &overwrittenFilesSize,
                              qint64 &newFiles, qint64 &newFilesSize,
                              qint64 &directoriesCount, qint64 &newDirectories, qint64 &removedDirectories)

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
                    QFile(targetFileInfo.filePath()).setPermissions(QFile::ReadOther | QFile::WriteOther);
                    QFile::remove(targetFileInfo.filePath());
                    if (QFile::exists(targetFileInfo.filePath())) {
                        emit signalError(QString("Cannot remove file " + targetFileInfo.filePath()));
                    }
                    removedFilesSize += targetFileInfo.size();
                    removedFiles++;
                    if (!showStatus(QString("remove file " + targetFileInfo.filePath()), showDetails,
                                    removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                                    newFiles, newFilesSize, directoriesCount, newDirectories, removedDirectories)) return false;
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
                        QFile(targetFN).setPermissions(QFile::ReadOther | QFile::WriteOther);
                        QFile::remove(targetFN);
                        QFile::copy(sourceFileInfo.filePath(), targetFN);
                        overwrittenFilesSize += sourceFileInfo.size();
                        overwrittenFiles++;
                        if (!showStatus(QString("overwrite " + targetFN), showDetails,
                                        removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                                        newFiles, newFilesSize, directoriesCount, newDirectories, removedDirectories)) return false;
                    }
                }
                else {
                    QFile::copy(sourceFileInfo.filePath(), targetFN);
                    newFilesSize += sourceFileInfo.size();
                    newFiles++;
                    if (!showStatus(QString("copy " + sourceFileInfo.fileName()), showDetails,
                                    removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                                    newFiles, newFilesSize, directoriesCount, newDirectories, removedDirectories)) return false;
                }
            }
        }
    }

    return true;
}



/*!
 * \fn bool MainWindow::synchronizeDirectories(QString sourceDirectory, QString targetDirectory, bool showDetails)
 * \brief recursively copy and remove subdirectories
 * \param sourceDirectory: full path to source directory
 * \param targetDirectory: full path to target directory
 * \param showDetails: print detailed message
 * \return true if archiving was successful
 */
bool Copier::synchronizeDirectories(QString sourceDirectory, QString targetDirectory, bool showDetails,
                                    qint64 &removedFiles, qint64 &removedFilesSize,
                                    qint64 &overwrittenFiles, qint64 &overwrittenFilesSize,
                                    qint64 &newFiles, qint64 &newFilesSize,
                                    qint64 &directoriesCount, qint64 &newDirectories, qint64 &removedDirectories)
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
                    QFile(targetFileInfo.filePath()).setPermissions(QFile::ReadOther | QFile::WriteOther);
                    subDir = new QDir(targetFileInfo.filePath());
                    subDir->removeRecursively();
                    removedDirectories++;
                    if (!showStatus(QString("remove directory " + targetFileInfo.filePath()), true,
                                    removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                                    newFiles, newFilesSize, directoriesCount, newDirectories, removedDirectories)) return false;
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
                    if (!copyDirectories(sourceFileInfo.filePath(), targetFN, showDetails,
                                         removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                                         newFiles, newFilesSize,
                                         directoriesCount, newDirectories, removedDirectories))
                        return false;
                }
                else {
                    targetDir->mkdir(sourceFN);
                    newDirectories++;
                    if (!copyDirectories(sourceFileInfo.filePath(), targetFN, showDetails,
                                         removedFiles, removedFilesSize, overwrittenFiles, overwrittenFilesSize,
                                         newFiles, newFilesSize,
                                         directoriesCount, newDirectories, removedDirectories))
                        return false;
                }
            }
        }
    }

    return true;
}
