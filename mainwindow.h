#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    const char* SOURCEDIRID = "source.siba";
    const char* TARGETDIRID = "target.siba";
    const int STATISTICSLIMIT = 32;
    const int MESSAGELIMIT = 32 * STATISTICSLIMIT;

    long _removedFiles = 0;
    long double _removedFilesSize = 0.0;

    long _overwrittenFiles = 0;
    long double _overwrittenFilesSize = 0.0;

    long _newFiles = 0;
    long double _newFilesSize = 0.0;

    long _directoriesCount = 0;
    long _newDirectories = 0;
    long _removedDirectories = 0;

    int _msgCount = 0;

    bool _running = false;
    bool _cancelling = false;

public:
    explicit MainWindow(QWidget *parent = 0);
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

    void showError(const char *msg);
    void showError(QString *msg);

    void showMessage(const char *msg, bool print=true);
    void showMessage(QString *msg, bool print=true);

    QString getStatString(long fileCount, long double fileSize);
    void showStatistics();

    bool copyDirectories(QString sourceDirectory, QString targetDirectory, bool validate, bool printMessage=false);

    bool synchronizeFiles(QString sourceDirectory, QString targetDirectory, bool printMessage=false);
    bool synchronizeDirectories(QString sourceDirectory, QString targetDirectory, bool printMessage=false);
};

#endif // MAINWINDOW_H
