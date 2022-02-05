#ifndef COPIER_H
#define COPIER_H

#include <QThread>

/*!
 * *****************************************************************
 *                               SiBa
 * *****************************************************************
 * \file copier.h
 *
 * \author M. Koren, milan.koren3@gmail.com
 * Source: https:\\github.com/milan-koren/SiBa
 * Licence: EUPL v. 1.2
 * https://joinup.ec.europa.eu/collection/eupl
 * *****************************************************************
 */


/*!
 * \brief The Copier class.
 *
 * \remark Main class.
 */

class Copier : public QThread
{
    Q_OBJECT

private:
    const char* SOURCEDIRID = "source.siba"; //!< the default name of source directory validation file
    const char* TARGETDIRID = "target.siba"; //!< the default name of target directory validation file

    QString _sourceDirectory; //!< source directory
    QString _targetDirectory; //!< target directory
    bool _validate; //!< validation is required
    bool _showDetails; //!* report processing details

    const qint64 MESSAGELIMITSECONDS = 3; //!< minimum time interval between subsequent messages
    qint64 lastMessageSeconds; //!< system time in seconds since the last displayed message

public:
    explicit Copier(QObject *parent=nullptr);
    virtual ~Copier();

    void Setup(QString sourceDirectory, QString targetDirectory, bool validate, bool showDetails);
    virtual void run();

protected:
    bool showStatus(QString message, bool showDetails,
                    qint64 &removedFiles, qint64 &removedFilesSize,
                    qint64 &overwrittenFiles, qint64 &overwrittenFilesSize,
                    qint64 &newFiles, qint64 &newFilesSize,
                    qint64 &directoriesCount, qint64 &newDirectories, qint64 &removedDirectories);

    bool copyDirectories(QString sourceDirectory, QString targetDirectory, bool showDetails,
                         qint64 &removedFiles, qint64 &removedFilesSize,
                         qint64 &overwrittenFiles, qint64 &overwrittenFilesSize,
                         qint64 &newFiles, qint64 &newFilesSize,
                         qint64 &directoriesCount, qint64 &newDirectories, qint64 &removedDirectories);

    bool synchronizeFiles(QString sourceDirectory, QString targetDirectory, bool showDetails,
                          qint64 &removedFiles, qint64 &removedFilesSize,
                          qint64 &overwrittenFiles, qint64 &overwrittenFilesSize,
                          qint64 &newFiles, qint64 &newFilesSize,
                          qint64 &directoriesCount, qint64 &newDirectories, qint64 &removedDirectories);

    bool synchronizeDirectories(QString sourceDirectory, QString targetDirectory, bool showDetails,
                                qint64 &removedFiles, qint64 &removedFilesSize,
                                qint64 &overwrittenFiles, qint64 &overwrittenFilesSize,
                                qint64 &newFiles, qint64 &newFilesSize,
                                qint64 &directoriesCount, qint64 &newDirectories, qint64 &removedDirectories);

signals:
    void signalError(QString message);
    void signalMessage(QString message);
    void signalStatus(QString message,
                      qint64 removedFiles, qint64 removedFilesSize,
                      qint64 overwrittenFiles, qint64 overwrittenFilesSize,
                      qint64 newFiles, qint64 newFilesSize,
                      qint64 directoriesCount, qint64 newDirectories, qint64 removedDirectories);
    void signalBackupFinished(qint64 removedFiles, qint64 removedFilesSize,
                              qint64 overwrittenFiles, qint64 overwrittenFilesSize,
                              qint64 newFiles, qint64 newFilesSize,
                              qint64 directoriesCount, qint64 newDirectories, qint64 removedDirectories);

public slots:
};

#endif // COPIER_H
