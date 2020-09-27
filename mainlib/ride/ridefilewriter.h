#ifndef RIDEFILEWRITER_H
#define RIDEFILEWRITER_H

#include <functional>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QXmlStreamWriter>

#include "model/ridefile.h"

/**
 * @brief The RideFileWriter class, used for writing a RideFile to a TCX file.
 */
class RideFileWriter: public QObject
{
    Q_OBJECT
public:
    explicit RideFileWriter(QObject *parent = 0);
    virtual ~RideFileWriter() {}

    /**
     * Write the RideFile to disk.
     * @param rideFile the RideFile to write.
     * @param progressFunction a function which takes a percentage. This can be used
     * to update a progress bar for instance, because writing the RideFile might take a while as TCX is a pretty verbose format.
     * @return the path to the newly written file.
     */
    const QString writeRideFile(const RideFile &rideFile, std::function<void(int)> progressFunction = [](int) {});
    /**
     * @brief determines the path to the RideFile.
     * @param rideFile the RideFile.
     * @return the path where the file will be written.
     */
    QString determineFilePath(const RideFile &rideFile) const;
private:
    void writeTcx(const RideFile &rideFile, QFile &outputFile, std::function<void(int)> &progressFunction);
    void writeLapSummary(QXmlStreamWriter &writer, const RideFile &rideFile);
    void writeTrackPoint(QXmlStreamWriter &writer, const QDateTime &startTime, const RideFile::Sample &sample);

    QString dateTimeString(const QDateTime dateTime, bool withMillis = false) const;
};

#endif // RIDEFILEWRITER_H
