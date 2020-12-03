#include "utils.h"
#include <QApplication>
#include <QTime>

QString read(QString Filename) {
    QFile mFile(Filename);

    if (!mFile.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "could not open file for read";
        return "";
    }

    QTextStream in(&mFile);
    in.setCodec("UTF-8");
    QString mText = in.readAll();

    //qDebug() << mText;

    mFile.close();

    return mText;
}

double clamp(double val, double lower, double upper) {
    return std::max(lower, std::min(val, upper));
}

int sign(double val) {
    return (0 < val) - (val < 0);
}

void delay(int time) {
    QTime dieTime = QTime::currentTime().addSecs(time);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
